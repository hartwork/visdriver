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

#include <stdint.h>
#include <string.h> // memset

#include "log.h"
#include "visualization.h"

#define VIS_FRAMES 576 // dictated by vis.h

winampVisModule *g_active_vis_module = NULL;

void __cdecl SAVSAInit(int maxlatency_in_ms, int srate) {
  log_debug("Input plugin announced: Maximum latency %dms, sampling rate %d "
            "(SAVSAInit).",
            maxlatency_in_ms, srate);
  g_active_vis_module->sRate = srate;
}

void __cdecl SAVSADeInit() {
  // Nothing to do
}

void __cdecl SAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {
  if (nch != 2 || bps != 16) {
    log_error("Need 16 bit stereo samples at the moment, "
              "got %d channels at %d bits per sample instead, skipping.",
              nch, bps);
    return;
  }

  const uint16_t *const interleaved = (uint16_t *)PCMData;

  for (int i = 0; i < VIS_FRAMES; i++) {
    // De-interleave and scale to 8bit
    g_active_vis_module->waveformData[0][i] = interleaved[2 * i] / 256;
    g_active_vis_module->waveformData[1][i] = interleaved[2 * i + 1] / 256;

    // TODO this is debugging data, do spectral analysis instead
    g_active_vis_module->spectrumData[0][i] = i * (256 - 1) / (VIS_FRAMES - 1);
    g_active_vis_module->spectrumData[1][i] =
        (VIS_FRAMES - i - 1) * (256 - 1) / (VIS_FRAMES - 1);
  }

  g_active_vis_module->nCh = 2;
  g_active_vis_module->Render(g_active_vis_module);
}

int __cdecl SAGetMode() {
  LOG_NOT_IMPLEMENTED();
  return 0; // TODO
}

int __cdecl SAAdd(void *data, int timestamp, int csa) {
  LOG_NOT_IMPLEMENTED();
  return 0; // TODO
}

void __cdecl VSAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {
  // Logging disabled for now because it gets called repeatedly.
  // LOG_NOT_IMPLEMENTED();
}

int __cdecl VSAGetMode(int *specNch, int *waveNch) {
  LOG_NOT_IMPLEMENTED();
  return 0; // TODO
}

int __cdecl VSAAdd(void *data, int timestamp) {
  LOG_NOT_IMPLEMENTED();
  return 0; // TODO
}

void __cdecl VSASetInfo(int srate, int nch) {
  log_debug(
      "Input plugin announced: Sampling rate %d, %d channels (VSASetInfo).",
      srate, nch);
  g_active_vis_module->sRate = srate;
}
