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

#include <stdbool.h>
#include <stdio.h>
#if !defined(_MSC_VER)
#include <unistd.h> // usleep
#endif

#include <winamp/wa_ipc.h>

#include "input_plugin.h"
#include "log.h"
#include "main_window.h"
#include "output_plugin.h"
#include "vis_plugin.h"
#include "visualization.h"

static void sleep_milliseconds(int milliseconds) {
#if defined(_MSC_VER)
  Sleep(milliseconds);
#else
  const useconds_t microseconds = milliseconds * 1000;
  usleep(microseconds);
#endif
}

static bool is_track_finished_message(const MSG *message, HWND main_window) {
  return message->hwnd == main_window && message->message == WM_WA_MPEG_EOF;
}

static void display_playback_status(In_Module *input_module,
                                    int current_track_index, int track_count) {
  const int ms_current = input_module->GetOutputTime();
  const int ms_total = input_module->GetLength();
  if (ms_total <= 0) {
    log_info("[%d/%d] At %dms of stream", current_track_index + 1, track_count,
             ms_current);
  } else {
    const int progress_percent =
        ms_total ? (int)(ms_current * 100.0 / ms_total) : 0;
    log_info("[%d/%d] At %dms of %dms total (%d%%)", current_track_index + 1,
             track_count, ms_current, ms_total, progress_percent);
  }
}

static bool start_playback(In_Module *input_module, const char *current_track,
                           int current_track_index, int track_count) {
  log_info("[%d/%d] Starting playback of \"%s\"...", current_track_index + 1,
           track_count, current_track);

  if (!can_play_file(input_module, current_track)) {
    log_info("[%d/%d] Input plugin refused to play location \"%s\".",
             current_track_index + 1, track_count, current_track);
    current_track_index++;
    return false;
  }

  char title[GETFILEINFO_TITLE_LENGTH] = "";
  int length_in_ms = 0;
  input_module->GetFileInfo(current_track, (char *)&title, &length_in_ms);
  if (title[0] || length_in_ms) {
    log_info("[%d/%d] \"%s\" (%dms).", current_track_index + 1, track_count,
             title[0] ? title : "???", length_in_ms);
  }

  input_module->Play(current_track);

  return true;
}

const char *get_progname(const char *argv_zero) {
  const char *last_backslash = strrchr(argv_zero, '\\');
  if (last_backslash == NULL) {
    return argv_zero;
  }
  return last_backslash + 1;
}

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr,
            "USAGE: %s PATH/IN.dll PATH/OUT.dll PATH/VIS.dll [AUDIO_FILE ..]",
            get_progname(argv[0]));
    return 1;
  }

  log_auto_configure_indent();

  const char *const input_plugin_filename = argv[1];
  const char *const output_plugin_filename = argv[2];
  const char *const vis_plugin_filename = argv[3];

  const char *const default_track =
      "line://"; // for in_line.dll or in_linein.dll
  const char *const *const default_tracks = &default_track;
  const char *const *const tracks =
      (argc >= 5) ? (const char *const *)(argv + 4) : default_tracks;
  const int track_count = (argc >= 5) ? (argc - 4) : 1;
  int current_track_index = -1;
  bool playing = false;

  // Create main window
  const HWND main_window = create_main_window();
  if (main_window == 0) {
    log_error("Could not create window.");
    return 1;
  }

  // Load output plugin
  log_info("Loading output plugin \"%s\"...", output_plugin_filename);
  Out_Module *const output_module =
      load_output_module(output_plugin_filename, main_window);
  if (output_module == NULL) {
    log_error("Output plugin could not be loaded, aborting.");
    return 2;
  }
  log_info("Output plugin is \"%s\" (API 0x%x).", output_module->description,
           output_module->version, output_plugin_filename);
  output_module->Init();

  // Load input plugin
  log_info("Loading input plugin \"%s\"...", input_plugin_filename);
  In_Module *const input_module =
      load_input_module(input_plugin_filename, main_window, output_module);
  if (input_module == NULL) {
    log_error("Input plugin could not be loaded, aborting.");
    unload_output_module(output_module);
    return 3;
  }
  log_info("Input plugin is \"%s\" (API 0x%x, flags 0x%x).",
           input_module->description, (unsigned)input_module->version,
           (unsigned)input_module->UsesOutputPlug);
  input_module->Init();

  // Load vis plugin
  log_info("Loading vis plugin \"%s\"...", vis_plugin_filename);
  HMODULE vis_dll_handle = 0;
  winampVisHeader *const vis_header =
      load_vis_header(vis_plugin_filename, &vis_dll_handle);
  if (vis_header == NULL) {
    log_error("Vis plugin could not be loaded.");
    unload_input_module(input_module);
    unload_output_module(output_module);
    return 1;
  }
  log_info("Vis plugin is \"%s\" (API %d).", vis_header->description,
           vis_header->version);
  winampVisModule *const vis_module =
      load_vis_module(vis_header, 0, main_window, vis_dll_handle);
  if (vis_module == NULL) {
    log_error("Vis plugin \"%s\" has no modules.", vis_plugin_filename);
    unload_vis_header(vis_header, vis_dll_handle);
    unload_input_module(input_module);
    unload_output_module(output_module);
    return 4;
  }

  // Configure vis plugin
  g_active_vis_module = vis_module;
  vis_module->Config(vis_module);
  vis_module->Init(vis_module);

  // Main loop
  MSG message = {};
  bool running = true;
  ULONGLONG last_stat_dump_at_ms = 0;

  while (running) {
    bool needs_playback_action = !playing;

    // Process all available messages
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessage(&message);

      if (is_track_finished_message(&message, main_window)) {
        needs_playback_action = true;
      }

      if (message.message == WM_QUIT) {
        log_debug("Window has been closed, shutting down...");
        running = false;
      }
    }

    // Start to play (at all or the next track)
    if (needs_playback_action) {
      current_track_index++;
      if (current_track_index >= track_count) {
        current_track_index = 0; // i.e. loop the playlist
      }

      const char *const current_track = tracks[current_track_index];

      if (start_playback(input_module, current_track, current_track_index,
                         track_count)) {
        playing = true;
      } else {
        current_track_index++;
      }
    }

    // Display playback stats roughly once per second
    if (playing) {
      const ULONGLONG now_ms = GetTickCount64();
      const ULONGLONG time_elapsed_ms = (now_ms - last_stat_dump_at_ms);
      if (time_elapsed_ms >= 1000) {
        display_playback_status(input_module, current_track_index, track_count);
        last_stat_dump_at_ms = now_ms;
      }
    }

    sleep_milliseconds(1); // to avoid 100% CPU usage
  }

  // Stop playback
  if (playing) {
    log_debug("Stopping playback...");
    input_module->Stop();
    playing = false;
  }

  unload_vis_module(vis_module);
  unload_vis_header(vis_header, vis_dll_handle);
  unload_input_module(input_module);
  unload_output_module(output_module);

  return 0;
}
