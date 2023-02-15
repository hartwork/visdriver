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

#ifndef VIS_PLUGIN_H
#define VIS_PLUGIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winamp/vis.h>

winampVisHeader *load_vis_header(const char *filename, HMODULE *p_dll_handle);

winampVisModule *load_vis_module(winampVisHeader *vis_header, int index,
                                 HWND main_window, HMODULE dll_handle);

void unload_vis_module(winampVisModule *vis_module);

void unload_vis_header(winampVisHeader *vis_header, HMODULE dll_handle);

#endif // ifndef VIS_PLUGIN_H
