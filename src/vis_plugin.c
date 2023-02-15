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

#include "vis_plugin.h"
#include "log.h"

winampVisHeader *load_vis_header(const char *filename, HMODULE *p_dll_handle) {
  const HMODULE dll_handle = LoadLibraryA(filename);
  if (dll_handle == NULL) {
    log_error("LoadLibraryA failed for file \"%s\".", filename);
    return NULL;
  }

  const char *const function_name = "winampVisGetHeader";
  winampVisGetHeaderType winampVisGetHeader =
      (winampVisGetHeaderType)GetProcAddress(dll_handle, function_name);
  if (winampVisGetHeader == NULL) {
    log_error("GetProcAddress failed for function \"%s\".", function_name);
    return NULL;
  }

  winampVisHeader *const vis_header = winampVisGetHeader();
  if (vis_header == NULL) {
    log_error("winampVisGetHeader failed.");
    FreeLibrary(dll_handle);
    return NULL;
  }

  *p_dll_handle = dll_handle;

  return vis_header;
}

winampVisModule *load_vis_module(winampVisHeader *vis_header, int index,
                                 HWND main_window, HMODULE dll_handle) {
  winampVisModule *const vis_module = vis_header->getModule(index);
  if (vis_module == NULL) {
    return NULL;
  }

  vis_module->hDllInstance = dll_handle;
  vis_module->hwndParent = main_window;

  return vis_module;
}

void unload_vis_module(winampVisModule *vis_module) {
  vis_module->Quit(vis_module);
}

void unload_vis_header(winampVisHeader *vis_header, HMODULE dll_handle) {
  (void)vis_header;
  FreeLibrary(dll_handle);
}
