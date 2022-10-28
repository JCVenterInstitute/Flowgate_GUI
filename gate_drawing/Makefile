#!/bin/env make
#
# @file
# Builds the FlowGate command-line tools.
#
# The FlowGate tools include C++ APIs and command-line tools to read
# and write FlowCytometry and Gating data in a variety of file formats.
# The tools also support plotting a file data, with or without gating,
# and computing clusters using gate trees.
#
# This software was developed for the J. Craig Venter Institute (JCVI)
# in partnership with the San Diego Supercomputer Center (SDSC) at the
# University of California at San Diego (UCSD).

#-----------------------------------------------------------------------
#
# Compiler configuration.
#
#-----------------------------------------------------------------------
# The name of the C++ compiler to use.
CC :=		g++


# Language features:
#	-std=c++17	Use C++17.
#	-fopenmp	Enable OpenMP language extensions.
#
# Debugging flags:
#	-g		Include debugging info.
#
# Optimization flags:
#	-O3		Optimize at the high level.
#	-m64		Compile for a 64-bit architecture.
#	-march=native	Generate code best for the current processor.
#	-ffast-math	Use faster (lower-precision) math.
#
# Reporting flags:
#	-Wall		Issue all possible warnings.
#
# Libraries:
# 	-lpthread	P-threads (used by OpenMP) library.
# 	-lgd		Graphics drawing library.
#
# Build binaries for debugging:
#CFLAGS :=	-std=c++17 -fopenmp -g -Wall
#LDFLAGS  :=	-std=c++17 -fopenmp -g

# Build binaries that work with multiple processor variants:
#CFLAGS :=	-std=c++17 -fopenmp -O3 -m64 -ffast-math -Wall
#LDFLAGS :=	-std=c++17 -fopenmp -O3 -m64 -ffast-math

# Best choice:
CFLAGS :=	-std=c++17 -fopenmp -O3 -m64 -march=native -ffast-math -Wall
LDFLAGS :=	-std=c++17 -fopenmp -O3 -m64 -march=native -ffast-math

# GASON JSON parser:
# The default stack size is just 32 frames, which is not sufficient for
# deep gate trees. The default zone size is 4096 bytes, which is also
# not sufficient for large gate trees. Define bigger sizes which we use
# below to patch the source.
JSON_STACK_SIZE	:=	256
JSON_ZONE_SIZE :=	32768
GASON_H :=		gason.h
GASON_CPP :=		gason.cpp
GASON_BIG_CPP :=	gason_big.cpp

DEFINES +=	
INCLUDES +=	-I. -I/opt/local/include -I/usr/include

# OS-vendor libraries are usually in /lib and /usr/lib. Third-party
# libraries are often in /usr/local/lib or /opt/local/lib.
LIBRARIES :=	-L/usr/local/lib -L/opt/local/lib -lpthread -lgd

CFLAGS +=	$(CFLAGS) $(DEFINES) $(INCLUDES)
LDFLAGS +=	$(GASON_BIG_CPP) $(LDFLAGS) $(LIBRARIES)
CPATH =		/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include

# Export CPATH, which is needed on MacOS.
.EXPORT_ALL_VARIABLES:

# .cpp compilation rule. Always builds an application
%:	%.cpp
	-@echo "  $<$(ERRORCOLOR)"
	-@$(CC) $(CFLAGS) $(INCLUDES) $< $(LDFLAGS) -o $*
	-@/bin/echo -n "$(NORMALCOLOR)"





#-----------------------------------------------------------------------
#
# Misc.
#
#-----------------------------------------------------------------------
UNDERLINE :=	"----------------------------------------------------------------------"
NORMALCOLOR :=	"\[0m"
TITLECOLOR :=	"\[1\;33m"
ERRORCOLOR :=	"\[1\;31m"





#-----------------------------------------------------------------------
#
# Source files.
#
#-----------------------------------------------------------------------
EVENT_FORMATS :=	FileFCS.h FileFGBinaryEvents.h FileFGTextEvents.h
GATING_CACHE_FORMATS :=	FileFGGatingCache.h
GATE_FORMATS :=		FileFGTextGates.h FileFGJsonGates.h FileGatingML.h rapidxml.hpp $(GASON_H) $(GASON_BIG_CPP)

EVENT_MODEL :=		EventTable.h
GATING_CACHE_MODEL :=	GatingCache.h
GATE_MODEL :=		GateTrees.h
PARAMETER_MAP_MODEL :=	ParameterMap.h

GATING :=		Gating.h
PLOT_UTIL :=		ScatterPlot.h





#-----------------------------------------------------------------------
#
# Targets.
#
#-----------------------------------------------------------------------
.PHONY:	help all clean size build

help:
	-@echo "$(TITLECOLOR)Targets$(NORMALCOLOR)"
	-@echo "$(TITLECOLOR)$(UNDERLINE)$(NORMALCOLOR)"
	-@echo "  all              Build everything"
	-@echo "  clean            Delete built files"
	-@echo "  size             Report on the lines of code"
	-@echo
	-@echo "  fgconvert        Build file format converter"
	-@echo "  fginfo           Build file information viewer"
	-@echo "  fggateplot       Build gater and gate plotter"
	-@echo "  fgplot           Build file plotter"

all:	fgconvert fginfo fggateplot fgplot

clean:
	-@echo "$(TITLECOLOR)Clean$(NORMALCOLOR)"
	-@echo "$(TITLECOLOR)$(UNDERLINE)$(NORMALCOLOR)"
	-@echo "Removing old build files..."
	-@rm -rf *.dSYM *.gcda *.gcno *.gch *.o *.ii *.s $(GASON_BIG_CPP)
	-@echo "Removing old compiled tools..."
	-@rm -rf fginfo fgconvert fgplot fggateplot

size:
	-@echo "$(TITLECOLOR)Code size$(NORMALCOLOR)"
	-@echo "$(TITLECOLOR)$(UNDERLINE)$(NORMALCOLOR)"
	-@wc -l *.h *.cpp

$(GASON_BIG_CPP):	$(GASON_CPP)
	-@echo "  Patch $(GASON_BIG_CPP)"
	-@cat $(GASON_CPP) | \
		sed 's/^#define JSON_STACK_SIZE.*$$/#define JSON_STACK_SIZE $(JSON_STACK_SIZE)/' | \
		sed 's/^#define JSON_ZONE_SIZE.*$$/#define JSON_ZONE_SIZE $(JSON_ZONE_SIZE)/' > $(GASON_BIG_CPP)

fgconvert:	fgconvert.cpp \
		$(EVENT_MODEL) $(GATE_MODEL) $(PARAMETER_MAP_MODEL) $(GATING_CACHE_MODEL) \
		$(EVENT_FORMATS) $(GATING_CACHE_FORMATS) $(GATE_FORMATS)

fginfo:		fginfo.cpp \
		$(EVENT_MODEL) $(GATE_MODEL) $(PARAMETER_MAP_MODEL) $(GATING_CACHE_MODEL) \
		$(EVENT_FORMATS) $(GATING_CACHE_FORMATS) $(GATE_FORMATS)

fggateplot:	fggateplot.cpp \
		$(PLOT_UTIL) $(GATING) \
		$(EVENT_MODEL) $(GATE_MODEL) $(PARAMETER_MAP_MODEL) $(GATING_CACHE_MODEL) \
		$(EVENT_FORMATS) $(GATING_CACHE_FORMATS) $(GATE_FORMATS)

fgplot:		fgplot.cpp \
		$(EVENT_MODEL) $(PARAMETER_MAP_MODEL) $(PLOT_UTIL) \
		$(EVENT_FORMATS)
