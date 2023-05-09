# imgpop-miyoo
A tool for popping up an image at any given location on the screen on the MMP

# Issues
Aliasing around edges
Flickers due to MainUI redraw

# Usage 
./imgpop 5 /mnt/SDCARD/miyoo/app/skin/icon-wifi-signal-04.png 290 420 > /dev/null 2>&1

Syntax: ./imgpop duration image_path x_position y_position