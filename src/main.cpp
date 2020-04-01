/*
...........................................................................
|   Copyright (C) 2016-2020 Arindam Chaudhuri <ksharindam@gmail.com>       |
|                                                                          |
|   This program is free software: you can redistribute it and/or modify   |
|   it under the terms of the GNU General Public License as published by   |
|   the Free Software Foundation, either version 3 of the License, or      |
|   (at your option) any later version.                                    |
|                                                                          |
|   This program is distributed in the hope that it will be useful,        |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of         |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          |
|   GNU General Public License for more details.                           |
|                                                                          |
|   You should have received a copy of the GNU General Public License      |
|   along with this program.  If not, see <http://www.gnu.org/licenses/>.  |
...........................................................................
*/
#include "main.h"
#include "common.h"
#include "dialogs.h"
#include "transform.h"
#include "photogrid.h"
#include "inpaint.h"
#include "filters.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QPainter>
#include <QDesktopWidget>
#include <QSettings>
#include <QMenu>
#include <QRegExp>
#include <cmath>

Window:: Window()
{
    setupUi(this);
    QMenu *saveMenu = new QMenu(saveBtn);
    saveMenu->addAction("Overwrite", this, SLOT(overwrite()));
    saveMenu->addAction("Save a Copy", this, SLOT(saveACopy()));
    saveMenu->addAction("Save As...", this, SLOT(saveAs()));
    saveBtn->setMenu(saveMenu);
    QMenu *transformMenu = new QMenu(transformBtn);
    transformMenu->addAction("Mirror", this, SLOT(mirror()));
    transformMenu->addAction("Straighten", this, SLOT(perspectiveTransform()));
    transformBtn->setMenu(transformMenu);
    QMenu *decorateMenu = new QMenu(decorateBtn);
    decorateMenu->addAction("Photo Grid", this, SLOT(createPhotoGrid()));
    decorateMenu->addAction("Photo Collage", this, SLOT(createPhotoCollage()));
    decorateMenu->addAction("Add Border", this, SLOT(addBorder()));
    decorateBtn->setMenu(decorateMenu);
    QMenu *fxMenu = new QMenu(effectsBtn);
    fxMenu->addAction("Scanned Page", this, SLOT(adaptiveThresh()));
    QMenu *colorMenu = fxMenu->addMenu("Color");
        colorMenu->addAction("GrayScale", this, SLOT(toGrayScale()));
        colorMenu->addAction("Threshold", this, SLOT(toBlacknWhite()));
        colorMenu->addAction("White Balance", this, SLOT(whiteBalance()));
        colorMenu->addAction("Enhance Colors", this, SLOT(enhanceColors()));
    QMenu *brightnessMenu = fxMenu->addMenu("Brightness");
        brightnessMenu->addAction("Enhance Contrast", this, SLOT(sigmoidContrast()));
        brightnessMenu->addAction("Enhance Low Light", this, SLOT(enhanceLight()));
    QMenu *noiseMenu = fxMenu->addMenu("Noise Removal");
        noiseMenu->addAction("Despeckle", this, SLOT(reduceSpeckleNoise()));
        noiseMenu->addAction("Remove Dust", this, SLOT(removeDust()));
    fxMenu->addAction("Magic Eraser", this, SLOT(magicEraser()));
    fxMenu->addAction("Sharpen", this, SLOT(sharpenImage()));
    fxMenu->addAction("Smooth/Blur...", this, SLOT(blur()));
    fxMenu->addAction("Pencil Sketch", this, SLOT(pencilSketchFilter()));
    effectsBtn->setMenu(fxMenu);
    QAction *delAction = new QAction(this);
    delAction->setShortcut(QString("Delete"));
    connect(delAction, SIGNAL(triggered()), this, SLOT(deleteFile()));
    this->addAction(delAction);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new Canvas(this, scrollArea);
    layout->addWidget(canvas);
    timer = new QTimer(this);
    connectSignals();
    // Initialize Variables
    QDesktopWidget *desktop = QApplication::desktop();
    screen_width = desktop->availableGeometry().width();
    screen_height = desktop->availableGeometry().height();
    filename = QString("photoquick.jpg");
    QSettings settings;
    offset_x = settings.value("OffsetX", 4).toInt();
    offset_y = settings.value("OffsetY", 26).toInt();
    btnboxwidth = settings.value("BtnBoxWidth", 60).toInt();
}

void
Window:: connectSignals()
{
    // For the buttons of the left side
    connect(openBtn, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(resizeBtn, SIGNAL(clicked()), this, SLOT(resizeImage()));
    connect(cropBtn, SIGNAL(clicked()), this, SLOT(cropImage()));
    connect(quitBtn, SIGNAL(clicked()), this, SLOT(close()));
    // For the buttons of the right side
    connect(prevBtn, SIGNAL(clicked()), this, SLOT(openPrevImage()));
    connect(nextBtn, SIGNAL(clicked()), this, SLOT(openNextImage()));
    connect(zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomInImage()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOutImage()));
    connect(origSizeBtn, SIGNAL(clicked()), this, SLOT(origSizeImage()));
    connect(rotateLeftBtn, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightBtn, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(playPauseBtn, SIGNAL(clicked()), this, SLOT(playPause()));
    connect(timer, SIGNAL(timeout()), this, SLOT(openNextImage()));
    // Connect other signals
    connect(canvas, SIGNAL(imageUpdated()), this, SLOT(updateStatus()));
}

void
Window:: openFile()
{
    QString filefilter = "Image files (*.jpg *.png *.jpeg *.svg *.gif *.tiff *.ppm *.bmp);;JPEG Images (*.jpg *.jpeg);;"
                         "PNG Images (*.png);;SVG Images (*.svg);;All Files (*)";
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image", filename, filefilter);
    if (filepath.isEmpty()) return;
    openImage(filepath);
}

void
Window:: openImage(QString filepath)
{
    QImageReader img_reader(filepath);
    int frame_count = img_reader.imageCount();
    if (frame_count==1) {  // For still images
        QImage img = loadImage(filepath);  // Returns an autorotated image
        if (img.isNull()) return;
        canvas->scale = fitToScreenScale(img);
        canvas->setImage(img);
        adjustWindowSize();
        disableButtons(false);
        if (!timer->isActive())
            playPauseBtn->setIcon(QIcon(":/images/play.png"));
    }
    else if (frame_count>1) { // For animations
        QMovie *anim = new QMovie(filepath, QByteArray(), this);
        if (anim->isValid()) {
          canvas->setAnimation(anim);
          adjustWindowSize(true);
          statusbar->showMessage(QString("Resolution : %1x%2").arg(canvas->width()).arg(canvas->height()));
          playPauseBtn->setIcon(QIcon(":/images/pause.png"));
          disableButtons(true);
        }
    }
    else {
        statusbar->showMessage(QString("Invalid frame count : %1").arg(frame_count));
        return;
    }
    this->filename = filepath;
    QFileInfo fi(filename);
    QString dir = fi.dir().path();
    QDir::setCurrent(dir);
    setWindowTitle(fi.fileName());
}

void
Window:: saveImage(QString filename)
{
    QImage img = canvas->image;
    if (canvas->animation)
        img = canvas->movie()->currentImage();
    if (img.isNull()) return;
    int quality = -1;
    if (filename.endsWith(".jpg", Qt::CaseInsensitive)) {
        if (img.hasAlphaChannel()) { // converts background to white
            img = QImage(img.width(), img.height(), QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter painter(&img);
            painter.drawImage(0,0, canvas->image);
            painter.end();
        }
        QualityDialog *dlg = new QualityDialog(this, img);
        if (dlg->exec()==QDialog::Accepted){
            quality = dlg->qualitySpin->value();
        }
        else return;
    }
    img.save(filename, NULL, quality);
    setWindowTitle(QFileInfo(filename).fileName());
    this->filename = filename;
    Notifier *notifier = new Notifier(this);
    notifier->notify("Image Saved !", "Successfully Saved Image");
}

void
Window:: overwrite()
{
    saveImage(this->filename);
}

void
Window:: saveAs()
{
    QString filefilter = QString("Image files (*.jpg *.png *.jpeg *.ppm *.bmp *.tiff);;"
                                 "JPEG Image (*.jpg);;PNG Image (*.png);;Tagged Image (*.tiff);;"
                                 "Portable Pixmap (*.ppm);;X11 Pixmap (*.xpm);;Windows Bitmap (*.bmp)");
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", filename, filefilter);
    if (filepath.isEmpty()) return;
    saveImage(filepath);
}

void
Window:: saveACopy()    // generate a new filename and save
{
    QFileInfo fi(filename);
    QString dir = fi.dir().path();
    QString basename = fi.completeBaseName();
    QString ext = fi.suffix().isEmpty()? ".jpg": "."+fi.suffix();
    // extract the num just before the file extension
    QRegExp rx("(.*\\D)*(\\d*)");
    int pos = rx.indexIn(basename);
    if (pos==-1) return;
    int num = rx.cap(2).isEmpty()? 1: rx.cap(2).toInt();
    QString path;
    do {
        path = dir+ '/' + rx.cap(1) + QString::number(num++) + ext;
    }
    while (QFileInfo(path).exists());
    saveImage(path);
}

void
Window:: deleteFile()
{
    QString nextfile = getNextFilename(filename); // must be called before deleting
    QFile fi(filename);
    if (not fi.exists()) return;
    if (QMessageBox::warning(this, "Delete File?", "Are you sure to permanently delete this image?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        return;
    if (!fi.remove()) {
        QMessageBox::warning(this, "Delete Failed !", "Could not delete the image");
        return;
    }
    if (!nextfile.isNull())
        openImage(nextfile);
}

void
Window:: resizeImage()
{
    ResizeDialog *dialog = new ResizeDialog(this, canvas->image.width(), canvas->image.height());
    if (dialog->exec() == 1) {
        QImage img;
        Qt::TransformationMode tfmMode = dialog->smoothScaling->isChecked() ?
                        Qt::SmoothTransformation : Qt::FastTransformation;
        QString img_width = dialog->widthEdit->text();
        QString img_height = dialog->heightEdit->text();
        if ( !img_width.isEmpty() and !img_height.isEmpty() )
            img = canvas->image.scaled(img_width.toInt(), img_height.toInt(), Qt::IgnoreAspectRatio, tfmMode);
        else if (not img_width.isEmpty())
            img = canvas->image.scaledToWidth(img_width.toInt(), tfmMode);
        else if (not img_height.isEmpty())
            img = canvas->image.scaledToHeight(img_height.toInt(), tfmMode);
        else
            return;
        canvas->setImage(img);
    }
}

void
Window:: cropImage()
{
    hideButtons();
    Crop *crop = new Crop(canvas, statusbar);
    connect(canvas, SIGNAL(mousePressed(QPoint)), crop, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), crop, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), crop, SLOT(onMouseMove(QPoint)));
    connect(crop, SIGNAL(finished()), this, SLOT(hideButtons()));
}

void
Window:: addBorder()
{
    bool ok;
    int width = QInputDialog::getInt(this, "Add Border", "Enter Border Width :", 2, 1, 100, 1, &ok);
    if (ok) {
        QPainter painter(&(canvas->image));
        QPen pen(Qt::black);
        pen.setWidth(width);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.drawRect(width/2, width/2, canvas->image.width()-width, canvas->image.height()-width);
        canvas->showScaled();
    }
}

void
Window:: createPhotoGrid()
{
    GridDialog *dialog = new GridDialog(canvas->image, this);
    int dialog_h = screen_height - offset_y - offset_x;
    dialog->resize(1020, dialog_h);
    if (dialog->exec() == 1) {
        canvas->scale = fitToScreenScale(dialog->gridPaper->photo_grid);
        canvas->setImage(dialog->gridPaper->photo_grid);
        adjustWindowSize();
    }
}

void
Window:: createPhotoCollage()
{
    CollageDialog *dialog = new CollageDialog(this);
    int dialog_h = screen_height - offset_y - offset_x;
    dialog->resize(1050, dialog_h);
    CollageItem *item = new CollageItem(canvas->image);
    dialog->collagePaper->addItem(item);
    if (dialog->exec() == 1) {
        canvas->scale = fitToScreenScale(dialog->collage);
        canvas->setImage(dialog->collage);
        adjustWindowSize();
    }
}

void
Window:: magicEraser()
{
    InpaintDialog *dialog = new InpaintDialog(canvas->image, this);
    int dialog_h = screen_height - offset_y - offset_x;
    dialog->resize(1020, dialog_h);
    if (dialog->exec()==QDialog::Accepted) {
        canvas->setImage(dialog->canvas->image);
    }
}

void
Window:: toGrayScale()
{
    grayScale(canvas->image);
    canvas->showScaled();
}

void
Window:: toBlacknWhite()
{
    int thresh = calcOtsuThresh(canvas->image);
    threshold(canvas->image, thresh);
    canvas->showScaled();
}

void
Window:: adaptiveThresh()
{
    adaptiveThreshold(canvas->image);
    canvas->showScaled();
}

void
Window:: blur()
{
    bool ok;
    int radius = QInputDialog::getInt(this, "Blur Radius", "Enter Blur Radius :",
                                        1/*val*/, 1/*min*/, 30/*max*/, 1/*step*/, &ok);
    if (not ok) return;
    gaussianBlur(canvas->image, radius);
    //boxFilter(canvas->image, radius);
    canvas->showScaled();
}

void
Window:: sharpenImage()
{
    sharpen(canvas->image);
    canvas->showScaled();
}

void
Window:: reduceSpeckleNoise()
{
    despeckle(canvas->image);
    canvas->showScaled();
}

void
Window:: removeDust()
{
    medianFilter(canvas->image, 1);
    canvas->showScaled();
}

void
Window:: sigmoidContrast()
{
    sigmoidalContrast(canvas->image, 0.3);
    canvas->showScaled();
}
void
Window:: enhanceLight()
{
    stretchContrast(canvas->image);
    canvas->showScaled();
}

void
Window:: whiteBalance()
{
    autoWhiteBalance(canvas->image);
    canvas->showScaled();
}

void
Window:: enhanceColors()
{
    enhanceColor(canvas->image);
    canvas->showScaled();
}

void
Window:: pencilSketchFilter()
{
    pencilSketch(canvas->image);
    canvas->showScaled();
}

void
Window:: openPrevImage()
{
    QFileInfo fi(filename);
    if (not fi.exists()) return;
    QString filename = fi.fileName();
    QString basedir = fi.absolutePath();         // This does not include filename
    QString file_filter("*.jpg *.jpeg *.png *.gif *.svg *.bmp *.tiff");
    QStringList image_list = fi.dir().entryList(file_filter.split(" "));

    int index = image_list.indexOf(filename);
    if (index==0) index = image_list.count();
    QString prevfile = image_list[index-1];
    openImage(basedir + "/" + prevfile);
}

void
Window:: openNextImage()
{
    QString nextfile = getNextFilename(filename);
    if (!nextfile.isNull())
        openImage(nextfile);
}

void
Window:: zoomInImage()
{
    QScrollBar *vertical = scrollArea->verticalScrollBar();
    QScrollBar *horizontal = scrollArea->horizontalScrollBar();
    float relPosV = vertical->value()/(float)vertical->maximum();
    float relPosH = horizontal->value()/(float)horizontal->maximum();
    bool wasVisibleV = vertical->isVisible();
    bool wasVisibleH = horizontal->isVisible();
    if (not wasVisibleV) relPosV=0.5;
    if (not wasVisibleH) relPosH=0.5;
    // Integer scale to view small icons
    if (canvas->image.width() < 200 and canvas->image.height() < 200 and canvas->scale>=1)
        canvas->scale += 1;
    else
        canvas->scale *= (6.0/5);
    canvas->showScaled();
    if ((canvas->pixmap()->width()>scrollArea->width() or
            canvas->pixmap()->height()>scrollArea->height()) && not this->isMaximized())
        this->showMaximized();
    waitFor(30);
    vertical->setValue(vertical->maximum()*relPosV);
    horizontal->setValue(horizontal->maximum()*relPosH);
}

void
Window:: zoomOutImage()
{
    QScrollBar *vertical = scrollArea->verticalScrollBar();
    QScrollBar *horizontal = scrollArea->horizontalScrollBar();
    float relPosV = vertical->value()/(float)vertical->maximum();
    float relPosH = horizontal->value()/(float)horizontal->maximum();
    if (canvas->image.width() < 200 and canvas->image.height() < 200 and canvas->scale>1)
        canvas->scale -= 1;
    else
        canvas->scale *= (5.0/6);
    canvas->showScaled();
    waitFor(30);
    vertical->setValue(vertical->maximum()*relPosV);
    horizontal->setValue(horizontal->maximum()*relPosH);
}

// switches size between 1x and fit to window
void
Window:: origSizeImage()
{
    if (canvas->scale == 1.0) {
        canvas->scale = fitToWindowScale(canvas->image);
        canvas->showScaled();
        origSizeBtn->setIcon(QIcon(":/images/originalsize.png"));
        return;
    }
    canvas->scale = 1.0;
    canvas->showScaled();
    origSizeBtn->setIcon(QIcon(":/images/fit-to-screen.png"));
    if ((canvas->pixmap()->width()>scrollArea->width() or
            canvas->pixmap()->height()>scrollArea->height()) && not this->isMaximized())
        this->showMaximized();
}

void
Window:: rotateLeft()
{
    canvas->rotate(270);
}

void
Window:: rotateRight()
{
    canvas->rotate(90);
}

void
Window:: mirror()
{
    canvas->rotate(180, Qt::YAxis);
}

void
Window:: perspectiveTransform()
{
    hideButtons();
    PerspectiveTransform *transform = new PerspectiveTransform(canvas, statusbar);
    connect(canvas, SIGNAL(mousePressed(QPoint)), transform, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), transform, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), transform, SLOT(onMouseMove(QPoint)));
    connect(transform, SIGNAL(finished()), this, SLOT(hideButtons()));
}

void
Window:: playPause()
{
    if (timer->isActive()) {       // Stop slideshow
        timer->stop();
        playPauseBtn->setIcon(QIcon(":/images/play.png"));
        return;
    }
    if (canvas->animation) {
        if (canvas->movie()->state()==QMovie::Running) {
            canvas->movie()->setPaused(true);
            playPauseBtn->setIcon(QIcon(":/images/play.png"));
        }
        else {
            canvas->movie()->setPaused(false);
            playPauseBtn->setIcon(QIcon(":/images/pause.png"));
        }
    }
    else {// Start slideshow
        timer->start(3000);
        playPauseBtn->setIcon(QIcon(":/images/pause.png"));
    }
}

float
Window:: fitToWindowScale(QImage img)
{
    int img_w = img.width();
    int img_h =  img.height();
    int max_w = scrollArea->width();
    int max_h = scrollArea->height()-15;    // 15 is to compensate increased statusbar
    int out_w, out_h;
    fitToSize(img_w, img_h, max_w, max_h, out_w, out_h);
    float scale = img_w>img_h ? out_w/(float)img_w : out_h/(float)img_h;
    return scale;
}

float
Window:: fitToScreenScale(QImage img)
{
    float scale;
    int img_width = img.width();
    int img_height = img.height();
    int max_width = screen_width - (2*btnboxwidth + 2*offset_x);
    int max_height = screen_height - (offset_y + offset_x + 4+33); // 33 for statusbar with buttons
    if ((img_width > max_width) || (img_height > max_height)) {
        if (float(max_width)/max_height > float(img_width)/img_height) {
            scale = float(max_height)/img_height;
        }
        else
            scale = float(max_width)/img_width;
    }
    else
        scale = 1.0;
    return scale;
}

void
Window:: adjustWindowSize(bool animation)
{
    if (isMaximized()) return;
    if (animation) {
        waitFor(30);        // Wait little to let Label resize and get correct width height
        resize(canvas->width() + 2*btnboxwidth + 4,
               canvas->height() + 4+32);
    }
    else {
        resize(canvas->pixmap()->width() + 2*btnboxwidth + 4,
               canvas->pixmap()->height() + 4+33);
    }
    move((screen_width - (width() + 2*offset_x) )/2,
              (screen_height - (height() + offset_x + offset_y))/2 );
}

void
Window:: updateStatus()
{
    int width = canvas->image.width();
    int height = canvas->image.height();
    QString text = "Resolution : %1x%2 , Scale : %3x";
    statusbar->showMessage(text.arg(width).arg(height).arg(roundOff(canvas->scale, 2)));
}

// hide if not hidden, unhide if hidden
void
Window:: hideButtons()
{
    frame->setHidden(frame->isVisible());
    frame_2->setHidden(frame_2->isVisible());
}

void
Window:: disableButtons(bool disable)
{
    resizeBtn->setDisabled(disable);
    cropBtn->setDisabled(disable);
    transformBtn->setDisabled(disable);
    decorateBtn->setDisabled(disable);
    effectsBtn->setDisabled(disable);
    zoomInBtn->setDisabled(disable);
    zoomOutBtn->setDisabled(disable);
    origSizeBtn->setDisabled(disable);
    rotateLeftBtn->setDisabled(disable);
    rotateRightBtn->setDisabled(disable);
}

void
Window:: closeEvent(QCloseEvent *ev)
{
    QSettings settings;
    settings.setValue("OffsetX", geometry().x()-x());
    settings.setValue("OffsetY", geometry().y()-y());
    settings.setValue("BtnBoxWidth", frame->width());
    QMainWindow::closeEvent(ev);
}

// other functions
QString getNextFilename(QString current)
{
    QFileInfo fi(current);
    if (not fi.exists())
        return QString();
    QString filename = fi.fileName();
    QString basedir = fi.absolutePath();    // This does not include filename
    QString file_filter("*.jpg *.jpeg *.png *.gif *.svg *.bmp *.tiff");
    QStringList image_list = fi.dir().entryList(file_filter.split(" "));
    if (image_list.count()<2)
        return QString();
    int index = image_list.indexOf(filename);
    if (index >= image_list.count()-1) index = -1;
    return basedir + '/' + image_list[index+1];
}

// ************* main function ****************

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("photoquick");
    app.setApplicationName("photoquick");
#ifdef WIN32
	app.addLibraryPath(app.applicationDirPath());
#endif
    Window *win = new Window();
    win->show();
    if (argc > 1) {
        QString path = QString::fromUtf8(argv[1]);
        QFileInfo fileinfo(path);
        if (fileinfo.exists())
            win->openImage(path);
    }
    else {
        QImage img = QImage(":/images/photoquick.jpg");
        win->canvas->setImage(img);
        win->adjustWindowSize();
    }
    return app.exec();
}
