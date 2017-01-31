# This file is part of the xiFastMovie software, a movie recorder for Ximea
# cameras.
#
# Copyright 2016, 2017 Nicolas Bruot
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


# This configuration works for Windows 10, MSVC 2015.  It is highly recommmended
# to compile a 64-bit program, to allow recording movies bigger than 2 GB.  A
# skeleton of configuration for Linux is provided, but it has not been tested.


QT += core concurrent
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = xifastmovie
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += src

unix:INCLUDEPATH += \
    /usr/include/boost

win32:INCLUDEPATH += \
    C:\XIMEA\API

unix:LIBS += \
    -lboost_algorithm \
    -lboost_filesystem

contains(QT_ARCH, i386) {
    unix:LIBS += -L/usr/lib/i386-linux-gnu

    win32:INCLUDEPATH += \
        C:\lib\msvc2015_32\include
    win32:LIBS += \
        C:\lib\msvc2015_32\lib\boost\libboost_filesystem-vc140-mt-1_60.lib \
        C:\lib\msvc2015_32\lib\boost\libboost_program_options-vc140-mt-1_60.lib \
        C:\lib\msvc2015_32\lib\boost\libboost_system-vc140-mt-1_60.lib \
        C:\XIMEA\API\x86\xiapi32.lib
} else {
    unix:LIBS += -L/usr/lib/x86_64-linux-gnu

    win32:INCLUDEPATH += \
        C:\lib\msvc2015_64\include
    win32:LIBS += \
        C:\lib\msvc2015_64\lib\boost\libboost_filesystem-vc140-mt-1_60.lib \
        C:\lib\msvc2015_64\lib\boost\libboost_program_options-vc140-mt-1_60.lib \
        C:\lib\msvc2015_64\lib\boost\libboost_system-vc140-mt-1_60.lib \
        C:\XIMEA\API\x64\xiapi64.lib
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VPATH += src

HEADERS += \
    constants.h \
    xifastmovie.h

SOURCES += \
    main.cpp \
    src/constants.cpp \
    xifastmovie.cpp
