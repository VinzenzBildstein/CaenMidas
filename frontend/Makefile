# Makefile
# $Id$

LIBS = -lm -lz -lutil -lpthread -lCAENDigitizer -lrt

DRV_DIR         = $(MIDASSYS)/drivers
INC_DIR         = $(MIDASSYS)/include
LIB_DIR         = $(MIDASSYS)/lib

# MIDAS library
MIDASLIBS = $(LIB_DIR)/libmidas.a

# fix these for MacOS
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
MIDASLIBS = $(MIDASSYS)/darwin/lib/libmidas.a
LIB_DIR   = $(MIDASSYS)/darwin/lib
endif

ROOTFLAGS=$(shell $(ROOTSYS)/bin/root-config --cflags)

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -std=c++11 -g -O2 -Wall -Wuninitialized -I$(INC_DIR) -I$(DRV_DIR) -I$(VMICHOME)/include
CXXFLAGS = $(CFLAGS) -DHAVE_ROOT -DUSE_ROOT $(ROOTFLAGS)

# ROOT library
ROOTLIBS = $(shell $(ROOTSYS)/bin/root-config --libs) -lThread -Wl,-rpath,$(ROOTSYS)/lib

all: fecaen WriteToOdb

fecaen: $(MIDASLIBS) $(LIB_DIR)/mfe.o fecaen.o CaenSettings.o CaenDigitizer.o
	$(CXX) -o $@ $(CXXFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(ROOTLIBS) $(LIBS)

%: %.cc $(MIDASLIBS) CaenSettings.o
	$(CXX) -o $@ $(CXXFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(ROOTLIBS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

clean::
	-rm -f *.o *.exe

# end
