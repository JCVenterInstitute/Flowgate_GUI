FlowGate Tools Source
------------------------------------------------------------------------
This directory contains the source code to FlowGate's command-line tools
for loading files in various formats, converting them, plotting, gating,
and computing clusters.


Building the tools
------------------
The included Makefile builds the tools. To get a list of targets type:

	make help

To build all of the tools type:

	make all

Compilation requires a C++17 compiler, such as GCC 8 or later.


Dependencies
------------
Plotting requires the "libgd" library, which is open source and available
for all common OSes. The library has its own dependencies, which typically
include "freetype", "fontconfig", "jpeg", "libiconv", "libpng", "tiff",
"webp", "zlib", and "xpm".

XML parsing requires the "RapidXML" package, which is included here.

JSON parsing requires the "Gason" package, which is included here.
