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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct _visdriver_config_t {
  const char *input_plugin_filename;
  const char *output_plugin_filename;
  const char *vis_plugin_filename;
  const char *const *tracks;
  int track_count;
  bool render_from_input_plugin_thread;
} visdriver_config_t;

void parse_command_line(visdriver_config_t *config, int argc,
                        const char **argv);

#endif // ifndef CONFIG_H
