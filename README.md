# gateleg
version 1.0.0  
table-style tiling window manager.
An Xlib window manager written in C  

## usage
build is simple, but requires xlib to be installed:  
```
gcc *.c -o gateleg  -lX11
sudo ln gateleg /usr/bin/gateleg
```
Add `exec gateleg` to the end of `.xinitrc` without any other wm in running in the file and use startx to start it.
This is a non-compositing window manager, so use something like picom in-order to get those features  
```
Cmd-Shft-E: Exit
Cmd-N: New terminal (compile time configuration, default lxterminal because I use it)
M-Tab: Switch to next window clockwise
Cmd-#: Switch workspace
Cmd-J: Close window
Cmd-(Arrow Key): Move window in direction
Cmd-F: Fullscreen window
Cmd-V: Vertical window
Cmd-H: Horizontal window
Cmd-G: Reset size of window
Cmd-R: Reset tiling
```
I say the tiling is "table-style" as it has no partition beyond the initial four quadrants currently.

## TODO/Bugs
 - Certain things should not reset in size when they do
 - Add switching a windows workspace
