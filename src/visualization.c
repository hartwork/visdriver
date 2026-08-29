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

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h> // memset

#include <kissfft/kiss_fftr.h>

#include "log.h"
#include "visualization.h"

#define VIS_FRAMES 576 // dictated by vis.h

typedef enum _vis_frame_state_t {
  NOT_LOCKED = '_',
  LOCKED_BY_READER = 'R',
  LOCKED_BY_WRITER = 'W',
} vis_frame_state_t;

typedef struct _vis_bucket_t {
  volatile LONG *p_state; // only a pointer because InterlockedCompareExchange
                          // needs aligned memory
  unsigned char spectrumData[2][VIS_FRAMES];
  unsigned char waveformData[2][VIS_FRAMES];
} vis_bucket_t;

winampVisModule *g_active_vis_module = NULL;
static kiss_fftr_cfg g_kiss_fft_cfg = NULL;
static int16_t g_prev_interleaved[VIS_FRAMES * 2];
static vis_bucket_t g_buckets[3] = {0};
static volatile int g_last_bucket_written = -1;
static bool g_render_from_input_plugin_thread = false;
static kiss_fft_scalar g_hann_factors[VIS_FRAMES * 2];

static kiss_fft_scalar hann_factor(size_t index, size_t samples);

static void compute_hann_factors() {
  for (int i = 0; i < VIS_FRAMES * 2; i++) {
    g_hann_factors[i] = hann_factor(i, VIS_FRAMES * 2);
  }
}

static LONG compare_and_swap_long(LONG volatile *target, LONG expected,
                                  LONG new_value) {
#if defined(_MSC_VER)
  return InterlockedCompareExchange(target, new_value, expected);
#else
  return __sync_val_compare_and_swap(target, expected, new_value);
#endif
}

void __cdecl SAVSAInit(int maxlatency_in_ms, int srate) {
  log_debug("Input plugin announced: Maximum latency %dms, sampling rate %d "
            "(SAVSAInit).",
            maxlatency_in_ms, srate);
  g_active_vis_module->sRate = srate;
  memset(g_prev_interleaved, 0, sizeof(g_prev_interleaved));

  const size_t vis_bucket_count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  for (size_t index = 0; index < vis_bucket_count; index++) {
    g_buckets[index].p_state = malloc(sizeof(LONG));
    if (g_buckets[index].p_state) {
      *g_buckets[index].p_state = NOT_LOCKED;
    }
  }

  g_kiss_fft_cfg = kiss_fftr_alloc(VIS_FRAMES * 2, 0, NULL, NULL);

  compute_hann_factors();
}

void __cdecl SAVSADeInit() {
  const size_t vis_bucket_count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  for (size_t index = 0; index < vis_bucket_count; index++) {
    free((LONG *)g_buckets[index].p_state);
    g_buckets[index].p_state = NULL;
  }

  kiss_fftr_free(g_kiss_fft_cfg);
  g_kiss_fft_cfg = NULL;
}

static kiss_fft_scalar hann_factor(size_t index, size_t samples) {
  // https://en.wikipedia.org/wiki/File:Hanning.svg
  return 1 / 2.0f *
         (1 + cosf(2.0f * (float)M_PI * (index - samples / 2) / samples));
}

static int vis_lock_for_writing() {
  const size_t count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  const size_t start_index = g_last_bucket_written + 1;
  for (size_t distance = 0; distance < count; distance++) {
    const size_t index = (start_index + distance) % count;
    if (g_buckets[index].p_state &&
        compare_and_swap_long(g_buckets[index].p_state, NOT_LOCKED,
                              LOCKED_BY_WRITER) == NOT_LOCKED) {
      return index;
    }
  }
  return -1;
}

static void vis_unlock_after_writing(int index) {
#if !defined(NDEBUG)
  const int count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  assert(index <= count);
#endif

  *g_buckets[index].p_state = NOT_LOCKED;
  g_last_bucket_written = index;
}

static int vis_try_lock_for_reading() {
  if (g_last_bucket_written < 0) {
    return -1;
  }
  const size_t start_index = g_last_bucket_written;
  const size_t count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  for (size_t distance = 0; distance < count; distance++) {
    // NOTE: The reader tries most recent first and least recent last
    //       (i.e. backwards)
    const size_t index = (start_index + count - distance) % count;
    if (g_buckets[index].p_state &&
        compare_and_swap_long(g_buckets[index].p_state, NOT_LOCKED,
                              LOCKED_BY_READER) == NOT_LOCKED) {
      return index;
    }
  }
  return -1;
}

static void vis_unlock_after_reading(int index) {
  if (index < 0) {
    return;
  }

#if !defined(NDEBUG)
  const int count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  assert(index <= count);
#endif

  *g_buckets[index].p_state = NOT_LOCKED;
}

static void vis_read(int index, unsigned char spectrumData[2][VIS_FRAMES],
                     unsigned char waveformData[2][VIS_FRAMES]) {
  if (index < 0) {
    memset(spectrumData, 0, sizeof(g_buckets[index].spectrumData));
    memset(waveformData, 0, sizeof(g_buckets[index].waveformData));
    return;
  }

#if !defined(NDEBUG)
  const int count = sizeof(g_buckets) / sizeof(g_buckets[0]);
  assert(index <= count);
#endif

  memcpy(spectrumData, g_buckets[index].spectrumData,
         sizeof(g_buckets[index].spectrumData));
  memcpy(waveformData, g_buckets[index].waveformData,
         sizeof(g_buckets[index].waveformData));
}

void vis_configure(bool render_from_input_plugin_thread) {
  g_render_from_input_plugin_thread = render_from_input_plugin_thread;
  if (render_from_input_plugin_thread) {
    log_info("Will render from: input plugin thread.");
  } else {
    log_info("Will render from: main thread.");
  }
}

void vis_render() {
  if (g_render_from_input_plugin_thread) {
    return;
  }

  const int index = vis_try_lock_for_reading();

  vis_read(index, g_active_vis_module->spectrumData,
           g_active_vis_module->waveformData);

  g_active_vis_module->nCh = 2;
  g_active_vis_module->Render(g_active_vis_module);

  vis_unlock_after_reading(index);
}

void __cdecl SAAddPCMData(void *PCMData, int nch, int bps, int timestamp) {
  if (nch != 2 || bps != 16) {
    log_error("Need 16 bit stereo samples at the moment, "
              "got %d channels at %d bits per sample instead, skipping.",
              nch, bps);
    return;
  }

  const uint16_t *const interleaved = (uint16_t *)PCMData;

  int bucket_index = -1;
  unsigned char *target_spectrumData[2];
  unsigned char *target_waveformData[2];
  if (g_render_from_input_plugin_thread) {
    target_spectrumData[0] =
        (unsigned char *)&g_active_vis_module->spectrumData[0];
    target_spectrumData[1] =
        (unsigned char *)&g_active_vis_module->spectrumData[1];
    target_waveformData[0] =
        (unsigned char *)&g_active_vis_module->waveformData[0];
    target_waveformData[1] =
        (unsigned char *)&g_active_vis_module->waveformData[1];
  } else {
    bucket_index = vis_lock_for_writing();
    if (bucket_index < 0) { // not expected to ever happen
      const int count = sizeof(g_buckets) / sizeof(g_buckets[0]);
      log_error("All %d vis buckets are currently locked.", count);
      return;
    }
    target_spectrumData[0] =
        (unsigned char *)&g_buckets[bucket_index].spectrumData[0];
    target_spectrumData[1] =
        (unsigned char *)&g_buckets[bucket_index].spectrumData[1];
    target_waveformData[0] =
        (unsigned char *)&g_buckets[bucket_index].waveformData[0];
    target_waveformData[1] =
        (unsigned char *)&g_buckets[bucket_index].waveformData[1];
  }

  // For waveform: De-interleave and scale to 8bit
  for (int i = 0; i < VIS_FRAMES; i++) {
    target_waveformData[0][i] = interleaved[2 * i] / 256;
    target_waveformData[1][i] = interleaved[2 * i + 1] / 256;
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
          g_hann_factors[i];
      scalar_in_second_half[i] =
          (kiss_fft_scalar)(int16_t)interleaved[2 * i + channel] *
          g_hann_factors[i + VIS_FRAMES];
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

      target_spectrumData[channel][i] = final_amplitude;
    }
  }

  if (g_render_from_input_plugin_thread) {
    g_active_vis_module->nCh = 2;
    g_active_vis_module->Render(g_active_vis_module);
  } else {
    vis_unlock_after_writing(bucket_index);
  }

  // Feed future FFT
  memcpy(g_prev_interleaved, interleaved, sizeof(g_prev_interleaved));
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
