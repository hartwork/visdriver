[![Build on Linux](https://github.com/hartwork/visdriver/actions/workflows/linux-mingw.yml/badge.svg)](https://github.com/hartwork/visdriver/actions/workflows/linux-mingw.yml)
[![Build on Windows](https://github.com/hartwork/visdriver/actions/workflows/windows-msvc.yml/badge.svg)](https://github.com/hartwork/visdriver/actions/workflows/windows-msvc.yml)
[![Enforce clang-format](https://github.com/hartwork/visdriver/actions/workflows/clang-format.yml/badge.svg)](https://github.com/hartwork/visdriver/actions/workflows/clang-format.yml)


[![screenshots/visdriver_geiss_804x627.png](https://raw.githubusercontent.com/hartwork/visdriver/main/screenshots/visdriver_geiss_804x627.png)](https://github.com/hartwork/visdriver/blob/main/screenshots/visdriver_geiss_804x627.png)

(Re-titled with `wmctrl -r 'Default - Wine desktop' -N 'Geiss @ visdriver (800x600)'`)


# What is visdriver?

**visdriver** is
a Wine/Windows application
that uses **Winamp plug-ins**
to **visualize audio** without actual Winamp/WACUP,
in particular with MinGW on GNU/Linux.
It is written in C99,
uses plain win32api, and
is licensed under the "GPL v3 or later" license.

It needs:
- One input plug-in binary
  (e.g. `in_line.dll`
  [[source]](https://github.com/jaspervdg/lineinWA)
  [[binary]](https://home.hccnet.nl/th.v.d.gronde/dev/lineinWA2/)
  or `in_mad.dll`
  [[source]](https://sourceforge.net/projects/plainamp/files/in_mad/)
  [[binary]](https://www.mars.org/home/rob/proj/mpeg/mad-plugin/#install) for MP3 playback),
- One output plugin binary
  (e.g. `out_wave_gpl.dll`
  [[source]](https://sourceforge.net/projects/plainamp/files/out_wave_gpl/)
  [[binary]](https://sourceforge.net/projects/plainamp/files/Plainamp/0.2.3/)),
- One vis plugin binary
  (e.g. `vis_geis.dll`
  [[source]](https://github.com/geissomatik/geiss)
  [[binary]](https://github.com/geissomatik/geiss/releases)
  or `vis_avs.dll`
  [[source]](https://github.com/grandchild/vis_avs)
  [[binary]](https://github.com/grandchild/vis_avs/actions)),
- A MinGW compiler (or Visual Studio),
- Wine (or Windows),
- CMake >=3.0 (and potentially GNU make or Ninja).


# Download

If would you like to download ready-to-run Windows binaries
built by the CI off the latest code on branch `main`,
there are two options:
- [Binaries built by MinGW/GCC](https://github.com/hartwork/visdriver/actions/workflows/linux-mingw.yml?query=branch%3Amain)
- [Binaries built by Visual Studio](https://github.com/hartwork/visdriver/actions/workflows/windows-msvc.yml?query=branch%3Amain)

Just click the latest workflow run there for either of these, and
its page will list artifacts for download near the bottom.


# How to Compile

## With MinGW/GCC

```console
# cmake -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build
# make -C build -j$(nproc) VERBOSE=1
```

## With Visual Studio

```console
# cmake -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build
# cmake --build build
```


# How to Run

Let **visdriver** tell you what it needs:
```console
# WINEDEBUG=-all wine ./build/visdriver.exe --help
Usage: visdriver [OPTIONS] --in PATH/IN.dll --out PATH/OUT.dll --vis PATH/VIS.dll [--] [AUDIO_FILE ..]
   or: visdriver --help
   or: visdriver --version

visdriver uses Winamp plug-ins to visualize audio.

    -h, --help        show this help message and exit
    -V, --version     show the version and exit

Plug-in related arguments:
    -I, --in=<str>    input plug-in to use
    -O, --out=<str>   output plug-in to use
    -W, --vis=<str>   vis plug-in to use

Software libre licensed under GPL v3 or later.
Brought to you by Sebastian Pipping <sebastian@pipping.org>.

Please report bugs at https://github.com/hartwork/visdriver -- thank you!
```

If you end up with errors about missing DLLs, copying these files in place
should help.  E.g. for MinGW DLLs on Ubuntu 24.04 it would be:

```console
# cp -v \
    /usr/i686-w64-mingw32/lib/libwinpthread-1.dll \
    /usr/lib/gcc/i686-w64-mingw32/*-posix/libgcc_s_dw2-1.dll \
    /usr/lib/gcc/i686-w64-mingw32/*-posix/libstdc++-6.dll \
    .
```

The locations of these files vary among GNU/Linux distros.


# How to Force Fullscreen Visualization into a Window

If you would like to force a fullscreen vis plugin into using a Window, there are two options:
- a) Wine's built-in [virtual desktop](https://wiki.winehq.org/FAQ#How_do_I_get_Wine_to_launch_an_application_in_a_virtual_desktop.3F) feature
- b) Using [Xephyr](https://en.wikipedia.org/wiki/Xephyr) for a quick way to a nested Xorg server,
     that your distro has already packaged.

For Wine's [virtual desktop](https://wiki.winehq.org/FAQ#How_do_I_get_Wine_to_launch_an_application_in_a_virtual_desktop.3F) feature, this wrapper should do:
```bash
#! /usr/bin/env bash
exec wine explorer /desktop=visdriver,1024x768 ./build/visdriver.exe "$@"
```

For Xephyr, a wrapper script like this should do:
```bash
#! /usr/bin/env bash
set -x -e -u
NESTED_DISPLAY=:1
Xephyr -screen 1024x768 "${NESTED_DISPLAY}" &
xorg_pid=$!
kill_xorg() { kill -2 "${xorg_pid}"; }
trap kill_xorg EXIT
export DISPLAY=${NESTED_DISPLAY}

wine ./build/visdriver.exe "$@"
```

If you need help getting that set up, please reach out.


# Known Limitations

Please expect some rough edges, and potentially even crashes with some plug-ins.

In particular, known limitations are:
- Waveform/spectrum needs 16bit stereo samples input, at the moment.
- Unicode in- and output plug-ins are yet to be supported.
- `in_linein.dll` (SHA1 `7ab08fcc5bc9ebfcc9a8e3d729fadf2cb05e173a`)
  of Winamp 5.66 crashes right after loading for an unknown reason.

---
[Sebastian Pipping](https://github.com/hartwork), Berlin, 2023
