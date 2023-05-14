# imgpop-miyoo
A tool for popping up an image at any given location on the screen on the MMP

Based on the say package by Shauninman - https://github.com/shauninman/MiniUI

# Changes
- Fixed aliasing
- Added scaling option

# Issues
~- Aliasing around edges~
- Flickers on main menu due to MainUI redraw

# Usage 
`./imgpop 5 "/mnt/SDCARD/Themes/Silky by DiMo/skin/miyoo-topbar.png" 220 420 > /dev/null 2>&1 &`

`Syntax: ./imgpop duration image_path x_position y_position resize_percentage`

![MainUI_019](https://github.com/XK9274/imgpop-miyoo/assets/47260768/05ced503-caef-47b3-860c-4b9c462be61a)


