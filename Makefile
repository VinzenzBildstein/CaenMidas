.EXPORT_ALL_VARIABLES:

.SUFFIXES:

.PHONY: clean all

# := is only evaluated once

SHELL 		= /bin/sh

LIB_DIR 	= $(HOME)/lib
BIN_DIR		= $(HOME)/bin

NAME		= MidasHist

ROOTLIBS     	:= $(shell root-config --glibs)
ROOTINC      	:= -I$(shell root-config --incdir)

GRSILIBS   := $(shell grsi-config --all-libs)
GRSICFLAGS := $(shell grsi-config --cflags)

INCLUDES        = -I.

LIBRARIES	= CAENDigitizer

CC		= gcc
CXX   = g++
CPPFLAGS	= $(ROOTINC) $(GRSICFLAGS) $(INCLUDES) -fPIC
CXXFLAGS	= -pedantic -Wall -Wno-long-long -g -O3 -std=c++11 -DUSE_WAVEFORMS 
#-DUSE_CURSES

LDFLAGS		= -g -fpic

LDLIBS 		= -L$(LIB_DIR) $(ROOTLIBS) $(GRSILIBS) $(addprefix -l,$(LIBRARIES))

LOADLIBES = \
				CaenEvent.o \
				$(NAME)Dictionary.o 

# -------------------- implicit rules --------------------
# n.o made from n.c by 		$(CC) -c $(CPPFLAGS) $(CFLAGS)
# n.o made from n.cc by 	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS)
# n made from n.o by 		$(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)

# -------------------- rules --------------------

all:  $(BIN_DIR)/$(NAME)
	@echo Done

$(LIB_DIR)/lib$(NAME).so: $(LOADLIBES)
	$(CXX) $(LDFLAGS) -shared -Wl,-soname,lib$(NAME).so -o $(LIB_DIR)/lib$(NAME).so $(LOADLIBES) -lc

# -------------------- pattern rules --------------------
# this rule sets the name of the .cc file at the beginning of the line (easier to find)

%.o: %.cc %.hh
	$(CXX) $< -c $(CPPFLAGS) $(CXXFLAGS) -o $@

# -------------------- default rule for executables --------------------

$(BIN_DIR)/%: %.cc $(LOADLIBES)
	$(CXX) $< $(CXXFLAGS) $(CPPFLAGS) $(LOADLIBES) $(LDLIBS) -DHAS_XML -o $@

# -------------------- Root stuff --------------------

DEPENDENCIES = \
					CaenEvent.hh \
					RootLinkDef.h

$(NAME)Dictionary.o: $(NAME)Dictionary.cc
	 $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

$(NAME)Dictionary.cc: $(DEPENDENCIES)
	 rm -f $(NAME)Dictionary.cc $(NAME)Dictionary.h; rootcint -f $@ -c $(CPPFLAGS) $(DEPENDENCIES)

# -------------------- clean --------------------

clean:
	rm  -f $(BIN_DIR)/$(NAME) *.o
