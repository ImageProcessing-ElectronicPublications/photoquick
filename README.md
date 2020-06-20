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
Open terminal  
Then run these commands to compile...  
```
qmake src  
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
add Qt/4.8.7/bin directory and mingw32/bin directory in PATH environment variable.  
Open Command Line.  
Run command...  
```
qmake src  
make -j4  
```

You can download the precompiled windows exe package in the [release page](https://github.com/ksharindam/photoquick/releases).  

### Usage
To run this program...  
`photoquick`  

To open image.jpg with it...  
`photoquick image.jpg`  

### Keyboard Shortcuts
Reload Image : R  
Delete Image : Delete  

