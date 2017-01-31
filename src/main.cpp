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


#include <iostream>
#include <algorithm>
#include <exception>
#include <boost/program_options.hpp>
#include <QApplication>
#include "constants.h"
#include "xifastmovie.h"


namespace po = boost::program_options;


int main(int argc, char* argv[])
{
    // Required parameters
    uint64_t nFrames;
    int exposure;

    // Optional parameters
    uint32_t width = NULL;
    uint32_t height = NULL;
    bool isOffsetXSet = false;
    uint32_t offsetX;
    bool isOffsetYSet = false;
    uint32_t offsetY;
    float framerate = NULL;
    float refreshRate = NULL;
    float gain = NULL;
    std::string pixelFmtStr("mono8");
    std::string outputFile("");

    // Declare the supported options.
    po::options_description reqDesc("Required parameters");
    reqDesc.add_options()
        ("frames,n", po::value<uint64_t>(&nFrames), "Set number of frames")
        ("exposure,e", po::value<int>(&exposure), "Set exposure (microseconds)")
        ;
    po::options_description optDesc("Optional parameters");
    optDesc.add_options()
        // Help
        ("help", "Produce help message")
        ("version", "Print program version")
        ("width,w", po::value<uint32_t>(&width), "Set image width (pixels)")
        ("height,h", po::value<uint32_t>(&height), "Set image height (pixels)")
        ("offsetx,x", po::value<uint32_t>(&offsetX), "Set image x offset (pixels)")
        ("offsety,y", po::value<uint32_t>(&offsetY), "Set image y offset (pixels)")
        ("framerate,r", po::value<float>(&framerate), "Set framerate (fps)")
        ("refresh", po::value<float>(&refreshRate), "Set refresh framerate (fps)")
        ("gain,g", po::value<float>(&gain), "Set gain (dB)")
        ("format,f", po::value<std::string>(&pixelFmtStr), "Pixel format")
        ("output", po::value<std::string>(&outputFile), "Set output file")
        ;
    // The following positional options must also be listed above!
    po::positional_options_description posDesc;
    posDesc.add("output", 1);

    po::options_description allDesc;
    allDesc.add(reqDesc).add(optDesc);

    // Parse parameters
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(allDesc).positional(posDesc).run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << allDesc;
            return 0;
        }
        if (vm.count("version"))
        {
            std::cout << constants::APP_NAME << " movie recorder for Ximea cameras"
                      << std::endl;
            if (std::strcmp(constants::VERSION, constants::TARGET_VERSION) == 0)
                std::cout << "Version " << constants::VERSION << std::endl;
            else
                std::cout << "Dev version " << constants::VERSION
                << " -> " << constants::TARGET_VERSION << std::endl;

            std::cout << std::endl
                      << "Copyright (C) 2016, 2017 Nicolas Bruot" << std::endl
                      << std::endl
                      << "This program is released under the terms of the GNU General Public Licence v3." << std::endl
                      << "The source code is available at https://github.com/bruot/xifastmovie/." << std::endl
                      << std::endl
                      << "It uses xiApi, available at https://www.ximea.com/." << std::endl
                      << std::flush;
            return 0;
        }

        // Required parameters
        //
        // We don't use the required() method from the parser, because it
        // should be possible to print the help without specifying the
        // required parameters. Therefore, these parameters are actually
        // not required for the parser point of view, but required for the
        // rest of the program, which is checked below:
        if (vm.count("frames") == 0)
            throw std::exception("The --frames parameter is required.");
        if (vm.count("exposure") == 0)
            throw std::exception("The --exposure parameter is required.");

        // Optional parameters
        if (vm.count("offsetx")) isOffsetXSet = true;
        if (vm.count("offsety")) isOffsetYSet = true;
    }
    catch(std::exception& e)
    {
        std::cout << "Arguments parsing error: " << e.what()
            << std::endl << std::flush;
        return 1;
    }

    QApplication app(argc, argv);

    std::unique_ptr <xiFastMovie> xfm = std::make_unique<xiFastMovie>();

    try
    {
        xfm->openCamera();
    }
    catch(xiFastMovie::xiFastMovieException& e)
    {
        std::cout << "Error: " << e.what() << std::endl << std::flush;
        return 1;
    }

    try
    {
        // Set pixel format
        std::transform(pixelFmtStr.begin(), pixelFmtStr.end(),
                       pixelFmtStr.begin(), ::tolower);

        xfm->setPixelFmt(pixelFmtStr);

        // Set ROI
        if (width != NULL) xfm->setParamInt(XI_PRM_WIDTH, width);
        if (height != NULL) xfm->setParamInt(XI_PRM_HEIGHT, height);
        if (isOffsetXSet) xfm->setParamInt(XI_PRM_OFFSET_X, offsetX);
        if (isOffsetYSet) xfm->setParamInt(XI_PRM_OFFSET_Y, offsetY);

        // Set exposure
        if (exposure != NULL) xfm->setParamInt(XI_PRM_EXPOSURE, exposure);

        // Set framerate
        if (framerate != NULL) xfm->setFixedFramerate(framerate);

        // Refresh framerate
        if (refreshRate != NULL)
            xfm->setRefreshRate(refreshRate);

        // Set gain
        if (gain != NULL) xfm->setParamFloat(XI_PRM_GAIN, gain);

        xfm->printCameraParameters();

        xfm->show();
        xfm->acquireMovie(nFrames, outputFile);
        // Window and camera are automatically closed at the end of the
        // acquisition.
        app.exec();
    }
    catch (const xiFastMovie::xiFastMovieException& e)
    {
        std::cout << "Error: " << e.what() << std::endl << std::flush;
        // At this point, the camera is necessarily open.  So let's try to close it.
        try
        {
            xfm->closeCamera();
        }
        catch (xiFastMovie::xiFastMovieException){}
        return 1;
    }
    // If the program reaches this point, the camera is open.
    try
    {
        xfm->closeCamera();
    }
    catch (xiFastMovie::xiFastMovieException& e)
    {
        std::cout << "Error: " << e.what() << std::endl << std::flush;
        return 1;
    }

    return 0;
}
