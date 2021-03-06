#pragma once
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QImage>
#include <QTimer>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <cmath>
#include "common.h"
#include "filters.h"

#ifndef __PHOTOQUIK_DIALOGS
#define __PHOTOQUIK_DIALOGS

// Dialog to set JPG image quality for saving
class QualityDialog : public QDialog
{
    Q_OBJECT
public:
    QImage image;
    QGridLayout *layout;
    QLabel *qualityLabel, *sizeLabel;
    QSpinBox *qualitySpin;
    QCheckBox *showSizeCheck;
    QDialogButtonBox *btnBox;
    QTimer *timer;
    QualityDialog(QWidget *parent, QImage &img);
public slots:
    void checkFileSize();
    void toggleCheckSize(bool checked);
};

// dialog to choose paper size
class PaperSizeDialog : public QDialog
{
public:
    QVBoxLayout *vLayout;
    QLabel *label;
    QStringList items;
    QComboBox *combo;
    QCheckBox *landscape;
    QDialogButtonBox *btnBox;
    PaperSizeDialog(QWidget *parent, bool landscapeMode);
};

// dialog to choose border width and size
class ExpandBorderDialog : public QDialog
{
public:
    QVBoxLayout *vLayout;
    QLabel *label, *label2;
    QComboBox *combo;
    QStringList items;
    QSpinBox *widthSpin;
    QDialogButtonBox *btnBox;
    ExpandBorderDialog(QWidget *parent, int border_w);
};

// Preview Dialog for filter functions.
// This is Abstract and must be reimplemented
class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    QImage image;
    float scale;
    QTimer *timer;
    // if the filter applied on scaled image looks same, then image from
    // canvas pixmap is passed, and scale is set 1.0
    // else, the original image is passed, and scale is set to canvas->scale
    PreviewDialog(QLabel *parent, QImage img, float scale);
    void preview(QImage);
public slots:
    void onValueChange();
    // implement run() in subclass. apply filter, then call preview()
    virtual void run() = 0;
signals:
    void previewRequested(const QPixmap&);
};

class LensDialog : public PreviewDialog
{
public:
    float main=20.0;
    float edge=0;
    float zoom=10.0;
    QGridLayout *layout;
    QLabel *label0, *label1, *label2; 
    QDoubleSpinBox *mainSpin;
    QDoubleSpinBox *edgeSpin;
    QDoubleSpinBox *zoomSpin;
    QDialogButtonBox *btnBox;
    LensDialog(QLabel *parent, QImage img, float scale);
    void run();
};

class ThresholdDialog : public PreviewDialog
{
public:
    int thresh;
    QVBoxLayout *layout;
    QLabel *label0;
    QSpinBox *thresholdSpin;
    QDialogButtonBox *btnBox;
    ThresholdDialog(QLabel *parent, QImage img, float scale);
    void run();
};

class GammaDialog : public PreviewDialog
{
public:
    float gamma = 1.6;
    QVBoxLayout *layout;
    QLabel *label0;
    QDoubleSpinBox *gammaSpin;
    QDialogButtonBox *btnBox;
    GammaDialog(QLabel *parent, QImage img, float scale);
    void run();
};

class DeWarpDialog : public QDialog
{
public:
    int countn;
    QVBoxLayout *vLayout;
    QLabel *countLabel;
    QSpinBox *countSpin;
    QDialogButtonBox *btnBox;
    DeWarpDialog(QWidget *parent);
};

#endif /* __PHOTOQUIK_DIALOGS */
