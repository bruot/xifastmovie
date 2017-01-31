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
Interface to read .raw/.rawm movies from xiFastMovie
"""


import os
import numpy
import xml.etree.ElementTree


def _get_elem(tree, key):
    """Returns the an XML element or raises an appropriate exception"""

    elem = tree.find(key)
    if elem is None:
        raise KeyError('No "%s" element found in the XML data.' % key)
    return elem


def _get_attr(tree, attribute):
    """Returns the an XML attribute or raises an appropriate exception"""

    value = tree.get(attribute)
    if value is None:
        raise KeyError('No "%s" attribute found in the "%s" XML element.' % (attribute,
                                                                             tree.tag))
    return value


def load_mono(rawm_path):
    """Loads a .rawm movie into a numpy array"""

    root = xml.etree.ElementTree.parse(rawm_path).getroot()
    if root.tag != 'movie_metadata':
        raise KeyError('The XML root is not a "movie_metadata" element.')
    header = _get_elem(root, 'header')

    width = int(_get_elem(header, 'width').text)
    height = int(_get_elem(header, 'height').text)
    pixel_fmt = _get_elem(header, 'pixel_format').text
    endianness = _get_elem(header, 'endianness').text
    if endianness == 'little':
        data_type = '<'
    elif endianness == 'big':
        data_type = '>'
    else:
        raise ValueError('Unknown "endianness" parameter value.')

    if pixel_fmt == 'Mono8':
        data_type += 'i1'
    elif pixel_fmt in ['Mono10', 'Mono12', 'Mono14', 'Mono16']:
        data_type += 'i2'
    else:
        raise ValueError('Unkown "pixel_format" parameter value.')

    frames_tree = _get_elem(root, 'frames')
    frames = frames_tree.findall('frame')
    n_frames = len(frames)
    timestamps = numpy.empty(n_frames, numpy.uint64)
    for i in range(n_frames):
        timestamps[i] = int(_get_attr(frames[i], 'timestamp'))

    raw_path = '%s.raw' % os.path.splitext(rawm_path)[0]
    with open(raw_path) as f:
        data = numpy.fromfile(f, dtype=data_type)
    data = data.reshape((n_frames, height, width))

    return (data, timestamps)
