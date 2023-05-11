ifeq (,$(CROSS_COMPILE))
$(error missing CROSS_COMPILE for this toolchain)
endif

TARGET = imgpop
CC = $(CROSS_COMPILE)gcc 
CFLAGS   = -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve
LDFLAGS  = -Wl,-rpath-link=/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/lib
LIBS     = -lSDL -lSDL_image -lSDL_ttf -lSDL_gfx

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) main.c $(LIBS)

clean:
	rm -f $(TARGET)