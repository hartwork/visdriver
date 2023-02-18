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

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#endif

#include <stdbool.h>
#include <string.h>

#include <winamp/in2.h>

#include "audio_dsp.h"
#include "log.h"
#include "visualization.h"

typedef In_Module *(__cdecl *winamp_get_in_module2_func)(void);

void __cdecl SetInfo(int bitrate, int srate, int stereo, int synched) {
  // `srate` and `bitrate` are integer-divided by 1000 by the input plugin,
  // these values were likely intended for display only.
  log_debug("Input plugin announced: Bitrate %d (x1000), sampling rate %d "
            "(x1000), stereo %d, "
            "synched %d (SetInfo).",
            bitrate, srate, stereo, synched);
}

bool can_play_file(In_Module *in_module, const char *filename) {
  if (in_module->IsOurFile(filename)) {
    return true;
  }

  const size_t filename_len = strlen(filename);
  const char *pair_list = in_module->FileExtensions; // terminated by \0\0
  for (;;) {
    const size_t ext_group_len = strlen(pair_list);
    if (ext_group_len == 0) {
      return false;
    }

    // Parse a string like "MP3;MP2;MP1"
    const char *extension = pair_list;
    for (;;) {
      const char *after_last = strchr(extension, ';');
      if (after_last == NULL) {
        break;
      }

      const size_t extension_len = after_last - extension;

      if (extension_len + 1 <= filename_len) {
        const char *filename_ext_dot =
            filename + (filename_len - extension_len - 1);
        if (filename_ext_dot[0] != '.') {
          continue;
        }

        if (strncasecmp(filename_ext_dot + 1, extension, extension_len) == 0) {
          return true;
        }
      }

      extension = after_last;
    }

    pair_list += ext_group_len;
    pair_list += strlen(pair_list);
  }
}

In_Module *load_input_module(const char *filename, HWND main_window,
                             Out_Module *output_module) {
  const HMODULE dll_handle = LoadLibraryA(filename);
  if (dll_handle == NULL) {
    log_error("LoadLibraryA failed for file \"%s\".", filename);
    return NULL;
  }

  const char *const function_name = "winampGetInModule2";
  winamp_get_in_module2_func winampGetInModule2 =
      (winamp_get_in_module2_func)GetProcAddress(dll_handle, function_name);
  if (winampGetInModule2 == NULL) {
    log_error("GetProcAddress failed for function \"%s\".", function_name);
    return NULL;
  }

  In_Module *const in_module = winampGetInModule2();
  if (in_module == NULL) {
    log_error("winampGetInModule2 failed");
    FreeLibrary(dll_handle);
    return NULL;
  }

  if ((in_module->version & IN_UNICODE) == IN_UNICODE) {
    log_error("Unicode input plugins are not supported at the moment.");
    FreeLibrary(dll_handle);
    return NULL;
  }

  in_module->hDllInstance = dll_handle;
  in_module->hMainWindow = main_window;

  in_module->SAVSAInit = SAVSAInit;
  in_module->SAVSADeInit = SAVSADeInit;
  in_module->SAAddPCMData = SAAddPCMData;
  in_module->SAGetMode = SAGetMode;
  in_module->SAAdd = SAAdd;
  in_module->VSAAddPCMData = VSAAddPCMData;
  in_module->VSAGetMode = VSAGetMode;
  in_module->VSAAdd = VSAAdd;
  in_module->VSASetInfo = VSASetInfo;

  in_module->dsp_dosamples = dsp_dosamples;
  in_module->dsp_isactive = dsp_isactive;

  in_module->SetInfo = SetInfo;

  in_module->outMod = output_module;

  return in_module;
}

void unload_input_module(In_Module *in_module) {
  in_module->Quit();
  FreeLibrary(in_module->hDllInstance);
}
