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

#include "config.h"

#include <stdio.h>
#include <string.h>

static const char *get_progname(const char *argv_zero) {
  const char *last_backslash = strrchr(argv_zero, '\\');
  if (last_backslash == NULL) {
    return argv_zero;
  }
  return last_backslash + 1;
}

int parse_command_line(visdriver_config_t *config, int argc,
                       const char *const *argv) {
  if (argc < 4) {
    fprintf(stderr,
            "USAGE: %s PATH/IN.dll PATH/OUT.dll PATH/VIS.dll [AUDIO_FILE ..]",
            get_progname(argv[0]));
    return 1;
  }

  config->input_plugin_filename = argv[1];
  config->output_plugin_filename = argv[2];
  config->vis_plugin_filename = argv[3];

  static const char *const default_track =
      "line://"; // for in_line.dll or in_linein.dll
  const char *const *const default_tracks = &default_track;
  config->tracks =
      (argc >= 5) ? (const char *const *)(argv + 4) : default_tracks;
  config->track_count = (argc >= 5) ? (argc - 4) : 1;

  return 0;
}
