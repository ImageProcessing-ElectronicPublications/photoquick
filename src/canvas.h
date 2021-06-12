#pragma once
/* Image Label Object to display the image. */
#include "plugin.h"
#include <cmath>
#include <QDebug>
#include <QLabel>
#include <QMovie>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QCursor>
#include <QSizePolicy>
#include <QTransform>
#include <QPainter>

#ifndef __PHOTOQUIK_CANVAS
#define __PHOTOQUIK_CANVAS

//This is the widget responsible for displaying image
class Canvas : public QLabel
{
    Q_OBJECT
public:
    Canvas(QScrollArea *scrollArea, ImageData *img_dat);
    void setAnimation(QMovie *anim);
    void setImage(QImage img);
    void setMask(QImage mask);
    void clearMask();
    void rotate(int degree, Qt::Axis axis=Qt::ZAxis);
    // Variables
    ImageData *data;
    QImage mask;// 1 bpp binary mask image of format MonoLSB, 0=unmasked, 1=masked
    QImage tmp_image;// to restore masked area in data->image after applying filters
    bool animation = false;
    float scale;
    bool drag_to_scroll;    // if click and drag moves image
private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    // Variables
    bool mouse_pressed;
    int v_scrollbar_pos, h_scrollbar_pos;
    QPoint clk_global;
    QScrollBar *vScrollbar, *hScrollbar;
public slots:
    void showScaled();
    void invertMask();
signals:
    void mousePressed(QPoint);
    void mouseReleased(QPoint);
    void mouseMoved(QPoint);
    void imageUpdated();
};


// It is a drawing area for some dialogs

class PaintCanvas : public QLabel
{
    Q_OBJECT
public:
    PaintCanvas(QWidget *parent);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void setImage(QImage image);
signals:
    void mousePressed(QPoint);
    void mouseMoved(QPoint);
    void mouseReleased(QPoint);
};

QCursor roundCursor(int width);

// a QList<HistoryItem> is used as Undo or Redo stack
typedef struct {
    int x;
    int y;
    QImage image;// this image is replaced by new image at pos (x,y)
} HistoryItem;

#endif /* __PHOTOQUIK_CANVAS */
