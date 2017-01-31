function [data, timestamps] = importmonoraw(path)
% IMPORTMONORAW  Loads a .raw/.rawm movie into a 3d array.
%   data = IMPORTMONORAW(path) Loads a movie in data.
%   [data, timestamp] = IMPORTMONORAW(path) Loads a movie in data and timestamps vector.

% This file is part of the xiFastMovie software, a movie recorder for Ximea
% cameras.
%
% Copyright 2017 Nicolas Bruot
%
%
% xiFastMovie is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% xiFastMovie is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with xiFastMovie.  If not, see <http://www.gnu.org/licenses/>.

tree = xmlread(path);
root = tree.getElementsByTagName('movie_metadata').item(0);
header = root.getElementsByTagName('header').item(0);

% Read header
width = str2double(get_elem_value(header, 'width'));
height = str2double(get_elem_value(header, 'height'));
pixel_fmt = char(get_elem_value(header, 'pixel_format'));
if strcmp(pixel_fmt, 'Mono8')
    precision = 'uint8=>uint8';
elseif strcmp(pixel_fmt, 'Mono10') || strcmp(pixel_fmt, 'Mono12') ...
    || strcmp(pixel_fmt, 'Mono14') || strcmp(pixel_fmt, 'Mono16')
    precision = 'uint16=>uint16';
else
    throw(MException('Rawm:ValueError', ...
        'Pixel format "%s" not recognized.', pixel_fmt));
end
endianness = char(get_elem_value(header, 'endianness'));
if strcmp(endianness, 'little')
    big_endian = false;
elseif strcmp(endianness, 'big')
    big_endian = true;
else
    throw(MException('Rawm:ValueError', ...
        'Endianness "%s" not recognized.', endianness));
end

% Read frames information
frames_tree = root.getElementsByTagName('frames').item(0);
frames = frames_tree.getElementsByTagName('frame');
n_frames = frames.getLength;

if nargout > 1
    timestamps = zeros(n_frames, 1);
    for i = 1:n_frames
        timestamps(i) = str2double( ...
            frames.item(i - 1).getAttribute('timestamp'));
    end
end

% Read .raw file
[dir, filename, ~] = fileparts(path);
raw_path = fullfile(dir, strcat(filename, '.raw'));
fid = fopen(raw_path, 'rb');
if big_endian
    data = fread(fid, width * height * n_frames, precision, 'ieee-be');
else
    data = fread(fid, width * height * n_frames, precision);
end
fclose(fid);

data = reshape(data, [width, height, n_frames]);

end


function value = get_elem_value(tree, tag)
    value = tree.getElementsByTagName(tag).item(0).getFirstChild.getData;
end
