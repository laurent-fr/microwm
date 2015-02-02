# microWM
a microscopic window manager

* written in C11
* depends only on standard X11 libs

![screenshot](https://raw.github.com/laurent-fr/microwm/master/doc/screenshot.png)

Status :
--------

Early stage of development, only a few things implemented for now ...

v0.1
* Windows decorations automatically added to windows
* Moving and resizing windows is possible.

Documentation :
---------------

See [DOxygen documentation](http://laurent-fr.github.io/microwm/)

Compile :
---------

Dependencies :

	apt-get install libx11-dev libxpm-dev libxft-dev libfreetype6-dev

Testing :

	apt-get install colorgcc
	(or change CC=gcc or CC=clang in the Makefile)
	
	make -f MAkefile.generic

Release :

	apt-get install cmake
	cmake -G "Unix Makefiles"
	make
	sudo make install


Run :
-----

Here is how to setup a quick test environnment :

	apt-get install xnest xterm

	Xnest :1
	export DISPLAY=:1

	xterm &
	xsetroot -solid SteelBlue &
	./microwm


A sample config file is available in doc/microwm.config_sample, copy it into ~/.microwm

TODO :
------

For version 0.2 :

* cleanup on exit
* raise a window by clicking anywhere
* bugfix

For version 0.3 :

* implement Iconify (a taskbar will be needed ...)

After version 0.3 :

* ICCCM compliance
* EWMH compliance

