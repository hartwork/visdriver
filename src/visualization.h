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

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winamp/vis.h>

extern winampVisModule *g_active_vis_module;

void __cdecl SAVSAInit(int maxlatency_in_ms, int srate);

void __cdecl SAVSADeInit();

void __cdecl SAAddPCMData(void *PCMData, int nch, int bps, int timestamp);

int __cdecl SAGetMode();

int __cdecl SAAdd(void *data, int timestamp, int csa);

void __cdecl VSAAddPCMData(void *PCMData, int nch, int bps, int timestamp);

int __cdecl VSAGetMode(int *specNch, int *waveNch);

int __cdecl VSAAdd(void *data, int timestamp);

void __cdecl VSASetInfo(int srate, int nch);

void vis_configure(bool render_from_input_plugin_thread);

void vis_render();

#endif // ifndef VISUALIZATION_H
