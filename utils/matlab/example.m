% Example of use of importmonoraw

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


path = 'C:/path/to/your_movie.rawm';

% Load movie into 3d array data:
[data, timestamps] = importmonoraw(path);

% Get the first frame:
frame = transpose(data(:, :, 1));

% Print the first frame timestamp:
timestamps(1)

% Display the frame:
figure
image(frame)
% Adapt the colormap to an 8-bit grayscale image:
colormap(gray(256))
axis equal