/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"

// resize to fit if W and H is larger than max_w and max_h keeping aspect ratio
void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h)
{
    if (W<=max_w and H<=max_h) {
        out_w = W;
        out_h = H;
        return;
    }
    out_w = max_w;
    out_h = round((max_w/(float)W)*H);
    if (out_h > max_h) {
        out_h = max_h;
        out_w = round((max_h/(float)H)*W);
    }
}

// round off a float upto given decimal point
float roundOff(float num, int dec)
{
    double m = (num < 0.0) ? -1.0 : 1.0;   // check if input is negative
    double pwr = pow(10, dec);
    return float(floor((double)num * m * pwr + 0.5) / pwr) * m;
}

// waits for the specified time in milliseconds
void waitFor(int millisec)
{
    // Creates an eventloop to wait for a time
    QEventLoop *loop = new QEventLoop();
    QTimer::singleShot(millisec, loop, SLOT(quit()));
    loop->exec();
    loop->deleteLater();
}

// load an image from file
QImage loadImage(QString fileName)
{
    QImage img(fileName);
    if (img.isNull()) return img;
    // Converted because filters can only be applied to RGB32 or ARGB32 image
    if (img.hasAlphaChannel() && img.format()!=QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);
    else if (!img.hasAlphaChannel() and img.format()!=QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);
    // Get jpg orientation
/*
* FIX long name: Debian Wheezy
*
    char *filename = fileName.toUtf8().data();
    FILE *f = fopen(filename, "rb");
*/
    FILE *f = fopen(fileName.toUtf8().data(), "rb");
    int orientation = getOrientation(f);
    fclose(f);
    // rotate if required
    QTransform transform;
    switch (orientation) {
        case 6:
            return img.transformed(transform.rotate(90));
        case 3:
            return img.transformed(transform.rotate(180));
        case 8:
            return img.transformed(transform.rotate(270));
    }
    return img;
}


int getJpgFileSize(QImage image, int quality)
{
    if (image.isNull()) return 0;

    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", quality);
    int filesize = bArray.size();
    bArray.clear();
    buffer.close();
    return filesize;
}


Notifier:: Notifier(QObject *parent): QSystemTrayIcon(QIcon(":/icons/photoquick.png"), parent)
{
}

void
Notifier:: notify(QString title, QString message)
{
    show();
    waitFor(200);
    showMessage(title, message);
    QTimer::singleShot(3000, this, SLOT(deleteLater()));
}


#ifdef DEBUG
void debug(const char *format, ...)
{
    va_list args ;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#else
void debug(const char *, ...) {/* do nothing*/}
#endif

// transformation begin
//Mean for Isometric mode Un-tilt (PerspectiveTransform)
QPointF meanx2(QPolygonF p)
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
QPointF stdevx2(QPolygonF p)
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

// arc is drawn from p1 to p2 through p3
// p3 is at diagonal corner of p
// angle is  drawn counter clock-wise, and direction of y axis is
// upward, while direction of image y axis is downward
void calcArc(QPointF p/*center*/, QPointF p1, QPointF p2, QPointF p3,
                                    float &start, float &span)
{
    float ang1, ang2, ang3, gr = 180.0f / 3.14159265f;
    ang1 = atan2(p.y() - p1.y(), p1.x() - p.x()) * gr;
    ang2 = atan2(p.y() - p2.y(), p2.x() - p.x()) * gr;
    ang3 = atan2(p.y() - p3.y(), p3.x() - p.x()) * gr;
    if (ang1 > ang2)
    {
        ang1 += ang2;
        ang2 = ang1 - ang2;
        ang1 -= ang2;
    }
    if ((ang1 < ang3) and (ang3 < ang2))
    {
        start = ang1;
        span = ang2 - ang1;
    }
    else
    {
        start = ang2;
        span = 360 - (ang2 - ang1);
    }
}

float calcArea(QPolygonF p)
{
    int i, n = p.count();
    float area = 0.0f;
    if (n > 0)
    {
        area = p[0].y() * p[n - 1].x() - p[0].x() * p[n - 1].y();
        for (i = 0; i < n - 1; i++)
        {
            area += (p[i + 1].y() * p[i].x() - p[i + 1].x() * p[i].y());
        }
        area *= 0.5f;
        area = (area < 0) ? -area : area;
    }
    return area;
}

float InterpolateLagrangePolynomial (float x, QPolygonF p)
{
    int i, j, n = p.count();
    float basics_pol, lagrange_pol = 0.0f;

    for (i = 0; i < n; i++)
    {
        basics_pol = 1.0f;
        for (j = 0; j < n; j++)
        {
            if (j != i)
                basics_pol *= (x - p[j].x())/(p[i].x() - p[j].x());
        }
        lagrange_pol += basics_pol * p[i].y();
    }
    return lagrange_pol;
}

int SelectChannelPixel(QRgb pix, int channel)
{
    int value;
    switch (channel)
    {
     case 0:
        value = qRed(pix);
        break;
    case 1:
        value = qGreen(pix);
        break;
    case 2:
        value = qBlue(pix);
        break;
    default:
        value = qAlpha(pix);
        break;
    }
    return value;
}

QRgb InterpolateBiCubic (QImage img, float y, float x)
{
    int i, d, dn, xi, yi, xf, yf;
    float d0, d2, d3, a0, a1, a2, a3;
    float dx, dy, k2 = 1.0f / 2.0f, k3 = 1.0f / 3.0f, k6 = 1.0f / 6.0f;
    float Cc, C[4];
    int Ci, pt[4];
    int height = img.height();
    int width = img.width();;
    QRgb *row, imgpix;

    yi = (int)y;
    dy = y - yi;
    yi = (yi < 0) ? 0 : (yi < height) ? yi : (height - 1);
    xi = (int)x;
    dx = x - xi;
    xi = (xi < 0) ? 0 : (xi < width) ? xi : (width - 1);
    dn = (img.hasAlphaChannel()) ? 4 : 3;
    for(d = 0; d < dn; d++)
    {
        if (dy > 0.0f)
        {
            for(i = -1; i < 3; i++)
            {
                yf = (int)y + i;
                yf = (yf < 0) ? 0 : (yf < height) ? yf : (height - 1);
                row = (QRgb*)img.constScanLine(yf);
                if (dx > 0.0f)
                {
                    xf = xi;
                    a0 = SelectChannelPixel(row[xf], d);
                    xf = (int)x - 1;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d0 = SelectChannelPixel(row[xf], d);
                    d0 -= a0;
                    xf = (int)x + 1;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d2 = SelectChannelPixel(row[xf], d);
                    d2 -= a0;
                    xf = (int)x + 2;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d3 = SelectChannelPixel(row[xf], d);
                    d3 -= a0;
                    a1 = -k3 * d0 + d2 - k6 * d3;
                    a2 = k2 * d0 + k2 * d2;
                    a3 = -k6 * d0 - k2 * d2 + k6 * d3;
                    C[i + 1] = a0 + (a1 + (a2 + a3 * dx) * dx) * dx;
                }
                else
                {
                    xf = xi;
                    C[i + 1] = SelectChannelPixel(row[xf], d);
                }
            }
            a0 = C[1];
            d0 = C[0] - a0;
            d2 = C[2] - a0;
            d3 = C[3] - a0;
            a1 = -k3 * d0 + d2 - k6 * d3;
            a2 = k2 * d0 + k2 * d2;
            a3 = -k6 * d0 - k2 * d2 + k6 * d3;
            Cc = a0 + (a1 + (a2 + a3 * dy) * dy) * dy;
        }
        else
        {
            yf = yi;
            row = (QRgb*)img.constScanLine(yf);
            if (dx > 0.0f)
            {
                xf = xi;
                a0 = SelectChannelPixel(row[xf], d);
                xf = (int)x - 1;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d0 = SelectChannelPixel(row[xf], d);
                d0 -= a0;
                xf = (int)x + 1;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d2 = SelectChannelPixel(row[xf], d);
                d2 -= a0;
                xf = (int)x + 2;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d3 = SelectChannelPixel(row[xf], d);
                d3 -= a0;
                a1 = -k3 * d0 + d2 - k6 * d3;
                a2 = k2 * d0 + k2 * d2;
                a3 = -k6 * d0 - k2 * d2 + k6 * d3;
                Cc = a0 + (a1 + (a2 + a3 * dx) * dx) * dx;
            }
            else
            {
                xf = xi;
                Cc = SelectChannelPixel(row[xf], d);
            }
        }
        Ci = (int)(Cc + 0.5f);
        pt[d] = (Ci < 0) ? 0 : (Ci > 255) ? 255 : Ci;
    }
    imgpix = (dn > 3) ? qRgba(pt[0], pt[1], pt[2],  pt[3]) : qRgb(pt[0], pt[1], pt[2]);

    return imgpix;
}
// transformation end
