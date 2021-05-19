/*
This file is a part of photoquick program, which is GPLv3 licensed
*/
#include "transform.h"
#include "common.h"
#include <QSettings>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QRect>
#include <cmath>


//Mean for Isometric mode Un-tilt (PerspectiveTransform)
QPoint meanx2(QPolygon p)
{
    int i, n = p.count();
    float mx = 0.0f, my = 0.0f;
    if (n > 0)
    {
        for (i = 0; i < n; i++)
        {
            mx += (float)p[i].x();
            my += (float)p[i].y();
        }
        mx /= (float)n;
        my /= (float)n;
    }
    return QPoint((int)(mx + 0.5f), (int)(my + 0.5f));
}
//StDev for Isometric mode Un-tilt (PerspectiveTransform)
QPoint stdevx2(QPolygon p)
{
    int i, n = p.count();
    float mx = 0.0f, my = 0.0f, sx = 0.0f, sy = 0.0f;
    float dx, dy;
    if (n > 0)
    {
        for (i = 0; i < n; i++)
        {
            mx += (float)p[i].x();
            my += (float)p[i].y();
        }
        mx /= (float)n;
        my /= (float)n;
        for (i = 0; i < n; i++)
        {
            dx = mx - (float)p[i].x();
            dy = my - (float)p[i].y();
            sx += dx * dx;
            sy += dy * dy;
        }
        sx /= (float)n;
        sy /= (float)n;
        sx = sqrtf(sx);
        sy = sqrtf(sy);
    }
    return QPoint((int)(sx + 0.5f), (int)(sy + 0.5f));
}

// ******************************************************************* |
//                         Crop Manager
// ------------------------------------------------------------------- |
Crop:: Crop(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    topleft = QPoint(0,0);
    btmright = QPoint(pixmap.width()-1, pixmap.height()-1);
    p1 = QPoint(topleft);   // temporary topleft point
    p2 = QPoint(btmright);
    ratio_w = 3.5;
    ratio_h = 4.5;
    crop_mode = NO_RATIO;
    // add buttons
    QPushButton *setRatioBtn = new QPushButton("Set Ratio", statusbar);
    statusbar->addPermanentWidget(setRatioBtn);
    QMenu *ratioMenu = new QMenu(setRatioBtn);
    QActionGroup *ratioActions = new QActionGroup(ratioMenu);
    QAction *action1 = ratioActions->addAction("No Ratio");
    QAction *action2 = ratioActions->addAction("Ratio - Custom");
    QAction *action3 = ratioActions->addAction("Ratio - 1:1");
    QAction *action4 = ratioActions->addAction("Ratio - 3:4");
    QAction *action5 = ratioActions->addAction("Ratio - 2.5:3.5");
    QAction *action6 = ratioActions->addAction("Ratio - 3.5:4.5");
    QAction *action7 = ratioActions->addAction("Fixed Resolution");
    action1->setCheckable(true);
    action2->setCheckable(true);
    action3->setCheckable(true);
    action4->setCheckable(true);
    action5->setCheckable(true);
    action6->setCheckable(true);
    action7->setCheckable(true);
    action1->setChecked(true);
    ratioMenu->addAction(action1);
    ratioMenu->addAction(action2);
    ratioMenu->addAction(action3);
    ratioMenu->addAction(action4);
    ratioMenu->addAction(action5);
    ratioMenu->addAction(action6);
    ratioMenu->addAction(action7);
    setRatioBtn->setMenu(ratioMenu);
    connect(ratioActions, SIGNAL(triggered(QAction*)), this, SLOT(setCropMode(QAction*)));
    QWidget *spacer = new QWidget(statusbar);
    spacer->setMinimumWidth(40);
    statusbar->addPermanentWidget(spacer);
    QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(crop()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << setRatioBtn << spacer << cropnowBtn << cropcancelBtn;
    drawCropBox();
}

void
Crop:: onMousePress(QPoint pos)
{
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft, QSize(60, 60)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(btmright, QSize(-60, -60)).contains(clk_pos))
        clk_area = 2;   // bottom right corner clicked
    else if (QRect(topleft, btmright).contains(clk_pos)) // clicked inside cropbox
        clk_area = 3;   // inside cropbox
    else
        clk_area = 0;   // outside cropbox
}

void
Crop:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    topleft = p1;
    btmright = p2;
}

void
Crop:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    float imgAspect = float(canvas->data->image.width())/canvas->data->image.height();
    float boxAspect = ratio_w/ratio_h;
    switch (clk_area) {
    case 1 : { // Top left corner is clicked
        if (crop_mode==FIXED_RESOLUTION)
            break;
        QPoint new_p1 = topleft + moved;
        p1 = QPoint(MAX(0, new_p1.x()), MAX(0, new_p1.y()));
        if (crop_mode==FIXED_RATIO) {
            if (imgAspect>boxAspect) p1.setX(round(p2.x() - (p2.y()-p1.y()+1)*boxAspect -1));
            else p1.setY(round(p2.y() - (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 2 : { // Bottom right corner is clicked
        if (crop_mode==FIXED_RESOLUTION)
            break;
        QPoint new_p2 = btmright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_p2.x()), MIN(last_pt.y(), new_p2.y()));
        if (crop_mode==FIXED_RATIO) {
            if (imgAspect>boxAspect) p2.setX(round(p1.x() + (p2.y()-p1.y()+1)*boxAspect -1));
            else p2.setY(round(p1.y() + (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 3 : { // clicked inside cropbox but none of the corner selected.
        int min_dx, max_dx, min_dy, max_dy, dx, dy;
        min_dx = -topleft.x();
        max_dx = last_pt.x()-btmright.x();
        min_dy = -topleft.y();
        max_dy = last_pt.y()-btmright.y();
        dx = (moved.x() < 0) ? MAX(moved.x(), min_dx) : MIN(moved.x(), max_dx);
        dy = (moved.y() < 0) ? MAX(moved.y(), min_dy) : MIN(moved.y(), max_dy);
        p1 = topleft + QPoint(dx, dy);
        p2 = btmright + QPoint(dx, dy);
        break;
    }
    }
    drawCropBox();
}

void
Crop:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPixmap pm_box = pm.copy(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    QPainter painter(&pm);
    painter.fillRect(0,0, pm.width(), pm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1.x(), p1.y(), pm_box);
    painter.drawRect(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    painter.drawRect(p1.x(), p1.y(), 59, 59);
    painter.drawRect(p2.x(), p2.y(), -59, -59);
    painter.setPen(Qt::white);
    painter.drawRect(p1.x()+1, p1.y()+1, 57, 57);
    painter.drawRect(p2.x()-1, p2.y()-1, -57, -57);
    painter.end();
    canvas->setPixmap(pm);
    QString text = "Resolution : %1x%2";
    if (crop_mode==FIXED_RESOLUTION)
        statusbar->showMessage(text.arg(fixed_width).arg(fixed_height));
    else {
        int width = round((p2.x() - p1.x() + 1)/scaleX);
        int height = round((p2.y() - p1.y() + 1)/scaleY);
        statusbar->showMessage(text.arg(width).arg(height));
    }
}

void
Crop:: crop()
{
    int w,h;
    if (crop_mode==FIXED_RESOLUTION) {
        w = fixed_width;
        h = fixed_height;
    }
    else {
        w = round((btmright.x()-topleft.x()+1)/scaleX);
        h = round((btmright.y()-topleft.y()+1)/scaleY);
    }
    QImage img = canvas->data->image.copy(round(topleft.x()/scaleX), round(topleft.y()/scaleY), w, h);
    canvas->data->image = img;
    finish();
}

void
Crop:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    emit finished();
    this->deleteLater();
}

void
Crop:: setCropMode(QAction *action)
{
    if (action->text()==QString("Fixed Resolution")) {
        CropResolutionDialog *res_dlg = new CropResolutionDialog(canvas,canvas->data->image.width(),canvas->data->image.height());
        res_dlg->exec();
        fixed_width = res_dlg->widthSpin->value();
        fixed_height = res_dlg->heightSpin->value();
        p1 = topleft = QPoint(0,0);
        p2 = btmright = QPoint( round((fixed_width-1)*scaleX), round((fixed_height-1)*scaleY) );
        crop_mode = FIXED_RESOLUTION;
    }
    else if (action->text()==QString("Ratio - Custom")) {
        QSettings settings;
        ratio_w = settings.value("CropRatioX", 3.5).toFloat();
        ratio_h = settings.value("CropRatioY", 4.5).toFloat();
        CropRatioDialog *dlg = new CropRatioDialog(canvas, ratio_w, ratio_h);
        dlg->exec();
        ratio_w = dlg->widthSpin->value();
        ratio_h = dlg->heightSpin->value();
        settings.setValue("CropRatioX", ratio_w);
        settings.setValue("CropRatioY", ratio_h);
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 3.5:4.5")) {
        ratio_w = 3.5;
        ratio_h = 4.5;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 2.5:3.5")) {
        ratio_w = 2.5;
        ratio_h = 3.5;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 3:4")) {
        ratio_w = 3.0;
        ratio_h = 4.0;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 1:1")) {
        ratio_w = 1.0;
        ratio_h = 1.0;
        crop_mode = FIXED_RATIO;
    }
    else
        crop_mode = NO_RATIO;
    // draw crop box in selected ratio
    if (crop_mode == FIXED_RATIO) {
        int w = p2.x()-p1.x()+1;
        int h = p2.y()-p1.y()+1;
        float aspect = w/(float)h;
        float newAspect = ratio_w/ratio_h;
        if (aspect>newAspect)
            p2 = QPoint(p1.x()+ h*newAspect-1, p2.y());
        else
            p2 = QPoint(p2.x(), p1.y()+ w/newAspect -1);
        btmright = p2;
    }
    drawCropBox();
}


CropRatioDialog::
CropRatioDialog(QWidget *parent, double w, double h) : QDialog(parent)
{
    setWindowTitle("Enter Ratio");
    resize(200, 50);
    widthSpin = new QDoubleSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setDecimals(1);
    widthSpin->setSingleStep(0.1);
    widthSpin->setRange(0.1, 9.9);
    widthSpin->setValue(w);
    heightSpin = new QDoubleSpinBox(this);
    heightSpin->setAlignment(Qt::AlignHCenter);
    heightSpin->setDecimals(1);
    heightSpin->setSingleStep(0.1);
    heightSpin->setRange(0.1, 9.9);
    heightSpin->setValue(h);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *label1 = new QLabel("Width : Height", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(label1, 0,0,1,2);
    layout->addWidget(widthSpin, 1,0,1,1);
    layout->addWidget(heightSpin, 1,1,1,1);
    layout->addWidget(btnBox, 2,0,1,2);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
}

CropResolutionDialog::
CropResolutionDialog(QWidget *parent, int w, int h) : QDialog(parent)
{
    setWindowTitle("Enter Ratio");
    resize(200, 50);
    widthSpin = new QSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setRange(1, w);
    widthSpin->setValue(w);
    heightSpin = new QSpinBox(this);
    heightSpin->setAlignment(Qt::AlignHCenter);
    heightSpin->setRange(1, h);
    heightSpin->setValue(h);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *label1 = new QLabel("Width", this);
    QLabel *label2 = new QLabel("Height", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(label1, 0,0,1,1);
    layout->addWidget(label2, 0,1,1,1);
    layout->addWidget(widthSpin, 1,0,1,1);
    layout->addWidget(heightSpin, 1,1,1,1);
    layout->addWidget(btnBox, 2,0,1,2);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
}

// ******************************************************************* |
//                         Perspective Transform
// ------------------------------------------------------------------- |
PerspectiveTransform::
PerspectiveTransform(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    fisometric = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    int i, n = 4, sx = 1, sy = 1, xt, yt;
    for (i = 0; i < n; i++)
    {
        xt = (sx < 0) ? (pixmap.width() - 1) : 0;
        yt = (sy < 0) ? (pixmap.height() - 1) : 0;
        p << QPoint(xt, yt);
        pt << QPoint(xt, yt);
        sx = sx - sy;
        sy += sx; //  1, -1, -1,  1
        sx -= sy; //  1,  1, -1, -1
    }
    // add buttons
    QCheckBox *checkIso = new QCheckBox("Isometric", statusbar);
    statusbar->addPermanentWidget(checkIso);
    QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(checkIso, SIGNAL(clicked()), this, SLOT(isomode()));
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << checkIso << cropnowBtn << cropcancelBtn;
    statusbar->showMessage("Drag corners to fit edges around tilted image/document");
    drawCropBox();
}

void
PerspectiveTransform:: onMousePress(QPoint pos)
{
    int i, n, sx = 1, sy = 1;
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    clk_area = 0;
    n = pt.count();
    for (i = 0; i < n; i++)
    {
        if (QRect(pt[i], QSize(sx * 60, sy * 60)).contains(clk_pos))
            clk_area = i + 1;
        sx = sx - sy;
        sy += sx; //  1, -1, -1,  1
        sx -= sy; //  1,  1, -1, -1
    }
}

void
PerspectiveTransform:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    pt = p;
}

void
PerspectiveTransform:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    QPoint new_pt;
    if (clk_area > 0)
    {
        new_pt = pt[clk_area - 1] + moved;
        p[clk_area - 1] = QPoint(MIN(last_pt.x(), MAX(0, new_pt.x())), MIN(last_pt.y(), MAX(0, new_pt.y())));
    }
    drawCropBox();
}

void
PerspectiveTransform:: drawCropBox()
{
    float start, span;
    int i, n, j0, j1, j2, j3;
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    QPolygonF polygon;
    polygon << p[0] << p[1] << p[2] << p[3];
    painter.drawPolygon(polygon);
    n = p.count();
    if (n == 4)
    {
        for (i = 0; i < n; i++)
        {
            j0 = i;
            j3 = (i + 2) % 4;
            j1 = ((i - 1) < 0) ? (1 - i) : (i - 1);
            j2 = (j1 + 2) % 4;

            calcArc(p[j0], p[j1], p[j2], p[j3], start, span);
            painter.drawArc(p[i].x() - 30, p[i].y() - 30, 60, 60, 16 * start, 16 * span);
        }
        painter.setPen(Qt::white);
        polygon.clear();
        polygon << (p[0] + QPoint(1,1)) << (p[1] + QPoint(-1,1)) << (p[2] + QPoint(-1,-1)) << (p[3] + QPoint(1,-1));
        painter.drawPolygon(polygon);
        painter.end();
        canvas->setPixmap(pm);
    }
}

void
PerspectiveTransform:: isomode()
{
    fisometric = !fisometric;
}

void
PerspectiveTransform:: transform()
{
    int i, n, min_w, min_h, max_w, max_h;
    QPoint mxy;
    QPoint sxy;
    n = p.count();
    if (n == 4)
    {
        for (i = 0; i < n; i++)
            p[i] = QPoint(p[i].x() / scaleX, p[i].y() / scaleY);
        if (fisometric)
        {
            mxy = meanx2(p);
            sxy = stdevx2(p);
            min_w = mxy.x() - sxy.x();
            min_h = mxy.y() - sxy.y();
            max_w = mxy.x() + sxy.x();
            max_h = mxy.y() + sxy.y();
        }
        else
        {
            min_w = 0;
            min_h = 0;
            max_w = MAX(p[1].x() - p[0].x(), p[2].x() - p[3].x());
            max_h = MAX(p[3].y() - p[0].y(), p[2].y() - p[1].y());
        }
        QPolygonF mapFrom;
        mapFrom << p[0] << p[1] << p[2] << p[3];
        QPolygonF mapTo;
        mapTo << QPointF(min_w, min_h) << QPointF(max_w, min_h) << QPointF(max_w, max_h) << QPointF(min_w, max_h);
        QTransform tfm;
        QTransform::quadToQuad(mapFrom, mapTo, tfm);
        QImage img = canvas->data->image.transformed(tfm, Qt::SmoothTransformation);
        QTransform trueMtx = QImage::trueMatrix(tfm,canvas->data->image.width(),canvas->data->image.height());
        if (fisometric)
        {
            canvas->data->image = img;
        }
        else
        {
            pt[0] = trueMtx.map(p[0]);
            pt[2] = trueMtx.map(p[2]);
            canvas->data->image = img.copy(QRect(pt[0], pt[2]));
        }
    }
    finish();
}

void
PerspectiveTransform:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    emit finished();
    this->deleteLater();
}

// arc is drawn from p1 to p2 through p3
// p3 is at diagonal corner of p
// angle is  drawn counter clock-wise, and direction of y axis is
// upward, while direction of image y axis is downward
void calcArc(QPoint p/*center*/, QPoint p1, QPoint p2, QPoint p3,
                                    float &start, float &span)
{
    float x, ang1, ang2, ang3;
    x = (p.x()==0)? p.x()+1.0e-7: p.x();    // avoid zero division error
    ang1 = atan((p.y()-p1.y())/(p1.x()-x))*180/3.14159265;
    ang1 = (x>p1.x()) ? ang1+180 : ang1;
    ang1 = ang1<0 ? ang1+360: ang1;
    ang2 = atan((p.y()-p2.y())/(p2.x()-x))*180/3.14159265;
    ang2 = (x>p2.x()) ? ang2+180 : ang2;
    ang2 = ang2<0 ? ang2+360: ang2;
    ang3 = atan((p.y()-p3.y())/(p3.x()-x))*180/3.14159265;
    ang3 = (x>p3.x()) ? ang3+180 : ang3;
    ang3 = ang3<0 ? ang3+360: ang3;
    if (ang1 > ang2) {
        float tmp = ang1;
        ang1 = ang2;
        ang2 = tmp;
    }
    if (ang1<ang3 and ang3<ang2) {
        start = ang1;
        span = ang2-ang1;
    }
    else {
        start = ang2;
        span = 360 - (ang2-ang1);
    }
}


// *********************************************************************
//                              Resize Manager
// _____________________________________________________________________

// ResizeDialog object to get required image size
ResizeDialog:: ResizeDialog(QWidget *parent, int img_width, int img_height) : QDialog(parent)
{
    setupUi(this);
    frame->hide();
    resize(350, 100);
    QIntValidator validator(this);
    widthEdit->setValidator(&validator);
    heightEdit->setValidator(&validator);
    ratioEdit->setValidator(&validator);
    multRIS->setValidator(&validator);
    orig_width = img_width;
    orig_height = img_height;
    widthEdit->setText( QString::number(img_width));
    heightEdit->setText( QString::number(img_height));
    multRIS->setText( QString::number(1.0));
    spinWidth->setValue(img_width*2.54/300);
    spinHeight->setValue(img_height*2.54/300);
    QObject::connect(ratioEdit, SIGNAL(editingFinished()), this, SLOT(ratioTextChanged()));
    QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggleAdvanced(bool)));
    QObject::connect(spinWidth, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinHeight, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinDPI, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));
    widthEdit->setFocus();
}

void
ResizeDialog:: toggleAdvanced(bool checked)
{
    if (checked)
        frame->show();
    else {
        frame->hide();
        waitFor(50);
        resize(350, 100);
    }
}

void
ResizeDialog:: onValueChange(int)
{
    int DPI = spinDPI->value();
    widthEdit->setText( QString::number(round(DPI * spinWidth->value()/2.54)));
    heightEdit->setText( QString::number(round(DPI * spinHeight->value()/2.54)));
}

void
ResizeDialog:: ratioTextChanged()
{
    float ratio = ratioEdit->text().toDouble();
    float dr = (ratio > orig_ratio) ? (ratio - orig_ratio) : (orig_ratio - ratio);
    if (dr > 0.0)
    {
        if (ratio > 0.0)
        {
            widthEdit->setText( QString::number(round(ratio * orig_width)));
            heightEdit->setText( QString::number(round(ratio * orig_height)));
        } else {
            widthEdit->setText( QString::number(round(orig_width)));
            heightEdit->setText( QString::number(round(orig_height)));
        }
    }
    orig_ratio = ratio;
}
