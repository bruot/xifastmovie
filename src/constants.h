/*
 * This file is part of the xiFastMovie software, a movie recorder for Ximea
 * cameras.
 *
 * Copyright 2016, 2017 Nicolas Bruot
 *
 *
 * xiFastMovie is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xiFastMovie is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xiFastMovie.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <stdint.h>

namespace constants
{
    extern const char* const VERSION;
    extern const char* const TARGET_VERSION;

    extern const char* const APP_NAME;

    extern const char* const DATA_FILE_EXT;
    extern const char* const METADATA_FILE_EXT;

    extern const float MIN_DISPLAY_REFRESH_RATE;
    extern const float MAX_DISPLAY_REFRESH_RATE;
    extern const float DEFAULT_DISPLAY_REFRESH_RATE;

    extern const int32_t ZOOM_POW_MIN;
    extern const int32_t ZOOM_POW_MAX;
    extern const double ZOOM_BASE; // zoom is ZOOM_BASE^zoomPow
    extern const uint32_t MIN_WINDOW_WIDTH;
    extern const uint32_t MIN_WINDOW_HEIGHT;
}
