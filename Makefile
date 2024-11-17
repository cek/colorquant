#
# Makefile for variance-based color quantization routine
#
# Craig Kolb 12/88
#
# The demo program, 'quantdemo', uses the Utah Raster toolkit and
# has provisions for displaying a quantized image on an Iris workstation.
#
# Location of Utah Raster include directory and library:
RLEINC = /usr/u/utah/include
RLELIB = /usr/u/utah/lib/librle.a
#
# If you've got an IRIS 4D, use the following two lines.
CFLAGS = -O -I$(RLEINC) -DIRIS -float
LIBS = $(RLELIB) -lgl_s -lm
#
# Else, use some variation on the following two lines.
#CFLAGS = -O -I$(RLEINC)
#LIBS = $(RLELIB) -lm

all: quantdemo

quantdemo:	quantdemo.o colorquant.o
		cc $(CFLAGS) -o quantdemo quantdemo.o colorquant.o $(LIBS)

clean:
	/bin/rm -f *.o
