# gateleg
version 1.0.0  
Currently named after a table  
An Xlib window manager written in C  

## usage
To start, add gatelegs exectuable to the end of `.xinitrc`  
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

## TODO/Bugs
 - Certain things should not reset in size when they do
 - Add switching a windows workspace
