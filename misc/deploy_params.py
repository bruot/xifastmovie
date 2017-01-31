#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This file is part of the xiFastMovie software, a movie recorder for Ximea
# cameras.
#
# Copyright 2017 Nicolas Bruot
#
#
# xiFastMovie is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# xiFastMovie is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xiFastMovie.  If not, see <http://www.gnu.org/licenses/>.

"""
Deployment parameters for xiFastMovie

This is used by deploy.py.
"""


QT_VERSION = '5.8'

FILES = {
         'common': (
                    ('./deployment_README', 'README'),
                    ('../COPYING', ''),
                    ('../utils/matlab/example.m', 'utils/matlab/example.m'),
                    ('../utils/matlab/importmonoraw.m', 'utils/matlab/importmonoraw.m'),
                    ('../utils/python/example.py', 'utils/python/example.py'),
                    ('../utils/python/rawmovie.py', 'utils/python/rawmovie.py'),
                   ),
         'x86': (
                 ('C:/Qt/%s/msvc2015/bin/Qt5Core.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015/bin/Qt5Gui.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015/bin/Qt5Widgets.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015/plugins/platforms/qwindows.dll' % QT_VERSION, 'platforms/qwindows.dll'),
                 ('C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x86/Microsoft.VC140.CRT/msvcp140.dll', ''),
                 ('C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x86/Microsoft.VC140.CRT/vcruntime140.dll', ''),
                ),
         'x64': (
                 ('C:/Qt/%s/msvc2015_64/bin/Qt5Core.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015_64/bin/Qt5Gui.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015_64/bin/Qt5Widgets.dll' % QT_VERSION, ''),
                 ('C:/Qt/%s/msvc2015_64/plugins/platforms/qwindows.dll' % QT_VERSION, 'platforms/qwindows.dll'),
                 ('C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x64/Microsoft.VC140.CRT/msvcp140.dll', ''),
                 ('C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x64/Microsoft.VC140.CRT/vcruntime140.dll', ''),
                ),
        }

OUTPUT_DIR = '../../deployment'