# FantomBitmap 512 Colour Image Viewer for Sega Mega CD

## Description

This project shows an example of how the Sega Mega CD can be used to view 512 colour images based on FantomBitmap.  The code includes the additional ability to view LZ4W compressed DCI3 images (under 64KiB).

To convert images under VS Code in Terminal Prompt by using the command:  ./directconvert -i3 input output 

Example: ./directconvert -i3 Sonic.png Sonic.dci3

Drop the files into the image folder

To build this project use: 

build clean

build cd

## Features
- Fantom Bitmap 512 colour

## "FantomBitmap 512 Colour Image Viewer for Sega Mega CD" Credits
* Programming : Chilly Willy
* Additional Code for UI & LZ4W compression : Matt B (Matteusbeus)
* Base Docker Code : Victor Luchits
* Additional Docker Code for compatibility : Matt B (Matteusbeus)
* Support : Barone

## "FantomBitmap" Credits
* Programming : Oerg886

## "LZ4W compression" Credits
* Programming : Stephane Dallongeville

## 

## Links
http://gendev.spritesmind.net/forum/viewtopic.php?f=22&t=1166

https://github.com/Stephane-D/SGDK/blob/master/bin/lz4w.txt

https://github.com/viciious/d32xr/tree/master/.devcontainer

## License
All original code is available under the MIT license.
