# microWM
a microscopic window manager

![screenshot](https://raw.github.com/laurent-fr/microwm/master/doc/screenshot.png)

Status :
--------

v0.1
* Windows decorations automatically added to windows
* Moving and resizing windows is possible.

Documentation :
---------------

See [DOxygen documentation](http://laurent-fr.github.io/microwm/)

Compile :
---------

No generic Makefile for now, my dev system is a debian-like Linux.

apt-get install libx11-dev libxpm-dev libxft-dev libreetype6-dev

apt-get install colorgcc
(or change CC=gcc in the Makefile)

make

Run :
-----

Here is how to setup a quick test environnment :

apt-get install xnest xterm

Xnest :1

export DISPLAY=:1

xterm&
xsetroot -solid SteelBlue
./microwm


TODO :
------

For version 0.2 :

* cleanup on exit
* check windows hints when resizing window
* raise a window by clicking anywhere
* implement Iconify (a taskbar will be needed ...)

After version 0.2 :

* ICCCM compliance
* EWMH compliance

