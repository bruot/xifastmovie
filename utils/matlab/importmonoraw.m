function [data, timestamps] = importmonoraw(path, frame)
% IMPORTMONORAW  Loads a .raw/.rawm movie into a 3d array.
%   data = IMPORTMONORAW(path) Loads a movie in data.
%   [data, timestamp] = IMPORTMONORAW(path [, frame]) Loads a movie in data and timestamps vector.
%
% If the frame index "frame" is given, returns "data" and "timestamp" for
% that specific frame without reading the whole .raw file.

% This file is part of the xiFastMovie software, a movie recorder for Ximea
% cameras.
%
% Copyright 2017, 2018 Nicolas Bruot
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

if nargin == 2
    single_frame = 1;
    if ~isnumeric(frame) || ~isequal(size(frame), [1, 1])
        throw(MException('Rawm:TypeError', ...
            'The "frame" argument must be an integer.'));
    end
else
    single_frame = 0;
end

tree = xmlread(path);
root = tree.getElementsByTagName('movie_metadata').item(0);
header = root.getElementsByTagName('header').item(0);

% Read header
width = str2double(get_elem_value(header, 'width'));
height = str2double(get_elem_value(header, 'height'));
pixel_fmt = char(get_elem_value(header, 'pixel_format'));
if strcmp(pixel_fmt, 'Mono8')
    precision = 'uint8=>uint8';
    bytes_per_sample = 1;
elseif strcmp(pixel_fmt, 'Mono10') || strcmp(pixel_fmt, 'Mono12') ...
    || strcmp(pixel_fmt, 'Mono14') || strcmp(pixel_fmt, 'Mono16')
    precision = 'uint16=>uint16';
    bytes_per_sample = 2;
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
if single_frame
    if frame < 1 || frame > n_frames
        throw(MException('Rawm:ValueError', ...
            '"frame" is out of range.'));
    end
end

if nargout > 1
    if single_frame
        timestamps = str2double( ...
            frames.item(frame - 1).getAttribute('timestamp'));
    else
        timestamps = zeros(n_frames, 1);
        for i = 1:n_frames
            timestamps(i) = str2double( ...
                frames.item(i - 1).getAttribute('timestamp'));
        end
    end
end

[basedir, filename, ~] = fileparts(path);
raw_path = fullfile(basedir, strcat(filename, '.raw'));

% Check .raw file size
stats = dir(raw_path);
if stats.bytes ~= width * height * n_frames * bytes_per_sample
    throw(MException('Rawm:FileError', ...
        '.raw file has wrong size.'));
end

% Read .raw file
fid = fopen(raw_path, 'rb');
if single_frame
    fseek(fid, width * height * (frame - 1) * bytes_per_sample, 'bof');
    if big_endian
        data = fread(fid, width * height, precision, 'ieee-be');
    else
        data = fread(fid, width * height, precision);
    end
    data = reshape(data, [width, height]);
else
    if big_endian
        data = fread(fid, width * height * n_frames, precision, 'ieee-be');
    else
        data = fread(fid, width * height * n_frames, precision);
    end
    data = reshape(data, [width, height, n_frames]);
end
fclose(fid);

end


function value = get_elem_value(tree, tag)
    value = tree.getElementsByTagName(tag).item(0).getFirstChild.getData;
end
