PREFIX = /usr
CC = $(PREFIX)/bin/g++
O = 3
CFLAGS = -O$(O) -march=native -mfpmath=sse -ftree-vectorize -flto -Wall -Wextra
INC = $(PREFIX)/include $(PREFIX)/include/SDL
LIBRARIES = $(shell sdl-config --libs)
MISC_OBJ =
TARGET = MoireSDL
INSTALL = /usr/local/bin/
UNAME = $(shell uname)

ifeq ($(UNAME), Darwin)
PREFIX = /opt/local
CFLAGS += -mmacosx-version-min=10.4 -dead_strip -Wl,-no_compact_unwind
LIBDIR = $(PREFIX)/lib
LIB = libSDLmain.a libSDL.a
FW = AppKit OpenGL Cocoa ApplicationServices Carbon AudioToolbox AudioUnit IOKit
LIBRARIES = -L$(PREFIX)/lib $(foreach lib,$(LIB),$(LIBDIR)/$(lib)) -lX11 -lXext -lXrandr -lXrender -lc++ $(foreach fw,$(FW),-framework $(fw))
endif

ifneq (,$(findstring MINGW,$(UNAME))$(findstring CYGWIN,$(UNAME)))
PREFIX = /mingw
CFLAGS += -mwindows -static-libgcc -static-libstdc++ -Wl,--large-address-aware
LIBRARIES = -Wl,-Bstatic $(shell sdl-config --static-libs)
TARGET = MoireSDL.exe
MISC_OBJ += MoireSDL.res
INSTALL = /c/Windows/MoireSDL.scr
endif

.PHONY: default clean install

default: $(TARGET)

MoireSDL.res: MoireSDL.rc
	windres $< -O coff -o $@

MoireSDL MoireSDL.exe: $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(MISC_OBJ)
	$(CC) $(CFLAGS) $^ $(LIBRARIES) -o $@
	chmod ugo+rx $@

%.o: %.cpp $(wildcard *.h)
	$(CC) $(CFLAGS) -g -c $< $(foreach inc,$(INC),-I$(inc)) -o $@

clean:
	rm -rf *.o *.res $(TARGET)

install:
	cp $(TARGET) $(INSTALL)
