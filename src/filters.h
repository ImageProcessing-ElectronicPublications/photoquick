#pragma once
#include <cmath>
#include <chrono>
#include <QImage>
#include <QPainter>
#include "common.h"

#ifndef __PHOTOQUIK_FILTERS
#define __PHOTOQUIK_FILTERS

// macros for measuring execution time
#define TIME_START auto start = std::chrono::steady_clock::now();
#define TIME_STOP auto end = std::chrono::steady_clock::now();\
    double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();\
    qDebug() << "Execution Time :" << elapse;

#define PI 3.141593f
#define SQR(x) ((x)*(x))


// Convert image to grayscale
void grayScale(QImage &img);

// Calculate otsu threshold value
int calcOtsuThresh(QImage img);

// Apply threshold for given global threshold value
void threshold(QImage &img, int thresh);

// Apply adaptive integral threshold using bradley's method
void adaptiveThreshold(QImage &img, float T=0.15, int window_size=0);

// Gaussian Blur
void gaussianBlur(QImage &img, int radius=1, float sigma=0);

// Apply Box Blur
void boxFilter(QImage &img, int radius=1);

// Apply Median Filter (Remove salt and pepper noise)
void medianFilter(QImage &img, int radius=1);

// Sharpen by Unsharp masking
void unsharpMask(QImage &img, float factor=1.0, int thresh=5);

// remove speckle noise using crimmins speckle removal
void despeckle(QImage &img);

// Sigmoidal Contrast to enhance low light images
void sigmoidalContrast(QImage &img, float midpoint=0.5 /*0 to 1.0*/);

// Sigmoidal Contrast to enhance low light images
void stretchContrast(QImage &img);

// Gamma Encoding (apply pow(x, 1/gamma) function to each pixel)
void applyGamma(QImage &img, float gamma=1.6);

// Auto white balance
void autoWhiteBalance(QImage &img);

// Color balance using a version of Gray World Algorithm
void grayWorld(QImage &img);

// Auto white balance
void enhanceColor(QImage &img);

// Correct Lens Distortion
void lensDistortion(QImage &image, float main, float edge, float zoom);

// Vignette filter : darken edges in radial gradient
void vignette(QImage &img);

// Pencil Sketch effect
void pencilSketch(QImage &img);

QImage expandBorder(QImage img, int width);

QImage reFilter(QImage imgre, QImage img0, float mult);

#endif /* __PHOTOQUIK_FILTERS */
