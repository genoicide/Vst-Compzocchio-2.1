CC = gcc -g -pipe -D_REENTRANT

VST = ../../vstsdk2.4
SDK = $(VST)/public.sdk/source/vst2.x
GUI = $(VST)/vstgui_350/vstgui

FFLAGS = -fmessage-length=0 -finline-functions -fvisibility=hidden -fPIC -fno-exceptions -fpascal-strings -ffast-math
# -fno-rtti 
MFLAGS =
DEFINES = -D__mac__ -DUSE_LIBPNG -DTARGET_API_MAC_CARBON
#CFLAGS = -Wall -O3 -s -arch ppc -arch i386 -arch x86_64 -msse2 -c $(MFLAGS) $(FFLAGS) -I$(VST) -I$(SDK) -I$(GUI) $(DEFINES)
#LFLAGS = -Wall -bundle -arch ppc -arch i386 -arch x86_64 -framework Carbon -lstdc++ -pthread
CFLAGS = -Wall -O3 -s -arch ppc -arch i386 -msse2 -c $(MFLAGS) $(FFLAGS) -I$(VST) -I$(SDK) -I$(GUI) $(DEFINES)
LFLAGS = -Wall -bundle -arch ppc -arch i386 -framework Carbon -lstdc++ -pthread

SRCS=	../molotgui.cpp \
	../molotvst.cpp \
	../vstparam.cpp \
	$(SDK)/audioeffect.cpp \
	$(SDK)/audioeffectx.cpp \
	$(SDK)/vstplugmain.cpp \
	$(GUI)/aeffguieditor.cpp \
	$(GUI)/vstcontrols.cpp \
	$(GUI)/vstgui.cpp

OBJS= ${SRCS:.cpp=.o}

.SUFFIXES : .o .cpp
.cpp.o :
	$(CC) $(CFLAGS) -c $< -o $*.o


all:	Molot

Molot:	${OBJS}
	$(CC) $(LFLAGS) -o $@ $^
	
clean:
	rm -f ${OBJS} Molot
