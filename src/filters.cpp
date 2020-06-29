// this file is part of photoquick program which is GPLv3 licensed
#include "filters.h"
#include "common.h"
#include <cmath>

// macros for measuring execution time
#include <chrono>
#define TIME_START auto start = std::chrono::steady_clock::now();
#define TIME_STOP auto end = std::chrono::steady_clock::now();\
    double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();\
    qDebug() << "Execution Time :" << elapse;

#define PI 3.141592654

#define BLEND(top, btm, alpha_top) ((top)*(alpha_top) + (btm)*(1-alpha_top));
// Byte order is ARGB if big endian else BGRA
inline bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}

// clamp an integer in 0-255 range
#define Clamp(a) ( (a)&(~0xff) ? (uchar)((~a)>>31) : (a) )

// Reference : http://www.easyrgb.com/en/math.php
/* HSV colorspace utilities
we are using 32 bit unsigned int to hold a hsv color
first 16 bit is for hue, next 8 bit is for saturation and last
8 bit is for value
h = 0-359, s = 0-255, v = 0-255
*/

typedef unsigned int QHsv;

inline int qHue(QHsv hsv) { return ((hsv>>16) & 0xffff); }
// sat and val has same pos like green and blue i.e last 2 bytes
#define qSat qGreen
#define qVal qBlue

inline QHsv qHsv(int h, int s, int v)
{
    return ( (h & 0xffffu)<<16 | (s & 0xffu)<<8 | (v & 0xffu) );
}

inline void rgbToHsv(QRgb rgb, int &h, int &s, int &v)
{
    float r = qRed(rgb)/255.0;
    float g = qGreen(rgb)/255.0;
    float b = qBlue(rgb)/255.0;
    float mx = MAX(MAX(r,g),b);
    float mn = MIN(MIN(r,g),b);
    float df = mx - mn;

    if (mx==mn)
        h = 0;
    else if (mx==r)
        h = (int)roundf(60*(g-b)/df+360) % 360;
    else if (mx==g)
        h = (int)roundf(60*(b-r)/df+120) % 360;
    else if (mx==b)
        h = (int)roundf(60*(r-g)/df+240) % 360;

    s = (mx==0)? 0 : roundf(255*df/mx);
    v = 255*mx;
}
// calculate remainder i.e a%b (works in case of float)
#define mod(a,b) ( (a) - ((int)((a)/(b)))*(b) )
void hsvToRgb(QHsv hsv, int &r, int &g, int &b)
{
    float h = qHue(hsv)/60.0;
    float s = qSat(hsv)/255.0;
    float v = qVal(hsv)/255.0;
    float k;
    k = mod(5 + h, 6);
    r = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
    k = mod(3 + h, 6);
    g = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
    k = mod(1 + h, 6);
    b = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
}

// convert rgb image to hsv image
void hsvImg(QImage &img)
{
    int w = img.width();
    int h = img.height();
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QHsv *row;
        int h=0,s=0,v=0;
        #pragma omp critical
        { row = (QHsv*)img.scanLine(y);}
        for (int x=0; x<w; x++) {
            rgbToHsv(row[x],h,s,v);
            row[x] = qHsv(h,s,v);
        }
    }
}

// HCL <--> RGB colorspace conversion functions
// Hue = 0->359, Chroma=0->131, Luminance = 0->100
// it is same as CIE LCH colorspace, but here components are in reverse order
typedef unsigned int QHcl;
#define qHcl qHsv
#define qCro qGreen
#define qLum qBlue

// D65 standard referent
#define LAB_Xn 0.950470
#define LAB_Yn 1
#define LAB_Zn 1.088830

#define LAB_t0 0.137931034  // 16 / 116
#define LAB_t1 0.206896552  // 24 / 116
#define LAB_t2 0.12841855   // 3 * t1 * t1
#define LAB_t3 0.008856452  // t1^3

float rgb_xyz(float r) { //sRGB to Linear
    if ((r /= 255) <= 0.04045) { return r / 12.92; }
    return pow((r + 0.055) / 1.055, 2.4);
}

float xyz_lab(float t)
{
    if (t > LAB_t3) { return pow(t, 1.0 / 3); }
    return t / LAB_t2 + LAB_t0;
}

void rgbToHcl(QRgb rgb, int &h, int &c, int &l)
{
    float r,g,b, x,y,z,a;
    r = rgb_xyz(qRed(rgb));
    g = rgb_xyz(qGreen(rgb));
    b = rgb_xyz(qBlue(rgb));
    // convert to xyz colorspace
    x = xyz_lab((0.4124564 * r + 0.3575761 * g + 0.1804375 * b) / LAB_Xn);
    y = xyz_lab((0.2126729 * r + 0.7151522 * g + 0.0721750 * b) / LAB_Yn);
    z = xyz_lab((0.0193339 * r + 0.1191920 * g + 0.9503041 * b) / LAB_Zn);
    // convert to LAB colorspace
    l = 116 * y - 16;
    l = l < 0 ? 0 : l,
    a = 500 * (x - y);
    b = 200 * (y - z);
    // convert to HCL colorspace
    c = sqrt(a*a + b*b);
    h = (int)round(atan2(b, a) * 180/PI + 360) % 360;
}

float xyz_rgb(float r) { //linear to sRGB
    return 255 * (r <= 0.00304 ? 12.92 * r : 1.055 * pow(r, 1 / 2.4) - 0.055);
};

float lab_xyz(float t) {
    return t > LAB_t1 ? t*t*t : LAB_t2 * (t - LAB_t0);
};

void hclToRgb(QHcl hcl, int &r, int &g, int &b_)
{
    float x,y,z, a,b, h,c,l;
    h = qHue(hcl);
    c = qCro(hcl);
    l = qLum(hcl); // L=[0..100]
    // convert lch to LAB colorspace
    h *= PI/180;
    a = cos(h)*c; // A=[-100..100]
    b = sin(h)*c; // B=[-100..100]
    // convert lab to xyz colorspace
    y = (l + 16) / 116;
    x = a==0 ? y : y + a/500;
    z = b==0 ? y : y - b/200;

    x = LAB_Xn * lab_xyz(x);
    y = LAB_Yn * lab_xyz(y);
    z = LAB_Zn * lab_xyz(z);
    // convert xyz to rgb
    r = Clamp((int)xyz_rgb(3.2404542 * x - 1.5371385 * y - 0.4985314 * z));
    g = Clamp((int)xyz_rgb(-0.9692660 * x + 1.8760108 * y + 0.0415560 * z));
    b_ = Clamp((int)xyz_rgb(0.0556434 * x - 0.2040259 * y + 1.0572252 * z));
}

// Expand each size of Image by certain amount of border
QImage expandBorder(QImage img, int width)
{
    int w = img.width();
    int h = img.height();
    QImage dst = QImage(w+2*width, h+2*width, img.format());
    // copy all image pixels at the center
    QRgb *row, *dstRow;
    for (int y=0; y<h; y++) {
        row = (QRgb*)img.constScanLine(y);
        dstRow = (QRgb*)dst.scanLine(y+width);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate first row
    row = (QRgb*)img.constScanLine(0);
    for (int i=0; i<width; i++) {
        dstRow = (QRgb*)dst.scanLine(i);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate last row
    row = (QRgb*)img.constScanLine(h-1);
    for (int i=0; i<width; i++) {
        dstRow = (QRgb*)dst.scanLine(width+h+i);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate left and right sides
    QRgb left_clr, right_clr;
    for (int y=0; y<h; y++) {
        dstRow = (QRgb*)dst.scanLine(y+width);
        left_clr = ((QRgb*)img.constScanLine(y))[0];
        right_clr = ((QRgb*)img.constScanLine(y))[w-1];
        for (int x=0; x<width; x++) {
            dstRow[x] = left_clr;
            dstRow[width+w+x] = right_clr;
        }
    }
    // duplicate corner pixels
    row = (QRgb*)img.constScanLine(0);
    for (int y=0; y<width; y++) {
        dstRow = (QRgb*)dst.scanLine(y);
        for (int x=0; x<width; x++) {
            dstRow[x] = row[0];
            dstRow[width+w+x] = row[w-1];
        }
    }
    row = (QRgb*)img.constScanLine(h-1);
    for (int y=0; y<width; y++) {
        dstRow = (QRgb*)dst.scanLine(width+h+y);
        for (int x=0; x<width; x++) {
            dstRow[x] = row[0];
            dstRow[width+w+x] = row[w-1];
        }
    }
    return dst;
}


//********** --------- Gray Scale Image --------- ********** //
void grayScale(QImage &img)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y));}
        for (int x=0;x<img.width();x++) {
            int val = qGray(line[x]);
            line[x] = qRgba(val,val,val, qAlpha(line[x]));
        }
    }
}

//********* ---------- Invert Colors or Negate --------- ********** //
void invert(QImage &img)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y));}
        for (int x=0;x<img.width();x++) {
            line[x] = qRgba(255-qRed(line[x]), 255-qGreen(line[x]), 255-qBlue(line[x]), qAlpha(line[x]));
        }
    }
}


//********* --------- Global Threshold -------- ***********//
#define HISTOGRAM_SIZE 256

int calcOtsuThresh(QImage img)
{
    // Compute number of pixels
    int N = img.width()*img.height();

    // Create Histogram
    unsigned int histogram[HISTOGRAM_SIZE] = {};
    int x, y;
    for (y = 0; y < img.height(); ++y)
    {
        QRgb *row = (QRgb*)img.constScanLine(y);
        for (x = 0; x < img.width(); x++)
            ++histogram[qGray(row[x])];
    }

    // Calculate sum
    int sum = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx)
        sum += idx * histogram[idx];

    // Compute threshold
    int threshold = 0;
    int sumB = 0;
    int q1 = 0;
    double max = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx)
    {
        q1 += histogram[idx]; // q1 = Weighted Background
        if (q1 == 0)
            continue;

        const int q2 = N - q1; // q2 = Weighted Forground
        if (q2 == 0)
            break;

        sumB += (idx * histogram[idx]);

        const double m1m2 =
            (double)sumB / q1 -         // Mean Background
            (double)(sum - sumB) / q2;  // Mean Forground

        // Note - There is an insidious casting situation going on here.
        // If one were to multiple by q1 or q2 first, an explicit cast would be required!
        const double between = m1m2 * m1m2 * q1 * q2;

        if (between > max)
        {
            threshold = idx;
            max = between;
        }
    }
    return threshold;
}

void threshold(QImage &img, int thresh)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y)); }
        for (int x=0;x<img.width();x++) {
            if (qGray(line[x]) > thresh)
                line[x] = qRgb(255,255,255);
            else
                line[x] = qRgb(0,0,0);
        }
    }
}

//*********---------- Adaptive Threshold ---------**********//
// Apply Bradley threshold (to get desired output, tune value of T and s)
void adaptiveThreshold(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // Allocate memory for integral image
    int len = sizeof(int *) * h + sizeof(int) * w * h;
    unsigned **intImg = (unsigned **)malloc(len);

    unsigned *ptr = (unsigned*)(intImg + h);
    for(int i = 0; i < h; i++)
        intImg[i] = (ptr + w * i);

    // Calculate integral image
    for (int y=0;y<h;++y)
    {
        QRgb *row = (QRgb*)img.constScanLine(y);
        int sum=0;
        for (int x=0;x<w;++x)
        {
            sum += qGray(row[x]);
            if (y==0)
                intImg[y][x] = qGray(row[x]);
            else
                intImg[y][x] = intImg[y-1][x] + sum;
        }
    }
    // Apply Bradley threshold
    float T = 0.15;
    int s = w/32 > 16? w/32: 16;
    int s2 = s/2;
    #pragma omp parallel for
    for (int i=0;i<h;++i)
    {
        int x1,y1,x2,y2, count, sum;
        y1 = ((i - s2)>0) ? (i - s2) : 0;
        y2 = ((i + s2)<h) ? (i + s2) : h-1;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(i); }
        for (int j=0;j<w;++j)
        {
            x1 = ((j - s2)>0) ? (j - s2) : 0;
            x2 = ((j + s2)<w) ? (j + s2) : w-1;

            count = (x2 - x1)*(y2 - y1);
            sum = intImg[y2][x2] - intImg[y2][x1] - intImg[y1][x2] + intImg[y1][x1];

            // threshold = mean*(1 - T) , where mean = sum/count, T = around 0.15
            if ((qGray(row[j]) * count) < (int)(sum*(1.0 - T)) )
                row[j] = qRgb(0,0,0);
            else
                row[j] = qRgb(255,255,255);
        }
    }
    free(intImg);
}


//*********---------- Apply Convolution Matrix ---------**********//
// Kernel Width Must Be An Odd Number
// Image must be larger than Kernel Width
#if (0)
void convolve(QImage &img, float kernel[], int width/*of kernel*/)
{
    int radius = width/2;
    int w = img.width();
    int h = img.height();
    QImage tmp = expandBorder(img, radius);
    int tmp_w = tmp.width();

    /* Build normalized kernel */
    float normal_kernel[width][width]; // = {}; // Throws error in C99 compiler
    memset(normal_kernel, 0, (width*width) * sizeof(float));

    float normalize = 0.0;
    for (int i=0; i < (width*width); i++)
        normalize += kernel[i];
    // if (abs(normalize) == 0) normalize=1.0;
    normalize = 1.0/normalize;
    for (int i=0; i < (width); i++) {
        for (int j=0; j< width; j++)
            normal_kernel[i][j] = normalize*kernel[i*width+j];
    }

    /* Convolve image */
    QRgb *data = (QRgb*)img.scanLine(0);
    QRgb *tmpData = (QRgb*)tmp.constScanLine(0);
    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row = data+(y*w);

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
            for (int i=0; i < width; i++)
            {
                QRgb *tmpRow = tmpData+(tmp_w*(y+i));
                for (int j=0; j < width; j++)
                {
                    QRgb clr = tmpRow[x+j];
                    r += normal_kernel[i][j] * qRed(clr);
                    g += normal_kernel[i][j] * qGreen(clr);
                    b += normal_kernel[i][j] * qBlue(clr);
                }
            }
            row[x] = qRgba(round(r), round(g), round(b), qAlpha(row[x]));
        }
    }
}
#endif
// convolve a 1D kernel first left to right and then top to bottom
void convolve1D(QImage &img, float kernel[], int width/*of kernel*/)
{
    /* Build normalized kernel */
    float normal_kernel[width]; // = {}; // Throws error in C99 compiler
    memset(normal_kernel, 0, width * sizeof(float));

    float normalize = 0.0;
    for (int i=0; i < width; i++)
        normalize += kernel[i];
    // if (abs(normalize) == 0) normalize=1.0;
    normalize = 1.0/normalize;
    for (int i=0; i < (width); i++) {
        normal_kernel[i] = normalize * kernel[i];
    }

    int radius = width/2;
    int w = img.width();
    int h = img.height();
    /* Convolve image */
    QImage src_img = expandBorder(img, radius);
    int src_w = src_img.width();

    QRgb *data_src = (QRgb*)src_img.constScanLine(0);
    QRgb *data_dst = (QRgb*)img.scanLine(0);

    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row_dst = data_dst + (y*w);
        QRgb *row_src = data_src + (src_w*(y+radius));

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
            for (int i=0; i < width; i++)
            {
                QRgb clr = row_src[x+i];
                r += normal_kernel[i] * qRed(clr);
                g += normal_kernel[i] * qGreen(clr);
                b += normal_kernel[i] * qBlue(clr);
            }
            row_dst[x] = qRgba(round(r), round(g), round(b), qAlpha(row_dst[x]));
        }
    }
    // Convolve from top to bottom
    src_img = expandBorder(img, radius);
    data_src = (QRgb*)src_img.constScanLine(0);

    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row_dst = data_dst + (y*w);

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
            for (int i=0; i < width; i++)
            {
                QRgb *row_src = data_src + (src_w*(y+i));
                QRgb clr = row_src[x+radius];
                r += normal_kernel[i] * qRed(clr);
                g += normal_kernel[i] * qGreen(clr);
                b += normal_kernel[i] * qBlue(clr);
            }
            row_dst[x] = qRgba( round(r), round(g), round(b), qAlpha(row_dst[x]) );
        }
    }
}

//*************---------- Gaussian Blur ---------***************//
// 1D Gaussian kernel -> g(x)   = 1/{sqrt(2.pi)*sigma} * e^{-(x^2)/(2.sigma^2)}
// 2D Gaussian kernel -> g(x,y) = 1/(2.pi.sigma^2) * e^{-(x^2 +y^2)/(2.sigma^2)}

void gaussianBlur(QImage &img, int radius, float sigma/*standard deviation*/)
{
    if (sigma==0)  sigma = radius/2.0 ;
    int kernel_width = 2*radius + 1;
    // build 1D gaussian kernel
    float kernel[kernel_width];

    for (int i=0; i<kernel_width; i++)
    {
        int u = i - radius;
        double alpha = exp(-(u*u)/(2.0*sigma*sigma));
        kernel[i] = alpha/(sqrt(2*PI)*sigma);
    }
    convolve1D(img, kernel, kernel_width);
}


//**********----------- Box Blur -----------*************//
// also called mean blur
void boxFilter(QImage &img, int r/*blur radius*/)
{
    int w = img.width();
    int h = img.height();
    int kernel_w = 2*r + 1;

    QImage src_img = expandBorder(img, r);
    int src_w = src_img.width();
    QRgb *data_src = (QRgb*) src_img.constScanLine(0);
    QRgb *data_dst = (QRgb*) img.scanLine(0);

    #pragma omp parallel for
    for (int y=0; y<h; ++y)
    {
        QRgb *row_dst = data_dst + (y*w);
        QRgb *row_src = data_src + ((y+r)*src_w);

        int sum_r = 0, sum_g = 0, sum_b = 0;
        for (int x=0; x<kernel_w; x++) {
            int clr = row_src[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        row_dst[0] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w, qAlpha(row_dst[0]));

        for (int x=1; x<w; x++) {
            int left = row_src[x-1];
            int right = row_src[x+r+r];
            sum_r += qRed(right) - qRed(left);
            sum_g += qGreen(right) - qGreen(left);
            sum_b += qBlue(right) - qBlue(left);
            row_dst[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w, qAlpha(row_dst[x]));
        }
    }
    src_img = expandBorder(img, r);
    data_src = (QRgb*) src_img.constScanLine(0);

    #pragma omp parallel for
    for (int x=0; x<w; ++x)
    {
        int sum_r = 0, sum_g = 0, sum_b = 0;

        for (int y=0; y<kernel_w; y++) {
            int clr = (data_src + (y*src_w))[x+r];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        (data_dst)[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w,
                                qAlpha((data_dst)[x]));//first row

        for (int y=1; y<h; y++) {
            int clr_top = (data_src + ((y-1)*src_w))[x+r];
            int clr_btm = (data_src + ((y+r+r)*src_w))[x+r];
            sum_r += qRed(clr_btm) - qRed(clr_top);
            sum_g += qGreen(clr_btm) - qGreen(clr_top);
            sum_b += qBlue(clr_btm) - qBlue(clr_top);
            (data_dst + (y*w))[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w,
                                        qAlpha((data_dst + (y*w))[x]));
        }
    }
}


//*************------------ Sharpen ------------****************
// Using unsharp masking
// output_image = input_image + factor*(input_image - blur_image)

void unsharpMask(QImage &img, float factor, int thresh)
{
    QImage mask = img.copy();
    boxFilter(mask, 1);
    int w = img.width();
    int h = img.height();
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row, *row_mask;
        #pragma omp critical
        { row_mask = (QRgb*)mask.constScanLine(y);
          row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++)
        {
            int r_diff = (qRed(row[x]) - qRed(row_mask[x]));
            int g_diff = (qGreen(row[x]) - qGreen(row_mask[x]));
            int b_diff = (qBlue(row[x]) - qBlue(row_mask[x]));
            // default threshold = 5, factor = 1.0
            int r = r_diff > thresh? qRed(row[x])   + factor*r_diff : qRed(row[x]);
            int g = g_diff > thresh? qGreen(row[x]) + factor*g_diff : qGreen(row[x]);
            int b = b_diff > thresh? qBlue(row[x])  + factor*b_diff : qBlue(row[x]);
            row[x] = qRgba(Clamp(r), Clamp(g), Clamp(b), qAlpha(row[x]));
        }
    }
}

// *******-------- Sigmoidal Contrast ---------**********
// Sigmoidal Contrast Image to enhance low contrast image

#define Sigmoidal(a,b,x) ( tanh((0.5*(a))*((x)-(b))) )

#define ScaledSigmoidal(a,b,x,min,max) (                    \
  (Sigmoidal((a),(b),(x))-Sigmoidal((a),(b),(min))) / \
  (Sigmoidal((a),(b),(max))-Sigmoidal((a),(b),(min))) )

// midpoint => range = 0.0 -> 1.0 , default = 0.5
// contrast => range =   1 -> 20,   default = 3
void sigmoidalContrast(QImage &img, float midpoint)
{
    int w = img.width();
    int h = img.height();
    uchar histogram[256];
    for (int i=0; i<256; i++) {
        histogram[i] = 255*ScaledSigmoidal(3, midpoint, i/255.0, 0.0,1.0);
    }
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int r = histogram[qRed(clr)];
            int g = histogram[qGreen(clr)];
            int b = histogram[qBlue(clr)];
            row[x] = qRgba(r,g,b, qAlpha(clr));
        }
    }
}

/*********** ---------- Stretch Contrast ------------- ***************/
// perc = percentile to calculate (range = 0-100)
// N = total number of samples (i.e number of pixels)
int percentile(unsigned int histogram[], float perc, int N)
{
    int A=0;
    for (unsigned int index = N*perc/100; index > histogram[A]; A++) {
        index -= histogram[A];
    }
    return A;
}

void stretchContrast(QImage &img)
{
    float midpoint = 0.3;
    int w = img.width();
    int h = img.height();
    hsvImg(img);
    // Calculate percentile
    unsigned int histogram[256] = {};
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        row = (QRgb*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram[qVal(row[x])];
        }
    }
    int min = percentile(histogram, 0.5, w*h);
    int max = percentile(histogram, 99.5, w*h);
    for (int i=0; i<256; i++) {
        int val = 255*ScaledSigmoidal(3, midpoint, i/255.0, min/255.0, max/255.0);
        histogram[i] = Clamp(val);
    }
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        int r=0,g=0,b=0;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int hsv = qHsv(qHue(clr), qSat(clr), histogram[qVal(clr)]);
            hsvToRgb(hsv, r,g,b);
            row[x] = qRgb(r,g,b);
        }
    }
}

// ************* ------------ Auto White Balance -------------************
// adopted from a stackoverflow code (white patch algorithm)

void autoWhiteBalance(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // Calculate percentile
    unsigned int histogram_r[256] = {};
    unsigned int histogram_g[256] = {};
    unsigned int histogram_b[256] = {};
    for (int y=0; y<h; y++)
    {
        QRgb *row = (QRgb*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram_r[qRed(row[x])];
            ++histogram_g[qGreen(row[x])];
            ++histogram_b[qBlue(row[x])];
        }
    }
    int min_r = percentile(histogram_r, 0.5, w*h);
    int min_g = percentile(histogram_g, 0.5, w*h);
    int min_b = percentile(histogram_b, 0.5, w*h);
    int max_r = percentile(histogram_r, 99.5, w*h);
    int max_g = percentile(histogram_g, 99.5, w*h);
    int max_b = percentile(histogram_b, 99.5, w*h);
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y);}
        for (int x=0; x<w; x++) {
            int r = 255.0*(qRed(row[x]) - min_r)/(max_r-min_r);// stretch contrast
            int g = 255.0*(qGreen(row[x]) - min_g)/(max_g-min_g);
            int b = 255.0*(qBlue(row[x]) - min_b)/(max_b-min_b);
            row[x] = qRgb(Clamp(r), Clamp(g), Clamp(b));
        }
    }
}

// Color Balance using a version of Gray World Algorithm
void grayWorld(QImage &img)
{
    int y, x;
    long long mi = 0, mr = 0, mg = 0, mb = 0, n = 0, mn;
    float a0r = 0.0f, a0g = 0.0f, a0b = 0.0f;
    float a1r = 1.0f, a1g = 1.0f, a1b = 1.0f;
    float km = 0.001f / 3.0f, vm = 127.0f;
    int vr, vg, vb;
    QRgb* line;

    for (y = 0; y < img.height(); y++)
    {
        line = (QRgb*) img.scanLine(y);
        for (x = 0; x < img.width(); x++)
        {
            n++;
            mr += qRed(line[x]);
            mg += qGreen(line[x]);
            mb += qBlue(line[x]);
        }
    }
    mi = (mr + mg + mb) * 1000;
    mn = mi / n;
    vm = km * (float)mn;
    if (mr > 0)
    {
        mr = mi / mr;
        a1r = km * (float)mr;
    }
    else
    {
        a0r = vm;
    }
    if (mg > 0)
    {
        mg = mi / mg;
        a1g = km * (float)mg;
    }
    else
    {
        a0g = vm;
    }
    if (mb > 0)
    {
        mb = mi / mb;
        a1b = km * (float)mb;
    }
    else
    {
        a0b = vm;
    }
    for (y = 0; y < img.height(); y++)
    {
        line = (QRgb*) img.scanLine(y);
        for (x = 0; x < img.width(); x++)
        {
            vr = qRed(line[x]);
            vr = (int)(a1r * (float)vr + a0r);
            vr = Clamp(vr);
            vg = qGreen(line[x]);
            vg = (int)(a1g * (float)vg + a0g);
            vg = Clamp(vg);
            vb = qBlue(line[x]);
            vb = (int)(a1b * (float)vb + a0b);
            vb = Clamp(vb);
            line[x] = qRgba(vr, vg, vb, qAlpha(line[x]));
        }
    }
}


// ************ ------------ Enhance Color ----------- ************* //
// Convert to CIE LCH colorspace, and stretch the chroma
void enhanceColor(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // convert to HCL colorspace
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QHcl *row;
        int h=0,c=0,l=0;
        #pragma omp critical
        { row = (QHcl*)img.scanLine(y);}
        for (int x=0; x<w; x++) {
            rgbToHcl(row[x],h,c,l);
            row[x] = qHcl(h,c,l);
        }
    }
    // Calculate percentile
    unsigned int histogram[132] = {};
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        row = (QRgb*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram[qCro(row[x])];
        }
    }
    int min = percentile(histogram, 0, w*h);
    int max = percentile(histogram, 100, w*h);
    if (max==0) // in case of all gray pixels
        max = 100;
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        int r=0,g=0,b=0,c=0;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            c = 100*(qCro(clr)-min)/(max-min);
            hclToRgb(qHcl(qHue(clr),c,qLum(clr)), r,g,b);
            row[x] = qRgb(r,g,b);
        }
    }
}

// ********* ---------- Despecle ---------- ***********
// Crimmins speckle removal

void Hull(int x_offset, int y_offset, int w, int h, int polarity, uchar *f, uchar *g)
{
    uchar *p, *q, *r, *s, v;
    p = f + w+2;
    q = g + w+2;
    r = p + (y_offset*(w+2)+x_offset);
    for (int y=0; y < h; y++)
    {
        int i=(2*y+1)+y*w;
        if (polarity > 0)
            for (int x=0; x < w; x++)
            {
                v = p[i];
                if (r[i] >= (v+2)) //increase color by 2 unit
                    v+=1;
                q[i] = v;
                i++;
            }
        else
            for (int x=0; x < w; x++)
            {
                v= p[i];
                if ( r[i] <= (v-2))
                    v-=1;
                q[i]= v;
                i++;
            }
    }

    p = f + (w+2);
    q = g + (w+2);
    r = q + (y_offset*(w+2)+x_offset);
    s = q - (y_offset*(w+2)+x_offset);
    for (int y=0; y < h; y++)
    {
        int i=(2*y+1)+y*w;
        if (polarity > 0)
            for (int x=0; x < w; x++)
            {
                v=q[i];
                if ((s[i] >= (v+2)) &&
                    ( r[i] > v))
                    v+=1;
                p[i]=v;
                i++;
            }
        else
            for (int x=0; x < w; x++)
            {
                v=q[i];
                if ((s[i] <= (v-2)) &&
                    ( r[i] < v))
                    v-=1;
                p[i] = v;
                i++;
            }
    }
}

void despeckle(QImage &img)
{
    int w = img.width();
    int h = img.height();
    int X[4] = {0, 1, 1,-1}, Y[4] = {1, 0, 1, 1};
    int length = (w+2)*(h+2); // temp buffers contain 1 pixel border
    #pragma omp parallel for
    for (int i=0; i < 4; i++) // 4 channels ARGB32 image
    {
        if (i==0 and isBigEndian()) continue;  // skip Alpha for ARGB order
        if (i==3 and not isBigEndian()) continue;  // BGRA order
        // allocate memory for pixels array
        uchar *pixels = (uchar*)calloc(1,length);
        uchar *buffer = (uchar*)calloc(1,length);
        // draw image inside pixels array
        int j = w+2;    // leave first row
        for (int y=0; y < h; y++)
        {
            uchar *row;
            #pragma omp critical
            { row = (uchar*)img.constScanLine(y); }

            j++; //leave first column
            for (int x=0; x < w; x++)
            {
                pixels[j++] = row[x*4+i]; // clone image
            }
            j++; // leave last column
        }
        // reduce speckle noise
        for (int k=0; k < 4; k++)
        {
            Hull( X[k], Y[k], w,h, 1,pixels,buffer);
            Hull(-X[k],-Y[k], w,h, 1,pixels,buffer);
            Hull(-X[k],-Y[k], w,h,-1,pixels,buffer);
            Hull( X[k], Y[k], w,h,-1,pixels,buffer);
        }
        // draw pixels array over original image
        j=w+2;
        for (int y=0; y < h; y++)
        {
            uchar *row;
            #pragma omp critical
            { row = (uchar*)img.scanLine(y); }
            j++;
            for (int x=0; x < w; x++)
            {
                row[x*4+i] = pixels[j++];
            }
            j++;
        }
        free(buffer);
        free(pixels);
    }
}


// ******** ---------- Median Filter ---------**********//
// Edge preserving noise reduction filter to Reduce Salt & Pepper noise.
// Algorithm implemented from graphicsmagick source which is based on the
// paper "Skip Lists: A probabilistic Alternative to Balanced Trees"
// by William Pugh (1990)
typedef struct
{
  unsigned int
    next[9],
    count,
    signature;
} MedianListNode;

typedef struct
{
  MedianListNode *nodes;
  int level;
} MedianSkipList;

typedef struct
{
  MedianSkipList list;

  unsigned int
    center,
    seed,
    signature;
} MedianPixelList;


void AddNodeMedianList(MedianPixelList *pixel_list, unsigned int color)
{
    MedianSkipList list = pixel_list->list;
    int level;
    unsigned int search, update[9];

    list.nodes[color].signature = pixel_list->signature;
    list.nodes[color].count = 1;
    // Determine where it belongs in the list.
    // This loop consumes most of the time.
    search = 256;
    for (level=list.level; level >= 0; level--)
    {
        while (list.nodes[search].next[level] < color)
            search = list.nodes[search].next[level];
        update[level] = search;
    }
    // Generate a pseudo-random level for this node.
    for (level=0; ; level++)
    {
        pixel_list->seed = (pixel_list->seed*42893621u)+1u;
        if ((pixel_list->seed & 0x300) != 0x300)
            break;
    }
    if (level > 8)
        level = 8;
    if (level > (list.level+2))
        level = list.level+2;
    // If we're raising the list's level, link back to the root node.
    while (level > list.level)
    {
        list.level++;
        update[list.level] = 256;
    }
    //Link the node into the skip-list.
    do
    {
        list.nodes[color].next[level] = list.nodes[update[level]].next[level];
        list.nodes[update[level]].next[level] = color;
    }
    while (level-- > 0);
}

inline
void InsertMedianList(MedianPixelList *pixel_list, uchar pixel)
{
    unsigned int index = (unsigned short)(pixel);
    if (pixel_list->list.nodes[index].signature == pixel_list->signature)
        pixel_list->list.nodes[index].count++;
    else
        AddNodeMedianList(pixel_list, index);
}

void ResetMedianList(MedianPixelList *pixel_list)
{
    MedianListNode *root;
    MedianSkipList list;

    list = pixel_list->list;
    root = list.nodes+256;
    list.level = 0;
    for (int level=0; level < 9; level++)
        root->next[level] = 256;

    pixel_list->seed = pixel_list->signature++;
}

void DestroyMedianList(void *pixel_list)
{
    MedianPixelList *skiplist = (MedianPixelList *) pixel_list;

    if (skiplist != (void *) NULL)
        free(skiplist->list.nodes);
    free(skiplist);
}

MedianPixelList* AllocateMedianList(const int width)
{
    MedianPixelList *skiplist;

    skiplist = (MedianPixelList *) calloc(1,sizeof(MedianPixelList));//TODO:align to 64
    if (skiplist == (MedianPixelList *) NULL)
        return skiplist;

    skiplist->center=width*width/2;
    skiplist->signature = 0xabacadabUL; //MagickSignature;
    skiplist->list.nodes = (MedianListNode*) calloc(1, 257*sizeof(MedianListNode));
    if (skiplist->list.nodes == (MedianListNode *) NULL)
    {
        DestroyMedianList(skiplist);
        skiplist=(MedianPixelList *) NULL;
    }
    return skiplist;
}

QRgb GetMedian(MedianPixelList *pixel_list)
{
    MedianSkipList list=pixel_list->list;

    unsigned int center,color,count;

    // Finds the median value
    center = pixel_list->center;
    color = 256;
    count = 0;
    do
    {
        color = list.nodes[color].next[0];
        count += list.nodes[color].count;
    }
    while (count <= center);
    return color;
}


void medianFilter(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    QImage tmp = expandBorder(img, radius);
    int tmp_w = tmp.width();

    uchar *data = (uchar*)img.scanLine(0);
    uchar *tmpData = (uchar*)tmp.constScanLine(0);

    #pragma omp parallel for
    for (int channel=0; channel<4; channel++) {
        MedianPixelList *skiplist = AllocateMedianList(2*radius+1);

        for (int y=0; y < h; y++)
        {
            uchar *row = data + (y*w*4);
            for (int x=0; x < w; x++)
            {
                ResetMedianList(skiplist);
                for (int i=y-radius; i <= y+radius; i++)
                {
                    uchar *tmpRow = tmpData + ((i+radius)*tmp_w*4);
                    for (int j=x-radius; j<=x+radius; j++)
                        InsertMedianList(skiplist, tmpRow[4*(j+radius)+channel]);
                }
                row[4*x+channel] = GetMedian(skiplist);
            }
        }
        DestroyMedianList(skiplist);
    }
}



// ********************* Experimental Features  ********************* //

// ************* ------------ Skin Detection -------------************ /
#if (0)
inline bool isSkin(int r, int g, int b){
    int h=0,s=0,v=0;
    rgbToHsv(qRgb(r,g,b), h,s,v);
    return (r>g) and (r>b) and (r>60) and (g>40) and (b>20) and (abs(r-g)>15) and
    ((h>350) or (h<35)) and (s<170) and (v < 310-s);
}
#endif
