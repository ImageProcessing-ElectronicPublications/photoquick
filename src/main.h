#pragma once
/*
This file is a part of qmageview program, which is GPLv3 licensed
*/

#include "ui_mainwindow.h"
#include "canvas.h"
#include <QSettings>
#include <QTimer>


class Window : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT
public:
    Window();
    //~Window();
    void openImage(QString filename);
    void adjustWindowSize(bool animation=false);
    //Variables declaration
    Canvas *canvas;
    QSettings settings;
    int screen_width, screen_height, offset_x, offset_y, btnboxwidth;
    QTimer *timer;      // Slideshow timer
    QString filepath;
private:
    void connectSignals();
    float getOptimumScale(QImage img);
    void disableButtons(bool disable);
    void closeEvent(QCloseEvent *ev);
    QList<QWidget *> crop_widgets;
public slots:
    void openFile();
    void saveFile();
    void resizeImage();
    void cropImage();
    void cancelCropping();
    void addBorder();
    void createPhotoGrid();
    void toGrayScale();
    void toBlacknWhite();
    void adaptiveThresh();
    void blur();
    void sharpenImage();
    void reduceSpeckleNoise();
    void sigmoidContrast(); // Enhance low light images
    void whiteBalance();
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
    void mirror();
    void playSlideShow(bool checked);
    void updateStatus();
};

void waitFor(int millisec);
// rounds up a number to given decimal point
float roundOff(float num, int dec);
