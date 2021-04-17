PSPSDK = $(shell psp-config --pspsdk-path)
PSPLIBSDIR = $(PSPSDK)/..
TARGET = MacFixer
OBJS = main.o IdStorage.o
LIBS = -lpsputility

BUILD_PRX=1

EXTRA_TARGETS = EBOOT.PBP

LIBS = -lpspwlan -lpspnet_adhoc -lpspnet_adhocctl
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

include $(PSPSDK)/lib/build.mak
