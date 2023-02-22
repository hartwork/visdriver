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

#include <argparse/argparse.h>

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int show_version_and_exit(struct argparse *self,
                                 const struct argparse_option *option) {
  printf("%s %s\n", "visdriver", "0.0.0");
  exit(0);
}

static struct argparse_option *
find_argument_writing_to(const void *target, struct argparse_option *options) {
  while (options->type != ARGPARSE_OPT_END) {
    if (options->value == target) {
      return options;
    }
    options++;
  }
  return NULL;
}

static void report_error(struct argparse_option *opt, const char *reason) {
  fprintf(stderr, "ERROR: Option `--%s` %s.\n", opt->long_name, reason);
}

static void blank_line(FILE *file) { fprintf(file, "\n"); }

static void require_argument_that_is_wired_to(const char *const *target,
                                              struct argparse *argparse,
                                              struct argparse_option *options) {
  const char *reason = "is required but missing";

  assert(target != NULL);

  if (*target != NULL) {
    return;
  }

  struct argparse_option *option = find_argument_writing_to(target, options);
  assert(option != NULL);

  report_error(option, reason);
  blank_line(stderr);

  argparse_usage(argparse);

  blank_line(stderr);
  report_error(option, reason);

  exit(1);
}

void parse_command_line(visdriver_config_t *config, int argc,
                        const char **argv) {
  static const char *const usages[] = {
      "visdriver [OPTIONS] --in PATH/IN.dll --out PATH/OUT.dll --vis "
      "PATH/VIS.dll [--] [AUDIO_FILE ..]",
      "visdriver --help",
      "visdriver --version",
      NULL,
  };

  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_BOOLEAN('V', "version", NULL, "show the version and exit",
                  show_version_and_exit, 0, OPT_NONEG),

      OPT_GROUP("Plug-in related arguments:"),
      OPT_STRING('I', "in", &config->input_plugin_filename,
                 "input plug-in to use", NULL, 0, 0),
      OPT_STRING('O', "out", &config->output_plugin_filename,
                 "output plug-in to use", NULL, 0, 0),
      OPT_STRING('W', "vis", &config->vis_plugin_filename, "vis plug-in to use",
                 NULL, 0, 0),

      OPT_END(),
  };

  const char *const description =
      "\n"
      "visdriver uses Winamp plug-ins to visualize audio.";
  const char *const epilog =
      "\n"
      "Software libre licensed under GPL v3 or later.\n"
      "Brought to you by Sebastian Pipping <sebastian@pipping.org>.\n"
      "\n"
      "Please report bugs at https://github.com/hartwork/visdriver -- thank "
      "you!";

  struct argparse argparse;
  argparse_init(&argparse, options, usages, 0);
  argparse_describe(&argparse, description, epilog);

  argc = argparse_parse(&argparse, argc, argv);

  // Check for required arguments
  require_argument_that_is_wired_to(&config->input_plugin_filename, &argparse,
                                    options);
  require_argument_that_is_wired_to(&config->output_plugin_filename, &argparse,
                                    options);
  require_argument_that_is_wired_to(&config->vis_plugin_filename, &argparse,
                                    options);

  // Apply defaults
  static const char *const default_track =
      "line://"; // for in_line.dll or in_linein.dll
  const char *const *const default_tracks = &default_track;

  config->tracks = (argc >= 1) ? (const char *const *)argv : default_tracks;
  config->track_count = (argc >= 1) ? argc : 1;
}
