# microWM
a microscopic window manager

Status :
--------

Very alpha code for now ...

* Windows decorations automatically added to mapped windows
* Moving and resizing windows is possible.

Compile :
---------

(no generic Makefile for now, my dev system is a debian-like Linux)

apt-get install libx11-dev libxpm-dev libxft-dev libreetype6-dev

apt-get install colorgcc

make

Run :
-----

apt-get install xnest xterm

Xnest :1

export DISPLAY=:1

xterm&
./microwm


TODO :
------

For version 0.1 :

* cleanup on exit
* check windows hints when resizing window
* implement close/fullscren buttons
* general bugfix

For version 0.2 :

* implement Iconify (a taskbar will be needed ...)

After version 0.2 :

* ICCCM compliance
* EWMH compliance

