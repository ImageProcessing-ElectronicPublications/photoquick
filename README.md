![GitHub release (latest by date)](https://img.shields.io/github/v/release/ImageProcessing-ElectronicPublications/photoquick)
![GitHub Release Date](https://img.shields.io/github/release-date/ImageProcessing-ElectronicPublications/photoquick)
![GitHub repo size](https://img.shields.io/github/repo-size/ImageProcessing-ElectronicPublications/photoquick)
![GitHub all releases](https://img.shields.io/github/downloads/ImageProcessing-ElectronicPublications/photoquick/total)
![GitHub](https://img.shields.io/github/license/ImageProcessing-ElectronicPublications/photoquick)

# PhotoQuick (for Linux and Windows)
A simple handy image viewer and editor with some useful features (written in qt4).

### Description
This program is aimed at ease of use, quick opening, and doing most necessary features.  

 * Export to PDF
 * Auto Resize to file size  
 * Crop in particular ratio  
 * Rotate, mirror, perspective transform  
 * Add Border  
 * Create photo grid for printing  
 * Magic Eraser (inpainting)  
 * Intelligent Scissor  
 * Filters  
 * Scan Page  
 * Auto Contrast  
 * White Balance  
 * Reduce Noise  


This image viewer is tested on Raspberry Pi (Raspbian).  

### Build (Linux)
Install dependencies...  
**Build dependencies ...**  
 * libqt4-dev  

To build this program, extract the source code zip.  
Open terminal and change directory to src/  
Then run these commands to compile...  
```
qmake  
make -j4  
```

To install run ...  
`sudo make install`  

To uninstall, run ...  
`sudo make uninstall`  

**Runtime Dependencies**  
* libqtcore4  
* libqtgui4  
* libqt4-svg  (optional for svg support)  
* libgomp1

### Build (Windows)
Download Qt 4.8.7 and minGW32  
Add Qt/4.8.7/bin directory and mingw32/bin directory in PATH environment variable.  
In src directory open Command Line.  
Run command...  
`qmake`  
`make -j4`  

You can download the precompiled windows exe package in the [release page](https://github.com/ksharindam/photoquick/releases).  

### Plugins
This program supports plugins. You can get a set of plugins
[here](https://github.com/ImageProcessing-ElectronicPublications/photoquick-plugins).   Also you can build your own plugins and use with it.  

### Usage
To run this program...  
`photoquick`  

To open image.jpg with it...  
`photoquick image.jpg`  

### Keyboard Shortcuts
Reload Image : R  
Delete Image : Delete  

### Supported Image Formats
All formats supported by Qt are supported in this program.  
**Read :** JPG, PNG, GIF, SVG, TIFF, ICO, BMP, XPM, XBM, PPM, PBM, PGM  
**Write :** JPG, PNG, TIFF, ICO, BMP, XPM, XBM, PPM  

JPEG2000 and WebP formats are supported via image format plugins.  
You can get these here...  
https://github.com/ksharindam/qt4-imageformat-plugins  

### Screenshots

Main Window  
![Main Window](data/screenshots/Screenshot1.jpg)  

Photo-Grid  
![Photo Grid](data/screenshots/Screenshot2.jpg)  

Scissor Tool  
![Scissor Tool](data/screenshots/Screenshot3.jpg)  

