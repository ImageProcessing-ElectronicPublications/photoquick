# QmageView
A simple image viewer with some useful features (written in qt4).

### Description
This program is aimed at ease of use, quick opening, and doing most necessary features.  

 * Resize  
 * Crop in particular ratio  
 * Rotate  
 * Create photo grid for printing  
 * Add Border  
 * Filters  
  * Grayscale  
  * Scan Page  
  * Blur  
  * Sharpen  
  * Auto Contrast  
  * White Balance
  * Despeckle  
  * Reduce Noise  

This image viewer is tested on Raspberry Pi (Raspbian).  

### Build
To build this program, extract the source code zip.  
Open terminal and change directory to qmageview/QmageView.  
**Build dependencies ...**  
 * libqt4-dev  

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
* libqt4-svg  
* libgomp1


### Usage
To run this program...  
`qmageview`

To open image.jpg with it...  
`qmageview image.jpg`  
