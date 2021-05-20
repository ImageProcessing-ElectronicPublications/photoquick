#pragma once
#include "canvas.h"
#include "ui_resize_dialog.h"
#include <QStatusBar>
#include <QDoubleSpinBox>
#include <QCheckBox>

typedef enum {
    NO_RATIO,
    FIXED_RATIO,
    FIXED_RESOLUTION
} CropMode;

// the crop manager
class Crop : public QObject
{
    Q_OBJECT
public:
    Crop(Canvas *canvas, QStatusBar *statusbar);
    Canvas *canvas;
    QStatusBar *statusbar;
private:
    QPixmap pixmap;
    bool mouse_pressed;
    QPoint topleft, btmright;   // corner pos at the time of mouse click
    QPoint p1, p2;              // corner pos while mouse moves
    QPoint clk_pos;
    int clk_area;
    float scaleX, scaleY; // using two scales give better accuracy on scaled canvas
    CropMode crop_mode;
    int fixed_width, fixed_height; // in FIXED_RESOLUTION mode
    float ratio_w, ratio_h;        // in FIXED_RATIO mode
    QList<QWidget *> crop_widgets;
    void drawCropBox();
private slots:
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
    void setCropMode(QAction *action);
    void crop();
    void finish();
signals:
    void finished();
};

class CropRatioDialog : public QDialog
{
public:
    CropRatioDialog(QWidget *parent, double w, double h);
    QDoubleSpinBox *widthSpin;
    QDoubleSpinBox *heightSpin;
};

class CropResolutionDialog : public QDialog
{
public:
    CropResolutionDialog(QWidget *parent, int img_w, int img_h);
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
};
// _____________________________________________________________________
// perspective transform manager

class PerspectiveTransform : public QObject
{
    Q_OBJECT
public:
    PerspectiveTransform(Canvas *canvas, QStatusBar *statusbar);
    Canvas *canvas;
    QStatusBar *statusbar;
private:
    QPixmap pixmap;
    bool mouse_pressed, fisometric;
    QPolygon pt, p;
    QPoint clk_pos;
    int clk_area;
    float scaleX, scaleY;
    QList<QWidget *> crop_widgets;
    void drawCropBox();
private slots:
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
    void isomode();
    void transform();
    void finish();
signals:
    void finished();
};


void calcArc(QPoint center, QPoint from, QPoint to, QPoint through,
                            float &start, float &span);

// _____________________________________________________________________
// ResizeDialog object to get required image size

class ResizeDialog : public QDialog, public Ui_ResizeDialog
{
    Q_OBJECT
public:
    ResizeDialog(QWidget *parent, int img_width, int img_height);
    int orig_width, orig_height;
    float orig_ratio = 1;
public slots:
    void toggleAdvanced(bool checked);
    void onValueChange(int value);
    void onValueChange(double value) {onValueChange(int(value));}
    void ratioTextChanged();
};

// _____________________________________________________________________
// simple dewarping transform manager

class DeWarping : public QObject
{
    Q_OBJECT
public:
    DeWarping(Canvas *canvas, QStatusBar *statusbar, int count);
    Canvas *canvas;
    QStatusBar *statusbar;
    QPushButton *cropnowBtn, *cropcancelBtn;
private:
    QPixmap pixmap;
    bool mouse_pressed;
    QPolygon lnht, lnh, lndt, lnd;
    QPoint clk_pos;
    int clk_area_h, clk_area_d;
    float scaleX, scaleY, areah, aread, ylnh, ylnd;
    QList<QWidget *> crop_widgets;
    void drawDeWarpLine();
private slots:
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
    void transform();
    void finish();
signals:
    void finished();
};

