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
Example of usage of the rawmovie module

This script loads a .raw/.rawm movie from xiFastMovie and displays the first
frame.
"""

import rawmovie
import matplotlib.pyplot as plt


path = 'C:/path/to/your_movie.rawm'

# load the movie into the numpy array "data":
(data, timestamps) = rawmovie.load_mono(path)

# data is a 3d array of size (width, height, number of frames).

# Extract the first frame:
frame = data[0, :, :]

# Print the frame timestamp:
print(timestamps[0])

# Display the frame:
plt.imshow(frame)
plt.gray()
plt.show()
