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


#include "constants.h"

namespace constants
{
    const char* const VERSION = "1.4";
    const char* const TARGET_VERSION = "1.4";

    const char* const APP_NAME = "xiFastMovie";

    const char* const DATA_FILE_EXT = ".raw";
    const char* const METADATA_FILE_EXT = ".rawm";

    const float MIN_DISPLAY_REFRESH_RATE = 1.0;
    const float MAX_DISPLAY_REFRESH_RATE = 200.0;
    const float DEFAULT_DISPLAY_REFRESH_RATE = 60.0;

    const int32_t ZOOM_POW_MIN = -8;
    const int32_t ZOOM_POW_MAX = 18;
    const double ZOOM_BASE = 4.0 / 3.0; // zoom is ZOOM_BASE^zoomPow
    const uint32_t MIN_WINDOW_WIDTH = 232;
    const uint32_t MIN_WINDOW_HEIGHT = 120;
}
