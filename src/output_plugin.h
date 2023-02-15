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

#ifndef OUTPUT_PLUGIN_H
#define OUTPUT_PLUGIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winamp/out.h>

Out_Module *load_output_module(const char *filename, HWND main_window);

void unload_output_module(Out_Module *out_module);

#endif // ifndef OUTPUT_PLUGIN_H
