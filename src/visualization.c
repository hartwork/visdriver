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
#define _USE_MATH_DEFINES // for M_PI from math.h
#else
#define _GNU_SOURCE // for M_PI from math.h
#endif

#include <math.h>
#include <stdint.h>
#include <string.h> // memset

#include <kissfft/kiss_fftr.h>

#include "log.h"
#include "visualization.h"

#define VIS_FRAMES 576 // dictated by vis.h

winampVisModule *g_active_vis_module = NULL;
static kiss_fftr_cfg g_kiss_fft_cfg = NULL;
static int16_t g_prev_interleaved[VIS_FRAMES * 2];

void __cdecl SAVSAInit(int maxlatency_in_ms, int srate) {
  log_debug("Input plugin announced: Maximum latency %dms, sampling rate %d "
            "(SAVSAInit).",
            maxlatency_in_ms, srate);
  g_active_vis_module->sRate = srate;
  memset(g_prev_interleaved, 0, sizeof(g_prev_interleaved));

  g_kiss_fft_cfg = kiss_fftr_alloc(VIS_FRAMES * 2, 0, NULL, NULL);
}

void __cdecl SAVSADeInit() {
  kiss_fftr_free(g_kiss_fft_cfg);
  g_kiss_fft_cfg = NULL;
}

static kiss_fft_scalar hann_factor(size_t index, size_t samples) {
  // https://en.wikipedia.org/wiki/File:Hanning.svg
  return 1 / 2.0f *
         (1 + cosf(2.0f * (float)M_PI * (index - samples / 2) / samples));
}

void __cdecl SAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {
  if (nch != 2 || bps != 16) {
    log_error("Need 16 bit stereo samples at the moment, "
              "got %d channels at %d bits per sample instead, skipping.",
              nch, bps);
    return;
  }

  const uint16_t *const interleaved = (uint16_t *)PCMData;

  // For waveform: De-interleave and scale to 8bit
  for (int i = 0; i < VIS_FRAMES; i++) {
    g_active_vis_module->waveformData[0][i] = interleaved[2 * i] / 256;
    g_active_vis_module->waveformData[1][i] = interleaved[2 * i + 1] / 256;
  }

  // For spectrum: De-interleave, do spectral analysis, and scale to 8bit
  for (int channel = 0; channel < 2; channel++) {
    kiss_fft_scalar scalar_in[VIS_FRAMES * 2];
    kiss_fft_cpx cx_out[VIS_FRAMES * 2];
    kiss_fft_scalar *const scalar_in_first_half = scalar_in;
    kiss_fft_scalar *const scalar_in_second_half = scalar_in + VIS_FRAMES;

    // Prepare FFT input
    for (int i = 0; i < VIS_FRAMES; i++) {
      // De-interleave and apply Hann window function
      scalar_in_first_half[i] =
          (kiss_fft_scalar)g_prev_interleaved[2 * i + channel] *
          hann_factor(i, VIS_FRAMES * 2);
      scalar_in_second_half[i] =
          (kiss_fft_scalar)(int16_t)interleaved[2 * i + channel] *
          hann_factor(i + VIS_FRAMES, VIS_FRAMES * 2);
    }

    // Apply FFT
    kiss_fftr(g_kiss_fft_cfg, scalar_in, cx_out);

    // Post-process FFT output, in particular do scaling:
    // - We need to compensate the scaling that FFT did:
    //   factor "1.0f / (VIS_FRAMES / 2)".
    // - We need to convert range from 0..2^15-1 to 0..2^8-1:
    //   factor "1.0f / INT16_MAX * UINT8_MAX".
    // - The rest is compensation of the Hann window plus additional zoom:
    //   factor "5.0f".
    const kiss_fft_scalar amplitude_scale =
        1.0f / (VIS_FRAMES / 2) * 5.0f / INT16_MAX * UINT8_MAX;

    for (int i = 0; i < VIS_FRAMES; i++) {
      const kiss_fft_scalar real = cx_out[i + 1].r;
      const kiss_fft_scalar imag = cx_out[i + 1].i;
      const kiss_fft_scalar amplitude =
          sqrt(real * real + imag * imag) * amplitude_scale;
      const unsigned char final_amplitude =
          (amplitude > UINT8_MAX)
              ? UINT8_MAX
              : ((amplitude < 0) ? 0 : (unsigned char)amplitude);

      g_active_vis_module->spectrumData[channel][i] = final_amplitude;
    }
  }

  // Feed future FFT
  memcpy(g_prev_interleaved, interleaved, sizeof(g_prev_interleaved));

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
