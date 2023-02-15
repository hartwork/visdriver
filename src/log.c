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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // for getenv
#include <string.h> // for strcmp

static bool g_log_indent = false;

static void log_intro(const char *log_level) {
  const char *const indent = g_log_indent ? "  " : "";
  fprintf(stderr, "%s%s: %s: ", indent, "visdriver", log_level);
}

void log_auto_configure_indent() {
  // Wine normally dumps a lot of debugging output which
  // makes our own messages rather hard to see.
  // So if Wine debug output is enabled, we indent our own messages
  // a bit in order to make them easier to tell apart from Wine debug output.
  if (getenv("WINEUSERNAME") == NULL) {
    return;
  }
  const char *winedebug = getenv("WINEDEBUG");
  if (winedebug != NULL && strcmp(winedebug, "-all") == 0) {
    return;
  }
  g_log_indent = true;
}

void log_debug(const char *format, ...) {
  log_intro("debug");
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

void log_error(const char *format, ...) {
  log_intro("ERROR");
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

void log_info(const char *format, ...) {
  log_intro(" INFO");
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}
