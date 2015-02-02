cmake_minimum_required(VERSION 2.8)

project("microwm")

set(CMAKE_C_FLAGS "-W -Wall -Werror -std=c11 -pedantic -D_GNU_SOURCE")

add_executable(microwm
	main.c
	microwm.c
	widgets.c
	icccm.c
	config.c
)

find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(XLIBS x11 xft xpm REQUIRED)

include_directories(${XLIBS_INCLUDE_DIRS})
link_directories(${XLIBS_LIBRARY_DIRS})

target_link_libraries(microwm
	X11
	Xft
	Xpm
)

install(TARGETS microwm DESTINATION bin/)
install(FILES LICENSE README.md DESTINATION share/microwm)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Laurent FRANCOISE")
set(CPACK_PACKAGE_VERSION "1.2.0")
include(CPack)
