// This file is part of the visdriver project.
//
// Copyright (c) 2023 Sebastian Pipping <sebastian@pipping.org>
//
// visdriver is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// visdriver is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with visdriver. If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>

#include <winamp/wa_ipc.h>

#include "log.h"
#include "main_window.h"

HWND g_main_window;
static HWND g_vis_window = NULL;

#define MESSAGE_CASE(hex, dec, name)                                           \
  case name:                                                                   \
    return #name

static const char *message_name_of(UINT message) {
  switch (message) {
    // Thanks to https://wiki.winehq.org/List_Of_Windows_Messages !
    MESSAGE_CASE(0x0000, 0, WM_NULL);
    MESSAGE_CASE(0x0001, 1, WM_CREATE);
    MESSAGE_CASE(0x0002, 2, WM_DESTROY);
    MESSAGE_CASE(0x0003, 3, WM_MOVE);

    MESSAGE_CASE(0x0005, 5, WM_SIZE);
    MESSAGE_CASE(0x0006, 6, WM_ACTIVATE);
    MESSAGE_CASE(0x0007, 7, WM_SETFOCUS);
    MESSAGE_CASE(0x0008, 8, WM_KILLFOCUS);

    MESSAGE_CASE(0x000a, 10, WM_ENABLE);
    MESSAGE_CASE(0x000b, 11, WM_SETREDRAW);
    MESSAGE_CASE(0x000c, 12, WM_SETTEXT);
    MESSAGE_CASE(0x000d, 13, WM_GETTEXT);
    MESSAGE_CASE(0x000e, 14, WM_GETTEXTLENGTH);
    MESSAGE_CASE(0x000f, 15, WM_PAINT);
    MESSAGE_CASE(0x0010, 16, WM_CLOSE);
    MESSAGE_CASE(0x0011, 17, WM_QUERYENDSESSION);
    MESSAGE_CASE(0x0012, 18, WM_QUIT);
    MESSAGE_CASE(0x0013, 19, WM_QUERYOPEN);
    MESSAGE_CASE(0x0014, 20, WM_ERASEBKGND);
    MESSAGE_CASE(0x0015, 21, WM_SYSCOLORCHANGE);
    MESSAGE_CASE(0x0016, 22, WM_ENDSESSION);

    MESSAGE_CASE(0x0018, 24, WM_SHOWWINDOW);

    MESSAGE_CASE(0x001a, 26, WM_WININICHANGE);
    MESSAGE_CASE(0x001b, 27, WM_DEVMODECHANGE);
    MESSAGE_CASE(0x001c, 28, WM_ACTIVATEAPP);
    MESSAGE_CASE(0x001d, 29, WM_FONTCHANGE);
    MESSAGE_CASE(0x001e, 30, WM_TIMECHANGE);
    MESSAGE_CASE(0x001f, 31, WM_CANCELMODE);
    MESSAGE_CASE(0x0020, 32, WM_SETCURSOR);
    MESSAGE_CASE(0x0021, 33, WM_MOUSEACTIVATE);
    MESSAGE_CASE(0x0022, 34, WM_CHILDACTIVATE);
    MESSAGE_CASE(0x0023, 35, WM_QUEUESYNC);
    MESSAGE_CASE(0x0024, 36, WM_GETMINMAXINFO);

    MESSAGE_CASE(0x0026, 38, WM_PAINTICON);
    MESSAGE_CASE(0x0027, 39, WM_ICONERASEBKGND);
    MESSAGE_CASE(0x0028, 40, WM_NEXTDLGCTL);

    MESSAGE_CASE(0x002a, 42, WM_SPOOLERSTATUS);
    MESSAGE_CASE(0x002b, 43, WM_DRAWITEM);
    MESSAGE_CASE(0x002c, 44, WM_MEASUREITEM);
    MESSAGE_CASE(0x002d, 45, WM_DELETEITEM);
    MESSAGE_CASE(0x002e, 46, WM_VKEYTOITEM);
    MESSAGE_CASE(0x002f, 47, WM_CHARTOITEM);
    MESSAGE_CASE(0x0030, 48, WM_SETFONT);
    MESSAGE_CASE(0x0031, 49, WM_GETFONT);
    MESSAGE_CASE(0x0032, 50, WM_SETHOTKEY);
    MESSAGE_CASE(0x0033, 51, WM_GETHOTKEY);

    MESSAGE_CASE(0x0037, 55, WM_QUERYDRAGICON);

    MESSAGE_CASE(0x0039, 57, WM_COMPAREITEM);

    MESSAGE_CASE(0x003d, 61, WM_GETOBJECT);

    MESSAGE_CASE(0x0041, 65, WM_COMPACTING);

    MESSAGE_CASE(0x0044, 68, WM_COMMNOTIFY);

    MESSAGE_CASE(0x0046, 70, WM_WINDOWPOSCHANGING);
    MESSAGE_CASE(0x0047, 71, WM_WINDOWPOSCHANGED);
    MESSAGE_CASE(0x0048, 72, WM_POWER);

    MESSAGE_CASE(0x004a, 74, WM_COPYDATA);
    MESSAGE_CASE(0x004b, 75, WM_CANCELJOURNAL);

    MESSAGE_CASE(0x004e, 78, WM_NOTIFY);

    MESSAGE_CASE(0x0050, 80, WM_INPUTLANGCHANGEREQUEST);
    MESSAGE_CASE(0x0051, 81, WM_INPUTLANGCHANGE);
    MESSAGE_CASE(0x0052, 82, WM_TCARD);
    MESSAGE_CASE(0x0053, 83, WM_HELP);
    MESSAGE_CASE(0x0054, 84, WM_USERCHANGED);
    MESSAGE_CASE(0x0055, 85, WM_NOTIFYFORMAT);

    MESSAGE_CASE(0x007b, 123, WM_CONTEXTMENU);
    MESSAGE_CASE(0x007c, 124, WM_STYLECHANGING);
    MESSAGE_CASE(0x007d, 125, WM_STYLECHANGED);
    MESSAGE_CASE(0x007e, 126, WM_DISPLAYCHANGE);
    MESSAGE_CASE(0x007f, 127, WM_GETICON);
    MESSAGE_CASE(0x0080, 128, WM_SETICON);
    MESSAGE_CASE(0x0081, 129, WM_NCCREATE);
    MESSAGE_CASE(0x0082, 130, WM_NCDESTROY);
    MESSAGE_CASE(0x0083, 131, WM_NCCALCSIZE);
    MESSAGE_CASE(0x0084, 132, WM_NCHITTEST);
    MESSAGE_CASE(0x0085, 133, WM_NCPAINT);
    MESSAGE_CASE(0x0086, 134, WM_NCACTIVATE);
    MESSAGE_CASE(0x0087, 135, WM_GETDLGCODE);
    MESSAGE_CASE(0x0088, 136, WM_SYNCPAINT);

    MESSAGE_CASE(0x00a0, 160, WM_NCMOUSEMOVE);
    MESSAGE_CASE(0x00a1, 161, WM_NCLBUTTONDOWN);
    MESSAGE_CASE(0x00a2, 162, WM_NCLBUTTONUP);
    MESSAGE_CASE(0x00a3, 163, WM_NCLBUTTONDBLCLK);
    MESSAGE_CASE(0x00a4, 164, WM_NCRBUTTONDOWN);
    MESSAGE_CASE(0x00a5, 165, WM_NCRBUTTONUP);
    MESSAGE_CASE(0x00a6, 166, WM_NCRBUTTONDBLCLK);
    MESSAGE_CASE(0x00a7, 167, WM_NCMBUTTONDOWN);
    MESSAGE_CASE(0x00a8, 168, WM_NCMBUTTONUP);
    MESSAGE_CASE(0x00a9, 169, WM_NCMBUTTONDBLCLK);

    MESSAGE_CASE(0x00ab, 171, WM_NCXBUTTONDOWN);
    MESSAGE_CASE(0x00ac, 172, WM_NCXBUTTONUP);
    MESSAGE_CASE(0x00ad, 173, WM_NCXBUTTONDBLCLK);

    MESSAGE_CASE(0x00ff, 255, WM_INPUT);
    MESSAGE_CASE(0x0100, 256, WM_KEYDOWN); // == WM_KEYFIRST
    MESSAGE_CASE(0x0101, 257, WM_KEYUP);
    MESSAGE_CASE(0x0102, 258, WM_CHAR);
    MESSAGE_CASE(0x0103, 259, WM_DEADCHAR);
    MESSAGE_CASE(0x0104, 260, WM_SYSKEYDOWN);
    MESSAGE_CASE(0x0105, 261, WM_SYSKEYUP);
    MESSAGE_CASE(0x0106, 262, WM_SYSCHAR);
    MESSAGE_CASE(0x0107, 263, WM_SYSDEADCHAR);

    MESSAGE_CASE(0x0109, 265, WM_UNICHAR); // == WM_KEYLAST

    MESSAGE_CASE(0x010d, 269, WM_IME_STARTCOMPOSITION);
    MESSAGE_CASE(0x010e, 270, WM_IME_ENDCOMPOSITION);
    MESSAGE_CASE(0x010f, 271, WM_IME_COMPOSITION); // == WM_IME_KEYLAST
    MESSAGE_CASE(0x0110, 272, WM_INITDIALOG);
    MESSAGE_CASE(0x0111, 273, WM_COMMAND);
    MESSAGE_CASE(0x0112, 274, WM_SYSCOMMAND);
    MESSAGE_CASE(0x0113, 275, WM_TIMER);
    MESSAGE_CASE(0x0114, 276, WM_HSCROLL);
    MESSAGE_CASE(0x0115, 277, WM_VSCROLL);
    MESSAGE_CASE(0x0116, 278, WM_INITMENU);
    MESSAGE_CASE(0x0117, 279, WM_INITMENUPOPUP);

    MESSAGE_CASE(0x011f, 287, WM_MENUSELECT);
    MESSAGE_CASE(0x0120, 288, WM_MENUCHAR);
    MESSAGE_CASE(0x0121, 289, WM_ENTERIDLE);
    MESSAGE_CASE(0x0122, 290, WM_MENURBUTTONUP);
    MESSAGE_CASE(0x0123, 291, WM_MENUDRAG);
    MESSAGE_CASE(0x0124, 292, WM_MENUGETOBJECT);
    MESSAGE_CASE(0x0125, 293, WM_UNINITMENUPOPUP);
    MESSAGE_CASE(0x0126, 294, WM_MENUCOMMAND);
    MESSAGE_CASE(0x0127, 295, WM_CHANGEUISTATE);
    MESSAGE_CASE(0x0128, 296, WM_UPDATEUISTATE);
    MESSAGE_CASE(0x0129, 297, WM_QUERYUISTATE);

    MESSAGE_CASE(0x0132, 306, WM_CTLCOLORMSGBOX);
    MESSAGE_CASE(0x0133, 307, WM_CTLCOLOREDIT);
    MESSAGE_CASE(0x0134, 308, WM_CTLCOLORLISTBOX);
    MESSAGE_CASE(0x0135, 309, WM_CTLCOLORBTN);
    MESSAGE_CASE(0x0136, 310, WM_CTLCOLORDLG);
    MESSAGE_CASE(0x0137, 311, WM_CTLCOLORSCROLLBAR);
    MESSAGE_CASE(0x0138, 312, WM_CTLCOLORSTATIC);

    MESSAGE_CASE(0x0200, 512, WM_MOUSEMOVE); // == WM_MOUSEFIRST
    MESSAGE_CASE(0x0201, 513, WM_LBUTTONDOWN);
    MESSAGE_CASE(0x0202, 514, WM_LBUTTONUP);
    MESSAGE_CASE(0x0203, 515, WM_LBUTTONDBLCLK);
    MESSAGE_CASE(0x0204, 516, WM_RBUTTONDOWN);
    MESSAGE_CASE(0x0205, 517, WM_RBUTTONUP);
    MESSAGE_CASE(0x0206, 518, WM_RBUTTONDBLCLK);
    MESSAGE_CASE(0x0207, 519, WM_MBUTTONDOWN);
    MESSAGE_CASE(0x0208, 520, WM_MBUTTONUP);
    MESSAGE_CASE(0x0209, 521, WM_MBUTTONDBLCLK); // == WM_MOUSELAST
    MESSAGE_CASE(0x020a, 522, WM_MOUSEWHEEL);
    MESSAGE_CASE(0x020b, 523, WM_XBUTTONDOWN);
    MESSAGE_CASE(0x020c, 524, WM_XBUTTONUP);
    MESSAGE_CASE(0x020d, 525, WM_XBUTTONDBLCLK);
    MESSAGE_CASE(0x020e, 526, WM_MOUSEHWHEEL);
    MESSAGE_CASE(0x0210, 528, WM_PARENTNOTIFY);
    MESSAGE_CASE(0x0211, 529, WM_ENTERMENULOOP);
    MESSAGE_CASE(0x0212, 530, WM_EXITMENULOOP);
    MESSAGE_CASE(0x0213, 531, WM_NEXTMENU);
    MESSAGE_CASE(0x0214, 532, WM_SIZING);
    MESSAGE_CASE(0x0215, 533, WM_CAPTURECHANGED);
    MESSAGE_CASE(0x0216, 534, WM_MOVING);
    MESSAGE_CASE(0x0218, 536, WM_POWERBROADCAST);
    MESSAGE_CASE(0x0219, 537, WM_DEVICECHANGE);

    MESSAGE_CASE(0x0220, 544, WM_MDICREATE);
    MESSAGE_CASE(0x0221, 545, WM_MDIDESTROY);
    MESSAGE_CASE(0x0222, 546, WM_MDIACTIVATE);
    MESSAGE_CASE(0x0223, 547, WM_MDIRESTORE);
    MESSAGE_CASE(0x0224, 548, WM_MDINEXT);
    MESSAGE_CASE(0x0225, 549, WM_MDIMAXIMIZE);
    MESSAGE_CASE(0x0226, 550, WM_MDITILE);
    MESSAGE_CASE(0x0227, 551, WM_MDICASCADE);
    MESSAGE_CASE(0x0228, 552, WM_MDIICONARRANGE);
    MESSAGE_CASE(0x0229, 553, WM_MDIGETACTIVE);

    MESSAGE_CASE(0x0230, 560, WM_MDISETMENU);
    MESSAGE_CASE(0x0231, 561, WM_ENTERSIZEMOVE);
    MESSAGE_CASE(0x0232, 562, WM_EXITSIZEMOVE);
    MESSAGE_CASE(0x0233, 563, WM_DROPFILES);
    MESSAGE_CASE(0x0234, 564, WM_MDIREFRESHMENU);

    MESSAGE_CASE(0x0281, 641, WM_IME_SETCONTEXT);
    MESSAGE_CASE(0x0282, 642, WM_IME_NOTIFY);
    MESSAGE_CASE(0x0283, 643, WM_IME_CONTROL);
    MESSAGE_CASE(0x0284, 644, WM_IME_COMPOSITIONFULL);
    MESSAGE_CASE(0x0285, 645, WM_IME_SELECT);
    MESSAGE_CASE(0x0286, 646, WM_IME_CHAR);

    MESSAGE_CASE(0x0288, 648, WM_IME_REQUEST);

    MESSAGE_CASE(0x0291, 657, WM_IME_KEYUP);

    MESSAGE_CASE(0x02a0, 672, WM_NCMOUSEHOVER);
    MESSAGE_CASE(0x02a1, 673, WM_MOUSEHOVER);
    MESSAGE_CASE(0x02a2, 674, WM_NCMOUSELEAVE);
    MESSAGE_CASE(0x02a3, 675, WM_MOUSELEAVE);

    MESSAGE_CASE(0x0300, 768, WM_CUT);
    MESSAGE_CASE(0x0301, 769, WM_COPY);
    MESSAGE_CASE(0x0302, 770, WM_PASTE);
    MESSAGE_CASE(0x0303, 771, WM_CLEAR);
    MESSAGE_CASE(0x0304, 772, WM_UNDO);
    MESSAGE_CASE(0x0305, 773, WM_RENDERFORMAT);
    MESSAGE_CASE(0x0306, 774, WM_RENDERALLFORMATS);
    MESSAGE_CASE(0x0307, 775, WM_DESTROYCLIPBOARD);
    MESSAGE_CASE(0x0308, 776, WM_DRAWCLIPBOARD);
    MESSAGE_CASE(0x0309, 777, WM_PAINTCLIPBOARD);
    MESSAGE_CASE(0x030a, 778, WM_VSCROLLCLIPBOARD);
    MESSAGE_CASE(0x030b, 779, WM_SIZECLIPBOARD);
    MESSAGE_CASE(0x030c, 780, WM_ASKCBFORMATNAME);
    MESSAGE_CASE(0x030d, 781, WM_CHANGECBCHAIN);
    MESSAGE_CASE(0x030e, 782, WM_HSCROLLCLIPBOARD);
    MESSAGE_CASE(0x030f, 783, WM_QUERYNEWPALETTE);
    MESSAGE_CASE(0x0310, 784, WM_PALETTEISCHANGING);
    MESSAGE_CASE(0x0311, 785, WM_PALETTECHANGED);
    MESSAGE_CASE(0x0312, 786, WM_HOTKEY);

    MESSAGE_CASE(0x0317, 791, WM_PRINT);
    MESSAGE_CASE(0x0318, 792, WM_PRINTCLIENT);
    MESSAGE_CASE(0x0319, 793, WM_APPCOMMAND);

    MESSAGE_CASE(0x0358, 856, WM_HANDHELDFIRST);

    MESSAGE_CASE(0x035f, 863, WM_HANDHELDLAST);
    MESSAGE_CASE(0x0360, 864, WM_AFXFIRST);

    MESSAGE_CASE(0x037f, 895, WM_AFXLAST);
    MESSAGE_CASE(0x0380, 896, WM_PENWINFIRST);

    MESSAGE_CASE(0x038f, 911, WM_PENWINLAST);

    MESSAGE_CASE(0x0400, 1024, WM_WA_IPC);
    MESSAGE_CASE(0x0401, 1025, WM_WA_SYSTRAY);
    MESSAGE_CASE(0x0402, 1026, WM_WA_MPEG_EOF);

    MESSAGE_CASE(0x8000, 32768, WM_APP);
  default:
    return "WM_???";
  }
}

#undef MESSAGE_CASE

static void log_window_proc_message(HWND window, UINT message, WPARAM wparam,
                                    LPARAM lparam) {
  if (message >= WM_WA_IPC && message <= WM_WA_MPEG_EOF)
    log_debug("Window message: %d %s(%d) %d %d", window,
              message_name_of(message), message, wparam, lparam);
}

static HWND embed_window(embedWindowState *state) { return g_main_window; }

static void resize_embedded_window(HWND embedded, HWND container) {
  RECT rect;
  if (GetClientRect(container, &rect)) {
    const LONG width = rect.right - rect.left;
    const LONG height = rect.bottom - rect.top;
    SetWindowPos(embedded, NULL, 0, 0, width, height, 0);
  }
}

static LRESULT __stdcall main_window_proc(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam) {
  log_window_proc_message(window, message, wparam, lparam);

  switch (message) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_SIZING:
    if (window == g_main_window && g_vis_window != NULL) {
      resize_embedded_window(g_vis_window, g_main_window);
    }
    break;

  case WM_SIZE: {
    if (g_vis_window != NULL) {
      resize_embedded_window(g_vis_window, g_main_window);
    }
    break;
  }

  case WM_WA_IPC:
    switch (lparam) {
    case IPC_GETVERSION: // == 0
      return 0x2900;     // AVS needs >=2.9
    case IPC_ISPLAYING:  // == 104
      return 1; // i.e. always pretend to be playing (for plugins that woulds
                // ask to first start music).
    case IPC_GETSKIN: // == 201
      strcpy((char *)wparam, "/tmp");
      return wparam;

    case IPC_GETINIFILE: // == 334
    {
      static char ini_path[1024] = "";
      if (ini_path[0] == '\0') {
        GetModuleFileNameA(NULL, ini_path, sizeof(ini_path) / sizeof(char));
        char *const dot_position = strrchr(ini_path, '.');
        if (dot_position == NULL) {
          ini_path[0] = '\0';
        } else {
          strcpy(dot_position, ".ini");
        }
      }
      return (LRESULT)ini_path;
    }
    case IPC_GET_EMBEDIF: // == 505
      ShowWindow(g_main_window, SW_SHOW);
      if (wparam == 0) {
        return (LRESULT)embed_window;
      } else {
        return (LRESULT)embed_window((embedWindowState *)wparam);
      }
      break;

    case IPC_SETVISWND: // == 611
      g_vis_window = (HWND)wparam;
      resize_embedded_window(g_vis_window, g_main_window);
      break;
    }
  }

  return DefWindowProcA(window, message, wparam, lparam);
}

HWND create_main_window() {
  static const char *const window_class_name = "hello";

  WNDCLASSEXA window_class_ex = {
      sizeof(WNDCLASSEXA), // UINT      cbSize;
      0,                   // UINT      style;
      main_window_proc,    // WNDPROC   lpfnWndProc;
      0,                   // int       cbClsExtra;
      0,                   // int       cbWndExtra;
      0,                   // HINSTANCE hInstance;
      0,                   // HICON     hIcon;
      0,                   // HCURSOR   hCursor;
      0,                   // HBRUSH    hbrBackground;
      NULL,                // LPCSTR    lpszMenuName;
      window_class_name,   // LPCSTR    lpszClassName;
      0,                   // HICON     hIconSm;
  };

  ATOM window_class = RegisterClassExA(&window_class_ex);
  if (window_class == 0) {
    log_error("RegisterClassExA failed.");
    return 0;
  }

  // Center the window on the primary screen
  const int window_width = 320;
  const int window_height = 240;
  const int primary_screen_width = GetSystemMetrics(SM_CXSCREEN);
  const int primary_screen_height = GetSystemMetrics(SM_CYSCREEN);
  const int window_left = (primary_screen_width - window_width) / 2;
  const int window_top = (primary_screen_height - window_height) / 2;

  const HWND window =
      CreateWindowExA(0,                 // [in]           DWORD    dwExStyle,
                      window_class_name, // [in, optional] LPCSTR lpClassName,
                      NULL,           // [in, optional] LPCSTR    lpWindowName,
                      WS_TILEDWINDOW, // [in]           DWORD     dwStyle,
                      window_left,    // [in]           int       X,
                      window_top,     // [in]           int       Y,
                      window_width,   // [in]           int       nWidth,
                      window_height,  // [in]           int       nHeight,
                      0,              // [in, optional] HWND      hWndParent,
                      0,              // [in, optional] HMENU     hMenu,
                      0,              // [in, optional] HINSTANCE hInstance,
                      NULL            // [in, optional] LPVOID    lpParam
      );
  if (window == 0) {
    log_error("CreateWindowExA failed.");
    return 0;
  }

  // ShowWindow(window, SW_SHOWNORMAL);

  g_main_window = window;

  return window;
}
