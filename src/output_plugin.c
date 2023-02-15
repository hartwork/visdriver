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

#include "output_plugin.h"
#include "log.h"

typedef Out_Module *(__cdecl *winamp_get_out_module_func)(void);

Out_Module *load_output_module(const char *filename, HWND main_window) {
  const HMODULE dll_handle = LoadLibraryA(filename);
  if (dll_handle == NULL) {
    log_error("LoadLibraryA failed for file \"%s\".", filename);
    return NULL;
  }

  const char *const function_name = "winampGetOutModule";
  winamp_get_out_module_func winampGetOutModule =
      (winamp_get_out_module_func)GetProcAddress(dll_handle, function_name);
  if (winampGetOutModule == NULL) {
    log_error("GetProcAddress failed for function \"%s\".", function_name);
    return NULL;
  }

  Out_Module *const out_module = winampGetOutModule();
  if (out_module == NULL) {
    log_error("winampGetOutModule failed.");
    FreeLibrary(dll_handle);
    return NULL;
  }

  if (out_module->version >= OUT_VER_U) {
    log_error("Unicode output plugins are not supported at the moment.");
    FreeLibrary(dll_handle);
    return NULL;
  }

  out_module->hDllInstance = dll_handle;
  out_module->hMainWindow = main_window;

  return out_module;
}

void unload_output_module(Out_Module *out_module) {
  out_module->Quit();
  FreeLibrary(out_module->hDllInstance);
}
