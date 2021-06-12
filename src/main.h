#pragma once
#include <cmath>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QPainter>
#include <QDesktopWidget>
#include <QSettings>
#include <QMenu>
#include <QRegExp>
#include <QBuffer>
#include <QImageWriter>
#include "ui_mainwindow.h"
#include "canvas.h"
#include "common.h"
#include "plugin.h"
#include "dialogs.h"
#include "transform.h"
#include "photogrid.h"
#include "inpaint.h"
#include "iscissor.h"
#include "filters.h"
#include "pdfwriter.h"

#ifndef __PHOTOQUIK_MAIN
#define __PHOTOQUIK_MAIN

typedef enum
{
    FILE_BUTTON,
    VIEW_BUTTON,
    EDIT_BUTTON
} ButtonType;


class Window : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT
public:
    Window();
    void openImage(QString filename);
    void saveImage(QString filename);
    void adjustWindowSize(bool animation=false);
    QImage resizeImageBicub (QImage, unsigned, unsigned);
    //Variables declaration
    Canvas *canvas;
    ImageData data;
    int screen_width, screen_height, offset_x, offset_y, btnboxwidth;
    QTimer *timer;      // Slideshow timer
    QMap<QString, QMenu*> menu_dict;
private:
    void connectSignals();
    float fitToScreenScale(QImage img);
    float fitToWindowScale(QImage img);
    void disableButtons(ButtonType type, bool disable);
    void closeEvent(QCloseEvent *ev);
    void addMaskWidget();
    void blurorbox(int method);
public slots:
    void openFile();
    void overwrite();
    void saveAs();
    void saveACopy();
    void autoResizeAndSave();
    void exportToPdf();
    void deleteFile();
    void reloadImage();
    void resizeImage();
    void cropImage();
    // tranform
    void mirror();
    void perspectiveTransform();
    void deWarping();
    void deOblique();
    void lensDistort();
    // decorate
    void addBorder();
    void expandImageBorder();
    void createPhotoGrid();
    void createPhotoCollage();
    // tools
    void magicEraser();
    void iScissor();
    void removeMaskWidget();
    // filters
    void toGrayScale();
    void applyThreshold();
    void adaptiveThresh();
    void box();
    void blur();
    void deblur();
    void sharpenImage();
    void reduceSpeckleNoise();
    void removeDust();
    void sigmoidContrast(); // Enhance low light images
    void enhanceLight();
    void gammaCorrection();
    void whiteBalance();
    void enhanceColors();
    void vignetteFilter();
    void pencilSketchFilter();
    void grayWorldFilter();
    // info menu
    void imageInfo();
    void showAbout();
    // file and view options
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
    void playPause();
    // others
    void loadPlugins();
    void resizeToOptimum();
    void showNotification(QString title, QString message);
    void onEditingFinished();
    void updateStatus();
};

QString getNextFileName(QString current);
QString getNewFileName(QString filename);

#endif /* __PHOTOQUIK_MAIN */
