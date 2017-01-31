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

#include <string>
#include <exception>

#ifdef WIN32
#include "xiApi.h" // Windows
#else
#include <xiApi.h> // Linux, OSX
#endif

#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QResizeEvent>
#include <QEvent>


class xiFastMovie : public QMainWindow
{
    Q_OBJECT

private:
    HANDLE xiH;

    QGraphicsScene* scene;
    QGraphicsView* view;
    QGraphicsPixmapItem* pixmapItem;
    QTimer* timer;

    int64_t currentFrameIndex;
    uint32_t frameWidth;
    uint32_t frameHeight;
    uint64_t frameSize;

    std::string pixelFmt;
    uint8_t bytesPerSample;
    uint8_t bitDepth;

    float refreshRate;

    unsigned char* data;
    unsigned char* currFrame8;

    int32_t zoomIndex;

    void checkGetParamResult(XI_RETURN result, const char* param) const;
    void checkSetParamResult(XI_RETURN result, const char* param) const;
    std::string getDefaultPath() const;
    void acquireMovieTask(const uint64_t nFrames, const std::string outputPath);
    void saveMetadata(const std::string path,
                      const uint64_t nFrames,
                      const uint64_t* frameNumbers,
                      const uint64_t* timestamps) const;
    void updateZoom();

private slots:
    void onAcquisitionFinish();
    void updateDisplay();
    void updateGeometry();


public:
    explicit xiFastMovie(QWidget* parent = 0);
    ~xiFastMovie();

    // Camera interfacing
    void openCamera();
    void closeCamera();
    //
    int getParamInt(const char* const param) const;
    float getParamFloat(const char* const param) const;
    char* getParamString(const char* const param, const uint32_t nBytes) const;
    //
    void setParamInt(const char* param, int value);
    void setParamFloat(const char* param, float value);
    void setPixelFmt(const std::string);
    void setFixedFramerate(const float framerate);
    void setRefreshRate(const float refreshRate);
    //
    void printCameraParameters() const;
    //
    void acquireMovie(const uint64_t nFrames, const std::string outputPath);

    class xiFastMovieException : public std::exception
    {
    private:
        std::string err_msg;

    public:
        xiFastMovieException(const char *msg) : err_msg(msg) {};
        xiFastMovieException(const std::string msg) : err_msg(msg) {};
        ~xiFastMovieException() noexcept {};

        const char *what() const noexcept override { return this->err_msg.c_str(); };
    };

protected:
    virtual bool eventFilter(QObject *target, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void acquisitionFinished();
    void changedGeometry();
};
