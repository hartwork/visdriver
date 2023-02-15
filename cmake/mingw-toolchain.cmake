# This file is part of the visdriver project.
#
# Copyright (c) 2023 Sebastian Pipping <sebastian@pipping.org>
#
# visdriver is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# visdriver is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with visdriver. If not, see <https://www.gnu.org/licenses/>.

set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(WIN32 ON)
set(MINGW ON)
