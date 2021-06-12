/*
...........................................................................
|   Copyright (C) 2017-2021 Arindam Chaudhuri <ksharindam@gmail.com>       |
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

Window:: Window()
{
    QMenu *fileMenu, *transformMenu, *decorateMenu, *filtersMenu,
          *toolsMenu, *infoMenu;
    QMenu *geometryMenu, *brightnessMenu, *colorMenu, *effectsMenu,
          *noiseMenu, *blurMenu, *thresholdMenu;
    QAction *delAction, *reloadAction;
    QHBoxLayout *layout;
    QDesktopWidget *desktop;
    QSettings settings;

    setupUi(this);
    fileMenu = new QMenu(fileBtn);
    fileMenu->addAction("Overwrite", this, SLOT(overwrite()));
    fileMenu->addAction("Save a Copy", this, SLOT(saveACopy()));
    fileMenu->addAction("Save As...", this, SLOT(saveAs()));
    fileMenu->addAction("Save by File Size", this, SLOT(autoResizeAndSave()));
    fileMenu->addSeparator();
    fileMenu->addAction("Export to PDF", this, SLOT(exportToPdf()));
    fileMenu->addSeparator();
    fileMenu->addAction("Open Image", this, SLOT(openFile()));
    fileBtn->setMenu(fileMenu);
    transformMenu = new QMenu(transformBtn);
    transformMenu->addAction("Mirror Image", this, SLOT(mirror()));
    geometryMenu = transformMenu->addMenu("Geometry");
        geometryMenu->addAction("Un-tilt Image", this, SLOT(perspectiveTransform()));
        geometryMenu->addAction("DeWarping", this, SLOT(deWarping()));
        geometryMenu->addAction("DeOblique", this, SLOT(deOblique()));
        geometryMenu->addAction("Lens Distortion", this, SLOT(lensDistort()));
    transformBtn->setMenu(transformMenu);
    decorateMenu = new QMenu(decorateBtn);
    decorateMenu->addAction("Photo Grid", this, SLOT(createPhotoGrid()));
    decorateMenu->addAction("Photo Collage", this, SLOT(createPhotoCollage()));
    decorateMenu->addAction("Add Border", this, SLOT(addBorder()));
    decorateMenu->addAction("Expand Border", this, SLOT(expandImageBorder()));
    decorateBtn->setMenu(decorateMenu);
    // Filters menu
    filtersMenu = new QMenu(filtersBtn);
    brightnessMenu = filtersMenu->addMenu("Brightness");
        brightnessMenu->addAction("Enhance Contrast", this, SLOT(sigmoidContrast()));
        brightnessMenu->addAction("Enhance Low Light", this, SLOT(enhanceLight()));
        brightnessMenu->addAction("Gamma Correction", this, SLOT(gammaCorrection()));
    colorMenu = filtersMenu->addMenu("Color");
        colorMenu->addAction("GrayScale", this, SLOT(toGrayScale()));
        colorMenu->addAction("Color Balance", this, SLOT(grayWorldFilter()));
        colorMenu->addAction("White Balance", this, SLOT(whiteBalance()));
        colorMenu->addAction("Enhance Colors", this, SLOT(enhanceColors()));
    effectsMenu = filtersMenu->addMenu("Effects");
        effectsMenu->addAction("Vignette", this, SLOT(vignetteFilter()));
        effectsMenu->addAction("PencilSketch", this, SLOT(pencilSketchFilter()));
    noiseMenu = filtersMenu->addMenu("Noise Removal");
        noiseMenu->addAction("Despeckle", this, SLOT(reduceSpeckleNoise()));
        noiseMenu->addAction("Remove Dust", this, SLOT(removeDust()));
    blurMenu = filtersMenu->addMenu("Smooth");
        blurMenu->addAction("Box...", this, SLOT(box()));
        blurMenu->addAction("Easy deBlur...", this, SLOT(deblur()));
        blurMenu->addAction("Sharpen", this, SLOT(sharpenImage()));
        blurMenu->addAction("Smooth/Blur...", this, SLOT(blur()));
    thresholdMenu = filtersMenu->addMenu("Threshold");
        thresholdMenu->addAction("Threshold", this, SLOT(applyThreshold()));
        thresholdMenu->addAction("Scanned Page", this, SLOT(adaptiveThresh()));
    filtersBtn->setMenu(filtersMenu);
    // Tools menu
    toolsMenu = new QMenu(toolsBtn);
    toolsMenu->addAction("Scissor && Eraser", this, SLOT(iScissor()));
    toolsMenu->addAction("Magic Eraser", this, SLOT(magicEraser()));
    toolsBtn->setMenu(toolsMenu);
    // More info menu
    infoMenu = new QMenu(infoBtn);
    infoMenu->addAction("Image Info", this, SLOT(imageInfo()));
    infoBtn->setMenu(infoMenu);

    delAction = new QAction(this);
    delAction->setShortcut(QString("Delete"));
    connect(delAction, SIGNAL(triggered()), this, SLOT(deleteFile()));
    this->addAction(delAction);
    reloadAction = new QAction(this);
    reloadAction->setShortcut(QString("R"));
    connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadImage()));
    this->addAction(reloadAction);

    layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new Canvas(scrollArea, &data);
    layout->addWidget(canvas);
    timer = new QTimer(this);
    connectSignals();
    // Initialize Variables
    data.filename = QString("photoquick.jpg");
    data.window = this;

    desktop = QApplication::desktop();
    screen_width = desktop->availableGeometry().width();
    screen_height = desktop->availableGeometry().height();
    offset_x = settings.value("OffsetX", 4).toInt();
    offset_y = settings.value("OffsetY", 26).toInt();
    btnboxwidth = settings.value("BtnBoxWidth", 60).toInt();
    data.max_window_w = screen_width - offset_x - offset_x;
    data.max_window_h = screen_height - offset_y - offset_x;


    menu_dict["File"] = fileMenu;
    menu_dict["Transform"] = transformMenu;
    menu_dict["Transform/Geometry"] = geometryMenu;
    menu_dict["Decorate"] = decorateMenu;
    menu_dict["Tools"] = toolsMenu;
    menu_dict["Info"] = infoMenu;
    menu_dict["Filters"] = filtersMenu;
    menu_dict["Filters/Brightness"] = brightnessMenu;
    menu_dict["Filters/Color"] = colorMenu;
    menu_dict["Filters/Effects"] = effectsMenu;
    menu_dict["Filters/Noise Removal"] = noiseMenu;
    menu_dict["Filters/Smooth"] = blurMenu;
    menu_dict["Filters/Threshold"] = thresholdMenu;
}

void
Window:: connectSignals()
{
    // For the buttons of the left side
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

QAction* addPluginMenuItem(QString menu_path, QMap<QString, QMenu *> &menu_dict)
{
    QStringList list;
    QString path;
    QMenu *menu;
    if (menu_path.isNull())
        return NULL;
    list = menu_path.split("/");
    if (list.count()<2)
        return NULL;
    path = list[0];
    if (not menu_dict.contains(path))
        return NULL;
    menu = menu_dict[path]; // button menu
    for (int i=1; i<list.count()-1; i++) { // create intermediate menus
        path += "/" + list[i];
        if (not menu_dict.contains(path)) {
            menu = menu->addMenu( list[i].replace("%", "/") );
            menu_dict[path] = menu;
        }
        menu = menu_dict[path];
    }
    return menu->addAction(list.last().replace("%", "/"));//use % if / is needed in menu name
}

void
Window:: loadPlugins()
{
    QString app_dir_path = qApp->applicationDirPath();
    QStringList dirs = { app_dir_path };
    if (app_dir_path.endsWith("/src"))
        dirs << app_dir_path + "/..";
#ifdef _WIN32
    QStringList filter = {"*.dll"};
#else
    QStringList filter = {"*.so"};
    // load system libraries only if the program is installed
    if (app_dir_path.endsWith("/bin"))
        dirs += {app_dir_path + "/../share/photoquick",
                QDir::homePath() + "/.local/share/photoquick"};
#endif
    for (QString dir : dirs) {
        QDir pluginsDir(dir + "/plugins");
        if (not pluginsDir.exists()) continue;
        //qDebug()<< dir + "/plugins";
        for (QString fileName : pluginsDir.entryList(filter, QDir::Files, QDir::Name)) {
            QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
            //qDebug() << "Loading :" << fileName;
            QObject *pluginObj = pluginLoader.instance();
            if (not pluginObj) {
                qDebug()<< fileName << ": create instance failed";
                continue;
            }
            Plugin *plugin = qobject_cast<Plugin *>(pluginObj);
            if (not plugin) {
                qDebug()<< fileName << ": casting failed";
                continue;
            }
            plugin->initialize(&data);
            connect(pluginObj, SIGNAL(imageChanged()), canvas, SLOT(showScaled()));
            connect(pluginObj, SIGNAL(optimumSizeRequested()), this, SLOT(resizeToOptimum()));
            connect(pluginObj, SIGNAL(sendNotification(QString,QString)), this, SLOT(showNotification(QString,QString)));
            // add menu items and window shortcuts
            QAction *action = addPluginMenuItem(plugin->menuItem(), menu_dict);
            if (action) connect(action, SIGNAL(triggered()), pluginObj, SLOT(onMenuClick()));
            for (QString menu_path : plugin->menuItems()) {
                action = addPluginMenuItem(menu_path, menu_dict);
                if (action) plugin->handleAction(action, ACTION_MENU);
            }
            for (QString shortcut : plugin->getShortcuts()) {
                action = new QAction(this);
                action->setShortcut(shortcut);
                this->addAction(action);
                plugin->handleAction(action, ACTION_SHORTCUT);
            }
        }
    }
    menu_dict["Info"]->addAction("About PhotoQuick", this, SLOT(showAbout()));
}

void
Window:: openFile()
{
    QString filefilter, filepath;
    filefilter = "Image files (*.jpg *.png *.jpeg *.svg *.gif *.tif *.tiff *.ppm *.bmp);;"
                 "JPEG Images (*.jpg *.jpeg);;PNG Images (*.png);;"
                 "TIFF Images (*.tif *.tiff);;SVG Images (*.svg);;"
                 "All Files (*)";
    filepath = QFileDialog::getOpenFileName(this, "Open Image", data.filename, filefilter);
    if (filepath.isEmpty()) return;
    openImage(filepath);
}

void
Window:: openImage(QString filepath)
{
    QFileInfo fileinfo(filepath);
    if (not fileinfo.exists()) return;

    QImageReader img_reader(filepath);
    int frame_count = img_reader.imageCount();
    if (frame_count==1) {  // For still images
        QImage img = loadImage(filepath);  // Returns an autorotated image
        if (img.isNull()) return;
        canvas->scale = fitToScreenScale(img);
        canvas->setImage(img);
        adjustWindowSize();
        disableButtons(VIEW_BUTTON, false);
        disableButtons(EDIT_BUTTON, false);
        if (!timer->isActive())
            playPauseBtn->setIcon(QIcon(":/icons/play.png"));
    }
    else if (frame_count>1) { // For animations
        QMovie *anim = new QMovie(filepath, QByteArray(), this);
        if (anim->isValid()) {
          canvas->setAnimation(anim);
          adjustWindowSize(true);
          statusbar->showMessage(QString("Resolution : %1x%2").arg(canvas->width()).arg(canvas->height()));
          playPauseBtn->setIcon(QIcon(":/icons/pause.png"));
          disableButtons(VIEW_BUTTON, true);
          disableButtons(EDIT_BUTTON, true);
        }
    }
    else { // unsupported file
        statusbar->showMessage("Unsupported File format");
        return;
    }
    data.filename = fileinfo.absoluteFilePath();
    QString dir = fileinfo.dir().path();
    QDir::setCurrent(dir);
    setWindowTitle(fileinfo.fileName());
}

void
Window:: saveImage(QString filename)
{
    QImage img = data.image;
    if (canvas->animation)
        img = canvas->movie()->currentImage();
    if (img.isNull()) return;
    int quality = -1;
    if (filename.endsWith(".jpg",  Qt::CaseInsensitive) ||
        filename.endsWith(".jpeg", Qt::CaseInsensitive))
    {
        if (img.hasAlphaChannel()) { // converts background to white
            img = QImage(img.width(), img.height(), QImage::Format_RGB32);
            img.fill(Qt::white);
            QPainter painter(&img);
            painter.drawImage(0,0, data.image);
            painter.end();
        }
        QualityDialog *dlg = new QualityDialog(this, img);
        if (dlg->exec()==QDialog::Accepted){
            quality = dlg->qualitySpin->value();
        }
        else return;
    }
    if (not img.save(filename, NULL, quality)) {
        showNotification("Failed !", "Could not save the image");
        return;
    }
    setWindowTitle(QFileInfo(filename).fileName());
    data.filename = filename;
    showNotification("Image Saved !", QFileInfo(filename).fileName());
}

void
Window:: overwrite()
{
    saveImage(data.filename);
}

void
Window:: saveAs()
{
    QStringList formats = {"jpg", "jp2", "png", "webp", "tiff", "ico", "bmp", "ppm"};
    QStringList names = {"JPEG Image", "JPEG 2000", "PNG Image", "WebP Image",
                "Tagged Image", "Windows Icon", "Windows Bitmap", "Portable Pixmap"};
    QStringList supported;
    for (QByteArray item : QImageWriter::supportedImageFormats())
        supported << QString(item);

    QString filters("All Files (*)");
    for (int i=0; i<formats.size(); i++) {
        if (supported.contains(formats[i]))
            filters += QString(";;%1 (*.%2)").arg(names[i]).arg(formats[i]);
    }
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", data.filename, filters);
    if (filepath.isEmpty()) return;
    saveImage(filepath);
}

void
Window:: saveACopy()    // generate a new filename and save
{
    QString path = getNewFileName(data.filename);
    saveImage(path);
}

void
Window:: autoResizeAndSave()
{
    float res1, size1, res2, size2, sizeOut, resOut, frac;
    bool ok;
    QImage scaled;
    if (data.image.isNull())
        return;
    res1 = data.image.width();
    size1 = getJpgFileSize(data.image) / 1024.0f;
    res2 = res1 * 0.5f;
    scaled = data.image.scaledToWidth(res2, Qt::SmoothTransformation);
    size2 = getJpgFileSize(scaled) / 1024.0f;
    sizeOut = QInputDialog::getInt(this, "File Size", "File Size below (kB) :", size1 * 0.5f, 1, size1, 1, &ok);
    if (not ok)
        return;
    resOut = log10(res1 / res2) / log10(size1 / size2) * log10(sizeOut / size1) + log10(res1);
    resOut = pow(10, resOut);
    scaled = data.image.scaledToWidth(resOut, Qt::SmoothTransformation);
    size2 = getJpgFileSize(scaled) / 1024.0f;
    for (frac = 0.95f; size2 > sizeOut; frac -= 0.05f)
    {
        scaled = data.image.scaledToWidth(resOut*frac, Qt::SmoothTransformation);
        size2 = getJpgFileSize(scaled) / 1024.0f;
    }
    // ensure that saved image is jpg
    QFileInfo fi(data.filename);
    QString dir = fi.dir().path();
    QString basename = fi.completeBaseName();
    QString path = dir + "/" + basename + ".jpg";
    path = getNewFileName(path);

    scaled.save(path);
    showNotification("Image Saved !", QFileInfo(path).fileName());
}

bool isMonochrome(QImage img)
{
    int y, x, clr;
    QRgb *row;
    for (y = 0; y < img.height(); y++)
    {
        row = (QRgb*) img.constScanLine(y);
        for (x = 0; x < img.width(); x++)
        {
            clr = (row[x] & 0xffffff);
            if (not ((clr == 0) or (clr == 0xffffff))) return false;
        }
    }
    return true;
}

void
Window:: exportToPdf()
{
    if (data.image.isNull()) return;
    QImage image = data.image;
    // get or calculate paper size
    PaperSizeDialog *dlg = new PaperSizeDialog(this, image.width()>image.height());
    if (dlg->exec()==QDialog::Rejected) return;
    int dpi;
    float pdf_w, pdf_h;
    switch (dlg->combo->currentIndex()) {
    case 1:
        pdf_w = 595.0; // A4
        pdf_h = 841.0;
        break;
    case 2:
        pdf_w = 420.0; // A5
        pdf_h = 595.0;
        break;
    case 3:
        pdf_w = round(image.width()/100.0*72); // 100 dpi
        pdf_h = round(image.height()/100.0*72);
        break;
    case 4:
        pdf_w = round(image.width()/300.0*72);
        pdf_h = round(image.height()/300.0*72);
        break;
    case 5:
        bool ok;
        dpi = QInputDialog::getInt(this, "Enter Dpi", "Enter Scanned Image Dpi :", 150, 72, 1200, 50, &ok);
        if (not ok) return;
        pdf_w = round( image.width()*72.0/dpi );
        pdf_h = round( image.height()*72.0/dpi );
        break;
    default:
        pdf_w = 595.0;
        pdf_h = ceilf((pdf_w*image.height())/image.width());
    }
    if (dlg->combo->currentIndex()!=0 and dlg->landscape->isChecked() ) {
        int tmp = pdf_w;
        pdf_w = pdf_h;
        pdf_h = tmp;
    }
    // get image dimension and position
    int img_w = pdf_w;
    int img_h = round((pdf_w/image.width())*image.height());
    if (img_h > pdf_h) {
        img_h = pdf_h;
        img_w = round((pdf_h/image.height())*image.width());
    }
    int x = (pdf_w-img_w)/2;
    int y = (pdf_h-img_h)/2;

    // remove transperancy
    if (image.format()==QImage::Format_ARGB32) {
        QImage new_img(image.width(), image.height(), QImage::Format_RGB32);
        new_img.fill(Qt::white);
        QPainter painter(&new_img);
        painter.drawImage(0,0, image);
        painter.end();
        image = new_img;
    }
    if (isMonochrome(image))
        image = image.convertToFormat(QImage::Format_Mono);

    QFileInfo fi(data.filename);
    QString dir = fi.dir().path();
    QString basename = fi.completeBaseName();
    QString path = dir + "/" + basename + ".pdf";
    path = getNewFileName(path);
    std::string path_str = path.toUtf8().constData();

    PdfWriter writer;
    writer.begin(path_str);
    PdfObj cont;
    PdfDict resources;
    PdfDict imgs;
    PdfObj img;
    img.set("Type", "/XObject");
    img.set("Subtype", "/Image");
    img.set("Width", image.width());
    img.set("Height", image.height());
    // using PNG compression is best for Monochrome images
    if (image.format()==QImage::Format_Mono) {
        img.set("ColorSpace", "[/Indexed /DeviceRGB 1 <ffffff000000>]");
        img.set("BitsPerComponent", "1");
        img.set("Filter", "/FlateDecode");
        PdfDict decode_params;
        decode_params.set("Predictor", 15);
        decode_params.set("Columns", image.width());
        decode_params.set("BitsPerComponent", 1);
        decode_params.set("Colors", 1);
        img.set("DecodeParms", decode_params);
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        std::string data = getPngIdat(bArray.data(), bArray.size());
        writer.addObj(img, data);
        bArray.clear();
        buffer.close();
    }
    // Embed image as whole JPEG image
    else {
        img.set("ColorSpace", "/DeviceRGB");
        img.set("BitsPerComponent", "8");
        img.set("Filter", "/DCTDecode"); // jpg = DCTDecode
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "JPG");
        std::string data(bArray.data(), bArray.size());
        writer.addObj(img, data);
        bArray.clear();
        buffer.close();
    }

    std::string matrix = imgMatrix(x, y, img_w, img_h, 0);
    std::string cont_strm = format("q %s /img0 Do Q\n", matrix.c_str());
    imgs.set("img0", img.byref());

    writer.addObj(cont, cont_strm);
    resources.set("XObject", imgs);
    PdfObj page = writer.createPage(pdf_w, pdf_h);
    page.set("Contents", cont);
    page.set("Resources", resources);
    writer.addPage(page);
    writer.finish();

    showNotification("PDF Saved !", QFileInfo(path).fileName());
}

void
Window:: deleteFile()
{
    QString nextfile = getNextFileName(data.filename); // must be called before deleting
    QFile fi(data.filename);
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
Window:: reloadImage()
{
    openImage(data.filename);
}

void
Window:: imageInfo()
{
    QString str = QString("Width  : %1\n").arg(data.image.width());
    str += QString("Height : %1\n").arg(data.image.height());
    std::string exif_str = str.toStdString();

    FILE *f = fopen(data.filename.toUtf8().data(), "rb");
    if (f) {
        if (!read_Exif(f, exif_str))
            exif_str += "\nNo Exif Data !";
        fclose(f);
    }
    QMessageBox *dlg = new QMessageBox(this);
    dlg->setWindowTitle("Image Info");
    dlg->setText(exif_str.c_str());
    dlg->exec();
}

void
Window:: showAbout()
{
    QString text =
        "<h1>%1</h1>"
        "Version : %2<br>"
        "Qt Version : %3<br><br>"
        "A simple, handy and useful image viewer and editor with plugin support<br><br>"
        "Copyright &copy; %4 %5 &lt;%6&gt;";
    text = text.arg(PROG_NAME).arg(PROG_VERSION).arg(qVersion()).arg(COPYRIGHT_YEAR).arg(AUTHOR_NAME).arg(AUTHOR_EMAIL);
    QMessageBox::about(this, "About PhotoQuick", text);
}

QImage Window:: resizeImageBicub (QImage img, unsigned new_height, unsigned new_width)
{
    unsigned y, x;
    unsigned height = img.height();
    unsigned width = img.width();;
    float xFactor = (float)width / new_width;
    float yFactor = (float)height / new_height;
    float ox, oy;
    QImage dstImg(new_width, new_height, img.format());

    for (y = 0; y < new_height; y++ )
    {
        oy  = ((float)y + 0.5f) * yFactor - 0.5f;
        QRgb *row = (QRgb*)dstImg.constScanLine(y);
        for (x = 0; x < new_width; x++ )
        {
            ox  = ((float)x  + 0.5f) * xFactor - 0.5f;
            row[x] = InterpolateBiCubic (img, oy, ox);
        }
    }
    return dstImg;
}

void
Window:: resizeImage()
{
    ResizeDialog *dialog = new ResizeDialog(this, data.image.width(), data.image.height());
    if (dialog->exec() == 1)
    {
        QImage img, imgb;
        int nwidth, nheight, owidth, oheight;
        int nsteps, i , nwidthstep, nheightstep;
        int mode = dialog->comboMethod->currentIndex();
        Qt::TransformationMode tfmMode = mode ? Qt::SmoothTransformation : Qt::FastTransformation;
        owidth = data.image.width();
        oheight = data.image.height();
        QString img_width = dialog->widthEdit->text();
        QString img_height = dialog->heightEdit->text();
        nwidth = (img_width.isEmpty()) ? 0 : img_width.toInt();
        nheight = (img_height.isEmpty()) ? 0 : img_height.toInt();
        if (nwidth < 1 and nheight < 1) return;
        nheight = (nheight > 0) ? nheight : (int)((float)oheight * ((float)nwidth / owidth) + 0.5f);
        nwidth = (nwidth > 0) ? nwidth : (int)((float)owidth * ((float)nheight / oheight) + 0.5f);
        nheight = (nheight > 0) ? nheight : 1;
        nwidth = (nwidth > 0) ? nwidth : 1;
        nsteps = dialog->spinStep->value();
        if ((nsteps == 1) && !dialog->checkRIS->isChecked())
        {
            img = (mode > 1) ? resizeImageBicub (data.image, nheight, nwidth) : data.image.scaled(nwidth, nheight, Qt::IgnoreAspectRatio, tfmMode);
        }
        else
        {
            imgb = data.image.copy();
            for (i = 0; i < nsteps; i++)
            {
                nheightstep = oheight + (nheight - oheight) * (i + 1) / nsteps;
                nwidthstep = owidth + (nwidth - owidth) * (i + 1) / nsteps;
                img = (mode > 1) ? resizeImageBicub (imgb, nheightstep, nwidthstep) : imgb.scaled(nwidthstep, nheightstep, Qt::IgnoreAspectRatio, tfmMode);
                imgb = img.copy();
            }
            if (dialog->checkRIS->isChecked())
            {
                QString multRIS = dialog->multRIS->text();
                float mult = (multRIS.isEmpty()) ? 0 : multRIS.toFloat();
                img = (mode > 1) ? resizeImageBicub (imgb, oheight, owidth) : imgb.scaled(owidth, oheight, Qt::IgnoreAspectRatio, tfmMode);
                img = reFilter(img, data.image, mult);
                for (i = 0; i < nsteps; i++)
                {
                    nheightstep = oheight + (nheight - oheight) * (i + 1) / nsteps;
                    nwidthstep = owidth + (nwidth - owidth) * (i + 1) / nsteps;
                    imgb = (mode > 1) ? resizeImageBicub (img, nheightstep, nwidthstep) : img.scaled(nwidthstep, nheightstep, Qt::IgnoreAspectRatio, tfmMode);
                    img = imgb.copy();
                }
            }
        }
        canvas->setImage(img);
        canvas->showScaled();
    }
}

void
Window:: cropImage()
{
    frame->hide();
    frame_2->hide();
    Crop *crop = new Crop(canvas, statusbar);
    connect(canvas, SIGNAL(mousePressed(QPoint)), crop, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), crop, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), crop, SLOT(onMouseMove(QPoint)));
    connect(crop, SIGNAL(finished()), this, SLOT(onEditingFinished()));
}

void
Window:: addBorder()
{
    bool ok;
    int width = QInputDialog::getInt(this, "Add Border", "Enter Border Width :", 2, 1, 100, 1, &ok);
    if (ok) {
        QPainter painter(&(data.image));
        QPen pen(Qt::black);
        pen.setWidth(width);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.drawRect(width/2, width/2, data.image.width()-width, data.image.height()-width);
        canvas->showScaled();
    }
}

void
Window:: expandImageBorder()
{
    ExpandBorderDialog *dlg = new ExpandBorderDialog(this, data.image.width()/5);
    if (dlg->exec() != QDialog::Accepted)
        return;
    int w = dlg->widthSpin->value();
    int index = dlg->combo->currentIndex();
    if (index==0) {
        data.image = expandBorder(data.image, w);
        canvas->showScaled();
        return;
    }
    QColor clr;
    switch (index) {
    case 1:
        clr = QColor(255,255,255);
        break;
    case 2:
        clr = QColor(0, 0, 0);
        break;
    default:
        clr = QColorDialog::getColor(QColor(255,255,255), this);
        if (not clr.isValid())
            return;
    }
    QImage img(data.image.width()+2*w, data.image.height()+2*w, data.image.format());
    img.fill(clr);
    for (int y=0; y<data.image.height(); y++) {
        QRgb *src = (QRgb*)data.image.constScanLine(y);
        QRgb *dst = (QRgb*)img.scanLine(y+w);
        memcpy(dst+w, src, 4*data.image.width());
    }
    data.image = img;
    canvas->showScaled();
}

void
Window:: createPhotoGrid()
{
    GridDialog *dialog = new GridDialog(data.image, this);
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
    CollageItem *item = new CollageItem(data.image);
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
    InpaintDialog *dialog = new InpaintDialog(data.image, this);
    int dialog_h = screen_height - offset_y - offset_x;
    dialog->resize(1020, dialog_h);
    if (dialog->exec()==QDialog::Accepted) {
        canvas->setImage(dialog->image);
    }
}

void
Window:: iScissor()
{
    IScissorDialog *dialog = new IScissorDialog(data.image, this);
    int dialog_h = screen_height - offset_y - offset_x;
    dialog->resize(1020, dialog_h);
    if (dialog->exec()==QDialog::Accepted) {
        if (dialog->is_mask) {
            addMaskWidget();
            canvas->setMask( dialog->image );
        }
        else
            canvas->setImage( dialog->image );
    }
}

void
Window:: addMaskWidget()
{
    QWidget *maskWidget = new QWidget(this);
    QHBoxLayout *maskLayout = new QHBoxLayout(maskWidget);
    maskLayout->setContentsMargins(0, 0, 0, 0);
    maskWidget->setLayout(maskLayout);
    QPushButton *invertMaskBtn = new QPushButton("Invert Mask", maskWidget);
    QPushButton *clearMaskBtn = new QPushButton("Clear Mask", maskWidget);
    maskLayout->addWidget(invertMaskBtn);
    maskLayout->addWidget(clearMaskBtn);
    connect(clearMaskBtn, SIGNAL(clicked()), this, SLOT(removeMaskWidget()));
    connect(clearMaskBtn, SIGNAL(clicked()), maskWidget, SLOT(deleteLater()));
    connect(invertMaskBtn, SIGNAL(clicked()), canvas, SLOT(invertMask()));
    statusbar->addPermanentWidget(maskWidget);
    // allow only filters, and disable all other buttons
    disableButtons(FILE_BUTTON, true);
    disableButtons(EDIT_BUTTON, true);
    filtersBtn->setEnabled(true);
}

void
Window:: removeMaskWidget()
{
    canvas->clearMask();
    disableButtons(FILE_BUTTON, false);
    disableButtons(EDIT_BUTTON, false);
}

void
Window:: lensDistort()
{
    QImage img = canvas->pixmap()->toImage();
    LensDialog *dlg = new LensDialog(canvas, img, 1.0);
    if (dlg->exec()==QDialog::Accepted) {
        lensDistortion(data.image, dlg->main, dlg->edge, dlg->zoom);
    }
    canvas->showScaled();
}

void
Window:: toGrayScale()
{
    grayScale(data.image);
    canvas->showScaled();
}

void
Window:: applyThreshold()
{
    QImage img = canvas->pixmap()->toImage();
    ThresholdDialog *dlg = new ThresholdDialog(canvas, img, 1.0);
    if (dlg->exec()==QDialog::Accepted) {
        threshold(data.image, dlg->thresh);
    }
    canvas->showScaled();
}

void
Window:: adaptiveThresh()
{
    adaptiveThreshold(data.image);
    canvas->showScaled();
}

void
Window:: blurorbox(int method)
{
    bool ok;
    int radius = QInputDialog::getInt(this, "Blur Radius", "Enter Blur Radius :",
                                        1/*val*/, -30/*min*/, 30/*max*/, 1/*step*/, &ok);
    if ((not ok) || (radius == 0)) return;
    if (radius < 0)
    {
        QImage imgb = data.image.copy();
        if (method > 0)
            gaussianBlur(imgb, -radius);
        else
            boxFilter(imgb, -radius);
        imgb = reFilter(imgb, data.image, 1.0f);
        data.image = imgb;
    } else {
        if (method > 0)
            gaussianBlur(data.image, radius);
        else
            boxFilter(data.image, radius);
    }
    canvas->showScaled();
}

void
Window:: box()
{
    blurorbox(0);
}

void
Window:: blur()
{
    blurorbox(1);
}

void
Window:: deblur()
{
    bool ok;
    int radius = QInputDialog::getInt(this, "Blur Radius", "Enter Blur Radius :",
                                        1/*val*/, 1/*min*/, 30/*max*/, 1/*step*/, &ok);
    if ((not ok) || (radius <= 0)) return;
    QImage imgb = data.image.copy();
    gaussianBlur(imgb, radius);
    int w = imgb.width();
    int h = imgb.height();
    int dn = (data.image.hasAlphaChannel()) ? 4 : 3;
    for (int y = 0; y < h; y++)
    {
        QRgb *row = (QRgb*)data.image.constScanLine(y);
        QRgb *rowb = (QRgb*)imgb.constScanLine(y);
        for (int x = 0; x < w; ++x)
        {
            int pim, bim, ims, imsd, r, g, b, a;
            int clr = row[x], clrb = rowb[x];

            pim = qRed(clr);
            bim = qRed(clrb);
            ims = pim;
            ims -= 127;
            imsd = (pim > bim) ? (pim - bim) : (bim - pim);
            imsd += 255;
            ims *= imsd;
            ims *= imsd;
            ims /= 255;
            ims /= 255;
            ims += 127;
            r = Clamp(ims);
            pim = qGreen(clr);
            bim = qGreen(clrb);
            ims = pim;
            ims -= 127;
            imsd = (pim > bim) ? (pim - bim) : (bim - pim);
            imsd += 255;
            ims *= imsd;
            ims *= imsd;
            ims /= 255;
            ims /= 255;
            ims += 127;
            g = Clamp(ims);
            pim = qBlue(clr);
            bim = qBlue(clrb);
            ims = pim;
            ims -= 127;
            imsd = (pim > bim) ? (pim - bim) : (bim - pim);
            imsd += 255;
            ims *= imsd;
            ims *= imsd;
            ims /= 255;
            ims /= 255;
            ims += 127;
            b = Clamp(ims);
            pim = qAlpha(clr);
            bim = qAlpha(clrb);
            ims = pim;
            ims -= 127;
            imsd = (pim > bim) ? (pim - bim) : (bim - pim);
            imsd += 255;
            ims *= imsd;
            ims *= imsd;
            ims /= 255;
            ims /= 255;
            ims += 127;
            a = Clamp(ims);
            row[x] = (dn > 3) ? qRgba(r, g, b, a) : qRgb(r, g, b);
        }
    }
    canvas->showScaled();
}

void
Window:: sharpenImage()
{
    unsharpMask(data.image);
    canvas->showScaled();
}

void
Window:: reduceSpeckleNoise()
{
    despeckle(data.image);
    canvas->showScaled();
}

void
Window:: removeDust()
{
    medianFilter(data.image, 1);
    canvas->showScaled();
}

void
Window:: sigmoidContrast()
{
    sigmoidalContrast(data.image, 0.3);
    canvas->showScaled();
}
void
Window:: enhanceLight()
{
    stretchContrast(data.image);
    canvas->showScaled();
}

void
Window:: gammaCorrection()
{
    QImage img = canvas->pixmap()->toImage();
    GammaDialog *dlg = new GammaDialog(canvas, img, 1.0);
    if (dlg->exec()==QDialog::Accepted) {
        applyGamma(data.image, dlg->gamma);
    }
    canvas->showScaled();
}

void
Window:: whiteBalance()
{
    autoWhiteBalance(data.image);
    canvas->showScaled();
}

void
Window:: grayWorldFilter()
{
    grayWorld(data.image);
    canvas->showScaled();
}

void
Window:: enhanceColors()
{
    enhanceColor(data.image);
    canvas->showScaled();
}

void
Window:: vignetteFilter()
{
    vignette(data.image);
    canvas->showScaled();
}

void
Window:: pencilSketchFilter()
{
    pencilSketch(data.image);
    canvas->showScaled();
}

void
Window:: openPrevImage()
{
    QFileInfo fi(data.filename);
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
    QString nextfile = getNextFileName(data.filename);
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
    if (data.image.width() < 200 and data.image.height() < 200 and canvas->scale>=1)
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
    if (data.image.width() < 200 and data.image.height() < 200 and canvas->scale>1)
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
        canvas->scale = fitToWindowScale(data.image);
        canvas->showScaled();
        origSizeBtn->setIcon(QIcon(":/icons/originalsize.png"));
        return;
    }
    canvas->scale = 1.0;
    canvas->showScaled();
    origSizeBtn->setIcon(QIcon(":/icons/fit-to-screen.png"));
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
    frame->hide();
    frame_2->hide();
    setWindowTitle("Perspective Transform");
    PerspectiveTransform *transform = new PerspectiveTransform(canvas, statusbar);
    connect(canvas, SIGNAL(mousePressed(QPoint)), transform, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), transform, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), transform, SLOT(onMouseMove(QPoint)));
    connect(transform, SIGNAL(finished()), this, SLOT(onEditingFinished()));
}

void
Window:: deOblique()
{
    frame->hide();
    frame_2->hide();
    setWindowTitle("DeOblique");
    DeOblique *transform = new DeOblique(canvas, statusbar);
    connect(canvas, SIGNAL(mousePressed(QPoint)), transform, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), transform, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), transform, SLOT(onMouseMove(QPoint)));
    connect(transform, SIGNAL(finished()), this, SLOT(onEditingFinished()));
}

void
Window:: deWarping()
{
    DeWarpDialog *dlg = new DeWarpDialog(this);
    if (dlg->exec() != QDialog::Accepted)
        return;
    int countnodes = dlg->countSpin->value();
    frame->hide();
    frame_2->hide();
    setWindowTitle("DeWarping");
    DeWarping *transform = new DeWarping(canvas, statusbar, countnodes);
    connect(canvas, SIGNAL(mousePressed(QPoint)), transform, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), transform, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), transform, SLOT(onMouseMove(QPoint)));
    connect(transform, SIGNAL(finished()), this, SLOT(onEditingFinished()));
}

void
Window:: playPause()
{
    if (timer->isActive()) {       // Stop slideshow
        timer->stop();
        playPauseBtn->setIcon(QIcon(":/icons/play.png"));
        return;
    }
    if (canvas->animation) {
        if (canvas->movie()->state()==QMovie::Running) {
            canvas->movie()->setPaused(true);
            playPauseBtn->setIcon(QIcon(":/icons/play.png"));
        }
        else {
            canvas->movie()->setPaused(false);
            playPauseBtn->setIcon(QIcon(":/icons/pause.png"));
        }
    }
    else {// Start slideshow
        timer->start(3000);
        playPauseBtn->setIcon(QIcon(":/icons/pause.png"));
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
Window:: resizeToOptimum()
{
    canvas->scale = fitToScreenScale(data.image);
    canvas->showScaled();
    adjustWindowSize();
}

void
Window:: showNotification(QString title, QString message)
{
    Notifier *notifier = new Notifier(this);
    notifier->notify(title, message);
}

void
Window:: updateStatus()
{
    int width = data.image.width();
    int height = data.image.height();
    QString text = "Resolution : %1x%2 , Scale : %3x";
    statusbar->showMessage(text.arg(width).arg(height).arg(roundOff(canvas->scale, 2)));
}

// hide if not hidden, unhide if hidden
void
Window:: onEditingFinished()
{
    frame->show();
    frame_2->show();
    setWindowTitle(QFileInfo(data.filename).fileName());
}

void
Window:: disableButtons(ButtonType type, bool disable)
{
    switch (type) {
    case FILE_BUTTON:
        fileBtn->setDisabled(disable);
        prevBtn->setDisabled(disable);
        nextBtn->setDisabled(disable);
        playPauseBtn->setDisabled(disable);
        break;
    case VIEW_BUTTON:
        zoomInBtn->setDisabled(disable);
        zoomOutBtn->setDisabled(disable);
        origSizeBtn->setDisabled(disable);
        break;
    case EDIT_BUTTON:
        resizeBtn->setDisabled(disable);
        cropBtn->setDisabled(disable);
        transformBtn->setDisabled(disable);
        decorateBtn->setDisabled(disable);
        toolsBtn->setDisabled(disable);
        filtersBtn->setDisabled(disable);
        rotateLeftBtn->setDisabled(disable);
        rotateRightBtn->setDisabled(disable);
    }
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
QString getNextFileName(QString current)
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

QString getNewFileName(QString filename)
{
    // assuming filename is valid string
    QFileInfo fi(filename);
    QString dir = fi.dir().path();
    if (not dir.isEmpty()) dir += "/";
    QString basename = fi.completeBaseName();
    QString ext = fi.suffix().isEmpty()? ".jpg": "."+fi.suffix();
    // extract the num just before the file extension
    QRegExp rx("(.*\\D)*(\\d*)");
    int pos = rx.indexIn(basename);
    if (pos==-1) return getNewFileName(dir + "newimage.jpg");

    int num = rx.cap(2).isEmpty()? 0: rx.cap(2).toInt();

    QString path(dir + basename + ext);
    while (QFileInfo(path).exists()){
        path = dir + rx.cap(1) + QString::number(++num) + ext;
    }
    return path;
}

// ************* main function ****************

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("photoquick");
    app.setApplicationName("photoquick");
#ifdef _WIN32
    // this is needed to load imageformat plugins
    app.addLibraryPath(app.applicationDirPath());
#endif
    Window *win = new Window();
    win->show();
    if (argc > 1) {
        QString path = QString::fromUtf8(argv[1]);
        win->openImage(path);
    }
    else {
        QImage img = QImage(":/photoquick.jpg");
        win->canvas->setImage(img);
        win->adjustWindowSize();
    }
    // plugins will be loaded after first image is shown.
    // Thus even if it has hundreds plugins, startup will not be slower
    QTimer::singleShot(30, win, SLOT(loadPlugins()));
    return app.exec();
}
