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
  [[1]](https://github.com/jaspervdg/lineinWA)
  [[2]](https://home.hccnet.nl/th.v.d.gronde/dev/lineinWA2/)
  or `in_mad.dll`
  [[1]](https://sourceforge.net/projects/plainamp/files/in_mad/)
  [[2]](https://www.mars.org/home/rob/proj/mpeg/mad-plugin/#install)
  ),
- One output plugin binary
  (e.g. `out_wave_gpl.dll`
  [[1]](https://sourceforge.net/projects/plainamp/files/out_wave_gpl/)
  ), and
- One vis plugin binary
  (e.g. `vis_geis.dll`
  or `vis_avs.dll`
  [[1]](https://github.com/grandchild/vis_avs)
  ),
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

```console
# cmake -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build
# make -C build -j$(nproc) VERBOSE=1
```

# How to Run

Let **visdriver** tell you what it needs:
```console
# WINEDEBUG=-all wine ./build/visdriver.exe --help
USAGE: visdriver.exe PATH/IN.dll PATH/OUT.dll PATH/VIS.dll [AUDIO_FILE ..]
```

If you end up with errors about missing DLLs, copying these files in place
should help.  E.g. for MinGW DLLs on Ubuntu 20.04 it would be:

```console
# cp -v \
    /usr/i686-w64-mingw32/lib/libwinpthread-1.dll \
    /usr/lib/gcc/i686-w64-mingw32/9.3-posix/libgcc_s_sjlj-1.dll \
    /usr/lib/gcc/i686-w64-mingw32/9.3-posix/libstdc++-6.dll \
    .
```

The locations of these files vary among GNU/Linux distros.


# How to Force Fullscreen Visualization into a Window

If you would like to force a fullscreen vis plugin into using a Window, there are two options:
- a) Wine's built-in [virtual desktop](https://wiki.winehq.org/FAQ#How_do_I_get_Wine_to_launch_an_application_in_a_virtual_desktop.3F) feature
- b) Using [Xephyr](https://en.wikipedia.org/wiki/Xephyr) for a quick way to a nested Xorg server,
     that your distro has already packaged.

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
