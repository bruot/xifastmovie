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


#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <exception>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <memory.h>
//
#ifdef WIN32
#include "xiApi.h" // Windows
#else
#include <xiApi.h> // Linux, OSX
#endif
//
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <qtconcurrentrun.h>
#include <QTimer>
#include <QEvent>
#include <QRectF>
#include <QGraphicsSceneWheelEvent>
#include "constants.h"
#include "xifastmovie.h"


namespace fs = boost::filesystem;


xiFastMovie::xiFastMovie(QWidget* parent) :
    QMainWindow(parent),
    scene{new QGraphicsScene(this)},
    pixmapItem{new QGraphicsPixmapItem()},
    timer{nullptr},
    currentFrameIndex{-1},
    frameWidth{0},
    frameHeight{0},
    frameSize{0},
    pixelFmt{"Mono8"},
    bytesPerSample{1},
    bitDepth{8},
    refreshRate{constants::DEFAULT_DISPLAY_REFRESH_RATE},
    data{nullptr},
    currFrame8{nullptr},
    zoomIndex{0}
{
    setMinimumSize(constants::MIN_WINDOW_WIDTH,
                   constants::MIN_WINDOW_HEIGHT);
    view = new QGraphicsView(scene);
    scene->installEventFilter(this);
    scene->addItem(pixmapItem);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(view);

    connect(this, SIGNAL(changedGeometry()), this, SLOT(updateGeometry()));
    connect(this, SIGNAL(acquisitionFinished()), this, SLOT(onAcquisitionFinish()));
}


xiFastMovie::~xiFastMovie()
{
    if (pixmapItem != nullptr)
    {
        scene->removeItem(pixmapItem);
        delete pixmapItem;
    }
    if (data != nullptr)
        delete[] data;
    // currFrame8 does not need to be deleted for 8-bit images, since, in that
    // case, currFrame8 is pointing to a frame in data that has just been
    // deleted.
    if (bytesPerSample > 1 && currFrame8 != nullptr)
        delete[] currFrame8;
}


void xiFastMovie::openCamera()
{
    // Open camera device

    XI_RETURN result = xiOpenDevice(0, &xiH);
    if (result != XI_OK)
        throw xiFastMovieException("Could not open camera.");
}


void xiFastMovie::closeCamera()
{
    // Close camera device

    XI_RETURN result = xiCloseDevice(xiH);
    if (result != XI_OK)
        throw xiFastMovieException("Could not close camera.");
}


int xiFastMovie::getParamInt(const char* const param) const
{
    int value;
    XI_RETURN result = xiGetParamInt(xiH, param, &value);
    checkGetParamResult(result, param);
    return value;
}


float xiFastMovie::getParamFloat(const char* const param) const
{
    float value;
    XI_RETURN result = xiGetParamFloat(xiH, param, &value);
    checkGetParamResult(result, param);
    return value;
}


char* xiFastMovie::getParamString(const char* const param,
                                  const uint32_t nBytes) const
{
    char *value = new char[nBytes]();
    XI_RETURN result = xiGetParamString(xiH, param, value, nBytes);
    checkGetParamResult(result, param);
    return value;
}


void xiFastMovie::setParamInt(const char* param, int value)
{
    XI_RETURN result = xiSetParamInt(xiH, param, value);
    checkSetParamResult(result, param);
}


void xiFastMovie::setParamFloat(const char* param, float value)
{
    XI_RETURN result = xiSetParamFloat(xiH, param, value);
    checkSetParamResult(result, param);
}


void xiFastMovie::setPixelFmt(const std::string pixelFmt)
{
    if (pixelFmt == std::string("mono8"))
    {
        this->pixelFmt = "Mono8";
        bytesPerSample = 1;
        bitDepth = 8;
        setParamInt(XI_PRM_IMAGE_DATA_FORMAT, XI_MONO8);
        setParamInt(XI_PRM_OUTPUT_DATA_BIT_DEPTH, 8);
    }
    else if (pixelFmt == std::string("mono10"))
    {
        this->pixelFmt = "Mono10";
        bytesPerSample = 2;
        bitDepth = 10;
        setParamInt(XI_PRM_IMAGE_DATA_FORMAT, XI_MONO16);
        setParamInt(XI_PRM_OUTPUT_DATA_BIT_DEPTH, 10);
        setParamInt(XI_PRM_OUTPUT_DATA_PACKING, XI_ON);
    }
    else if (pixelFmt == std::string("mono12"))
    {
        this->pixelFmt = "Mono12";
        bytesPerSample = 2;
        bitDepth = 12;
        setParamInt(XI_PRM_IMAGE_DATA_FORMAT, XI_MONO16);
        setParamInt(XI_PRM_OUTPUT_DATA_BIT_DEPTH, 12);
    }
    else // invalid format
        throw xiFastMovieException("Allowed pixel formats are \"mono8\", \"mono10\" and \"mono12\".");
}

void xiFastMovie::setFixedFramerate(const float framerate)
{
    // This function only sets the framerate if it is within the limits.

    // Set fixed framerate
    setParamInt(XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE);

    // Check framerate value
    const float min_fps = getParamFloat(XI_PRM_FRAMERATE XI_PRM_INFO_MIN);
    const float max_fps = getParamFloat(XI_PRM_FRAMERATE XI_PRM_INFO_MAX);
    if (framerate < min_fps || framerate > max_fps)
    {
        std::string msg = std::string("Framerate is outside range (")
            + std::to_string(min_fps)
            + std::string(", ")
            + std::to_string(max_fps)
            + std::string(").");
        throw xiFastMovieException(msg);
    }

    // Set framerate
    setParamFloat(XI_PRM_FRAMERATE, framerate);
}


void xiFastMovie::setRefreshRate(const float refreshRate)
{
    if (refreshRate < constants::MIN_DISPLAY_REFRESH_RATE ||
        refreshRate > constants::MAX_DISPLAY_REFRESH_RATE)
    {
        std::string msg = std::string("Refresh framerate is outside range (")
            + std::to_string(constants::MIN_DISPLAY_REFRESH_RATE)
            + std::string(", ")
            + std::to_string(constants::MAX_DISPLAY_REFRESH_RATE)
            + std::string(").");
        throw xiFastMovieException(msg);
    }
    this->refreshRate = refreshRate;
}


void xiFastMovie::printCameraParameters() const
{
    std::cout << "Camera parameters:" << std::endl;
    std::cout << "\t(offset_x, offset_y, width, height): ("
        << getParamInt(XI_PRM_OFFSET_X) << ", "
        << getParamInt(XI_PRM_OFFSET_Y) << ", "
        << getParamInt(XI_PRM_WIDTH) << ", "
        << getParamInt(XI_PRM_HEIGHT) << ")" << std::endl;
    std::cout << "\tExposure (microseconds): " << getParamInt(XI_PRM_EXPOSURE) << std::endl;
    std::cout << "\tFramerate (fps): " << getParamInt(XI_PRM_FRAMERATE) << std::endl;
    std::cout << "\tGain (dB): " << getParamInt(XI_PRM_GAIN) << std::endl << std::flush;
}


void xiFastMovie::acquireMovie(const uint64_t nFrames,
    const std::string outputPath)
{
    timer = new QTimer();
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    timer->start(1000.0 / refreshRate);

    this->show();

    QFuture<void> task = QtConcurrent::run(this, &xiFastMovie::acquireMovieTask,
        nFrames, outputPath);
}


bool xiFastMovie::eventFilter(QObject *target, QEvent *event)
{
    if (target == scene)
    {
        if (event->type() == QEvent::GraphicsSceneWheel)
        {
            QGraphicsSceneWheelEvent* wheelEvent;
            wheelEvent = static_cast<QGraphicsSceneWheelEvent*>(event);
            const int delta = wheelEvent->delta();
            if (delta > 0)
            {
                if (zoomIndex < constants::ZOOM_POW_MAX)
                {
                    ++zoomIndex;
                    updateZoom();
                }
            }
            else
            {
                if (zoomIndex > constants::ZOOM_POW_MIN)
                {
                    --zoomIndex;
                    updateZoom();
                }
            }
        }
    }
    return QMainWindow::eventFilter(target, event);
}


void xiFastMovie::closeEvent(QCloseEvent *event)
{
    if (!(!timer || !timer->isActive()))
    {
        timer->stop();
        timer->deleteLater();
    }
    return QMainWindow::closeEvent(event);
}

void xiFastMovie::checkGetParamResult(XI_RETURN result, const char* param) const
{
    if (result != XI_OK)
    {
        std::string msg = std::string("Could not get parameter ")
            + std::string(param)
            + std::string(".");
        throw xiFastMovieException(msg);
    }
}


void xiFastMovie::checkSetParamResult(XI_RETURN result, const char* param) const
{
    if (result != XI_OK)
    {
        std::string msg = std::string("Could not set parameter ")
            + std::string(param)
            + std::string(".");
        throw xiFastMovieException(msg);
    }
}


std::string xiFastMovie::getDefaultPath() const
{
    // Returns a default path without extension.

    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);
    buffer << std::put_time(&tm, "%Y%m%d_%H%M%S");

    fs::path filename = fs::path(buffer.str());
    fs::path path = fs::current_path() / filename;

    return path.string();
}


void xiFastMovie::acquireMovieTask(const uint64_t nFrames,
                                   std::string outputPath)
{
    // Prepare output path
    if (outputPath.empty())
        outputPath = getDefaultPath();
    else
    {
        if (boost::algorithm::ends_with(outputPath,
            constants::METADATA_FILE_EXT))
            outputPath.erase(outputPath.size() - 5);
    }

    // Print acquisition parameters
    std::cout << "Acquisition parameters: " << std::endl;
    std::cout << "\tFrames: " << nFrames << std::endl;
    std::cout << "\tOutput path: "
        << outputPath
        << constants::METADATA_FILE_EXT << std::endl;
    std::cout << std::endl << std::flush;

    frameWidth = getParamInt(XI_PRM_WIDTH);
    frameHeight = getParamInt(XI_PRM_HEIGHT);
    frameSize = frameWidth * frameHeight * bytesPerSample;
    emit changedGeometry();

    // Image buffer
    XI_IMG image;
    memset(&image, 0, sizeof(image));
    image.size = sizeof(XI_IMG);

    XI_RETURN result;

    // Allocate memory for the movie and metadata
    data = new unsigned char[nFrames * frameSize]();
    // Allocate memory for current frame to display.  Since for 8-bit images,
    // currFrame8 will simply point to a frame in data, allocation is not needed
    // in that case.
    if (bytesPerSample > 1)
        currFrame8 = new unsigned char[frameWidth * frameHeight]();
    uint64_t *frameNumbers = new uint64_t[nFrames]();
    uint64_t *timestamps = new uint64_t[nFrames]();

    // Starting acquisition
    std::cout << "Starting acquisition..." << std::endl;
    result = xiStartAcquisition(xiH);
    if (result != XI_OK)
        throw xiFastMovieException("Could not start acquisition.");

    const uint64_t printNSteps = 10; // Print percentage in n steps
    for (uint64_t i = 0; i < nFrames; i++)
    {
        // Get an image from camera
        result = xiGetImage(xiH, 5000, &image);

        if (result != XI_OK)
            throw xiFastMovieException("Could not get image from camera.");

        const uint64_t cursor = i * frameSize;
        unsigned char *frameData = (unsigned char*)image.bp;
        std::copy(frameData, frameData + frameSize, data + cursor);
        ++currentFrameIndex;

        frameNumbers[i] = image.nframe;
        timestamps[i] = (uint64_t)(image.tsSec) * 1000000 + image.tsUSec;

        // Print progress from time to time
        if ((i + 1) * printNSteps / nFrames - i * printNSteps / nFrames != 0)
            std::cout << 100.0 / printNSteps * (int)((i + 1) * printNSteps / nFrames)
            << " %" << std::endl << std::flush;
    }

    std::cout << "Stopping acquisition..." << std::endl << std::flush;
    result = xiStopAcquisition(xiH);
    if (result != XI_OK)
        throw xiFastMovieException("Could not stop acquisition.");

    std::cout << std::endl;
    // Save data
    std::cout << "Saving data to file..." << std::endl << std::flush;
    FILE *file;
    const std::string path = outputPath + constants::DATA_FILE_EXT;
    errno_t err = fopen_s(&file, path.c_str(), "wb");
    if (err != 0)
        throw xiFastMovieException("Could not open output file.");
    fwrite(data, sizeof(unsigned char), nFrames * frameSize / sizeof(unsigned char), file);
    fclose(file);

    // Save metadata
    const std::string metaPath = outputPath + constants::METADATA_FILE_EXT;
    saveMetadata(metaPath, nFrames, frameNumbers, timestamps);
    std::cout << "Done." << std::endl << std::flush;

    emit acquisitionFinished();
}


void xiFastMovie::saveMetadata(const std::string path,
                               const uint64_t nFrames,
                               const uint64_t *frameNumbers,
                               const uint64_t *timestamps) const
{
    // Saves a movie's metadata

    // Retrieve some parameters
    const int modelID = getParamInt(XI_PRM_DEVICE_MODEL_ID);
    char *deviceName = getParamString(XI_PRM_DEVICE_NAME, 20);
    char *deviceSN = getParamString(XI_PRM_DEVICE_SN, 20);
    char *apiVersion = getParamString(XI_PRM_API_VERSION, 20);
    char *drvVersion = getParamString(XI_PRM_DRV_VERSION, 20);
    char *mcu1Version = getParamString(XI_PRM_MCU1_VERSION, 20);
    // char *mcu2Version = getParamString(XI_PRM_MCU2_VERSION, 20);
    char *fpga1Version = getParamString(XI_PRM_FPGA1_VERSION, 20);
    char *hwRevision = getParamString(XI_PRM_HW_REVISION, 20);
    const float framerate = getParamFloat(XI_PRM_FRAMERATE);
    const int offsetX = getParamInt(XI_PRM_OFFSET_X);
    const int offsetY = getParamInt(XI_PRM_OFFSET_Y);
    const int exposure = getParamInt(XI_PRM_EXPOSURE);
    const float gain = getParamFloat(XI_PRM_GAIN);

    std::ofstream metaFile(path);
    if (metaFile.is_open())
    {
        metaFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        if (std::strcmp(constants::VERSION, constants::TARGET_VERSION) == 0)
            metaFile << "<movie_metadata app_name=\""
                << constants::APP_NAME << "\" "
                << "version=\"" << constants::VERSION << "\">\n";
        else
            metaFile << "<movie_metadata app_name=\""
                << constants::APP_NAME << "\" "
                << "version=\"" << constants::VERSION
                << "\" target_version=\"" << constants::TARGET_VERSION << "\">\n";

        // Print header
        metaFile << "\t<header>\n";
        metaFile << "\t\t<camera>\n";
        metaFile << "\t\t\t<device_name>" << deviceName << "</device_name>\n";
        metaFile << "\t\t\t<model_id>" << modelID << "</model_id>\n";
        metaFile << "\t\t\t<device_sn>" << deviceSN << "</device_sn>\n";
        metaFile << "\t\t\t<mcu1_firmware_version>" << mcu1Version << "</mcu1_firmware_version>\n";
        // metaFile << "\t\t\t<mcu2_firmware_version>" << mcu2Version << "</mcu2_firmware_version>\n";
        metaFile << "\t\t\t<fpga1_firmware_version>" << fpga1Version << "</fpga1_firmware_version>\n";
        metaFile << "\t\t\t<hardware_revision>" << hwRevision << "</hardware_revision>\n";
        metaFile << "\t\t</camera>\n";
        metaFile << "\t\t<api_version>" << apiVersion << "</api_version>\n";
        metaFile << "\t\t<driver_version>" << drvVersion << "</driver_version>\n";
        metaFile << "\t\t<offset_x>" << offsetX << "</offset_x>\n";
        metaFile << "\t\t<offset_y>" << offsetY << "</offset_y>\n";
        metaFile << "\t\t<width>" << frameWidth << "</width>\n";
        metaFile << "\t\t<height>" << frameHeight << "</height>\n";
        metaFile << "\t\t<pixel_format>" << pixelFmt << "</pixel_format>\n";
        metaFile << "\t\t<endianness>little</endianness>\n";
        metaFile << "\t\t<framerate>" << framerate << "</framerate>\n";
        metaFile << "\t\t<exposure>" << exposure << "</exposure>\n";
        metaFile << "\t\t<gain>" << gain << "</gain>\n";
        metaFile << "\t</header>\n";

        // Print frames metadata
        metaFile << "\t<frames>\n";
        for (uint64_t i = 0; i < nFrames; i++)
        {
            metaFile << "\t\t<frame frame=\"" << frameNumbers[i]
                << "\" timestamp=\"" << timestamps[i] << "\" />\n";
        }
        metaFile << "\t</frames>\n";

        // Print footer
        metaFile << "</movie_metadata>\n";

        metaFile.close();
    }
    else
    {
        std::string msg = std::string("Unable to open ")
            + path
            + std::string(".");
        throw xiFastMovieException(msg);
    }
}


void xiFastMovie::updateZoom()
{
    if (pixmapItem)
        pixmapItem->setScale(pow(constants::ZOOM_BASE, zoomIndex));
    updateGeometry();
}


void xiFastMovie::onAcquisitionFinish()
{
    close();
}


void xiFastMovie::updateDisplay()
{
    if (currentFrameIndex >= 0)
    {
        size_t cursor = currentFrameIndex * frameSize;
        if (bytesPerSample == 1)
            currFrame8 = data + cursor;
        else
        {
            for (size_t k = 0; k < frameWidth * frameHeight; k++)
            {
                uint8_t bitsShift = bitDepth - 8;
                uint16_t val = ((uint16_t) data[cursor + 2 * k + 1] << 8)
                    + (uint16_t) data[cursor + 2 * k]; // Little endian
                currFrame8[k] = val >> bitsShift;
            }
        }
        QImage image = QImage(currFrame8, frameWidth, frameHeight,
                              QImage::Format_Grayscale8);
        pixmapItem->setPixmap(QPixmap::fromImage(image));
    }
}


void xiFastMovie::updateGeometry()
{
    const uint32_t scaledWidth = frameWidth * pow(constants::ZOOM_BASE, zoomIndex);
    const uint32_t scaledHeight = frameHeight * pow(constants::ZOOM_BASE, zoomIndex);

    uint32_t windowWidth = std::max<uint32_t>(scaledWidth, constants::MIN_WINDOW_WIDTH);
    uint32_t windowHeight = std::max<uint32_t>(scaledHeight, constants::MIN_WINDOW_HEIGHT);
    windowWidth = std::min<uint32_t>(windowWidth, QApplication::desktop()->rect().width());
    windowHeight = std::min<uint32_t>(windowHeight, QApplication::desktop()->rect().height());

    QRectF rect(0, 0, scaledWidth, scaledHeight);
    view->setSceneRect(rect);
    setFixedSize(windowWidth, windowHeight);
}
