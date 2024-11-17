/*
 * This software is copyrighted as noted below.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is 
 * preserved on all copies.
 * 
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the author, who may or may not act on them as he desires.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the 
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 */
/*
 * quantdemo.c
 *
 * Demo program for variance-based color quantization routine.
 * Reads a Utah Raster format file and quantizes the image.
 * If compiled with IRIS is defined, code is included to display the
 * quantized image on an Iris 4D workstation.
 *
 * Author:	Craig Kolb
 *		Dept. of Mathematics
 *		Yale University
 *		kolb@yale.edu
 * Date: Wed Feb 8 1989
 * Copyright (C) 1989, Craig E. Kolb
 */
#include <stdio.h>
#include <svfb_global.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>

/*
 * DISCLAIMER:
 * This code is sloppy.
 */
#define DEFAULT_BITS		5
#define DEFAULT_ENTRIES		256
#define DEFAULT_FAST		1

#ifdef IRIS
#include <device.h>
#define MAP_OFFSET	512	/* Offset into Iris colormap */
unsigned short *scanline;
#endif

#define Quantize(x)	(x >> shift)

unsigned char *RGBmap, *Pixels[3];	/* RGB-->index map & original image */
char *Progname;				/* argv[0] */
int Xres, Yres, Bits;			/* Image resolution, # of sig. bits */
int Verbose;				/* Verbosity flag. */

main(argc, argv)
int argc;
char **argv;
{
	unsigned char *rp, *gp, *bp, *inline[3], *colormap[3];
	unsigned ByteCount;
	int x, y, colors, Entries, shift, Fast;
	struct tms time;


#ifdef IRIS
	short val;
#endif
	/*
	 * Set default values.
	 */
	Bits = DEFAULT_BITS;
	Entries = DEFAULT_ENTRIES;
	Fast = DEFAULT_FAST;

	Progname = argv[0];

	argv++;
	argc--;

	while (argc) {
		if (argv[0][0] != '-')
			break;
		switch(argv[0][1]) {
			case 'b':
				Bits = atoi(argv[1]);
				argc--;
				argv++;
				break;
			case 'c':
				Entries = atoi(argv[1]);
				argc--;
				argv++;
				break;
			case 'f':
				Fast = !Fast;
				break;
			case 'h':
				usage();
				exit(3);
			case 'v':
				Verbose = !Verbose;
				break;
			default:
				fprintf(stderr,"Unknown option: %s\n",argv[0]);
				usage();
				exit(1);
		}
		argv++;
		argc--;
	}

	if (argc) {
		sv_globals.svfb_fd = fopen(argv[0], "r");
		if (sv_globals.svfb_fd == (FILE *)NULL) {
			fprintf(stderr,"Cannot open %s.\n",argv[0]);
			exit(0);
		}
	} else
		sv_globals.svfb_fd = stdin;

	/*
	 * Read Utah Raster header.
	 */
	if (rle_get_setup(&sv_globals) < 0) {
		fprintf(stderr,"Error reading RLE header.\n");
		exit(1);
	}

	/*
	 * Ignore ALPHA channel, if any.
	 */
	SV_CLR_BIT(sv_globals, SV_ALPHA);
	Yres = sv_globals.sv_ymax - sv_globals.sv_ymin + 1;
	Xres = sv_globals.sv_xmax - sv_globals.sv_xmin + 1;
	sv_globals.sv_xmax -= sv_globals.sv_xmin;
	sv_globals.sv_xmin = 0;

	/*
	 * ByteCount gives the size off the RGBmap, and is equal to
	 * (2^Bits)^3
	 */
	ByteCount = 1 << Bits;
	ByteCount *= ByteCount * ByteCount;
	RGBmap = (unsigned char *)malloc(ByteCount*sizeof(unsigned char));

	Pixels[0]= (unsigned char *)malloc(Xres * Yres * sizeof(unsigned char));
	Pixels[1]= (unsigned char *)malloc(Xres * Yres * sizeof(unsigned char));
	Pixels[2]= (unsigned char *)malloc(Xres * Yres * sizeof(unsigned char));

	rp = Pixels[0];
	gp = Pixels[1];
	bp = Pixels[2];

	inline[0] = (unsigned char *)malloc(Xres * sizeof(unsigned char));
	inline[1] = (unsigned char *)malloc(Xres * sizeof(unsigned char));
	inline[2] = (unsigned char *)malloc(Xres * sizeof(unsigned char));

	shift = 8 - Bits;
	for (y = 0; y < Yres; y++) {
		rle_getrow(&sv_globals, inline);
		for (x = 0; x < Xres; x++) {
			/*
			 * Quantize input values.
			 */
			*rp++ = Quantize(inline[0][x]);
			*gp++ = Quantize(inline[1][x]);
			*bp++ = Quantize(inline[2][x]);
		}
	}

	free((char *)inline[0]);
	free((char *)inline[1]);
	free((char *)inline[2]);

	colormap[0] = (unsigned char *)malloc(Entries * sizeof(unsigned char));
	colormap[1] = (unsigned char *)malloc(Entries * sizeof(unsigned char));
	colormap[2] = (unsigned char *)malloc(Entries * sizeof(unsigned char));

	if (Verbose) {
		times(&time);
		printf("Preprocessing time: %.2fu  %.2fs\n",
			(float)time.tms_utime / (float)HZ,
			(float)time.tms_stime / (float)HZ);
	}

	colors = colorquant(Pixels[0], Pixels[1], Pixels[2],
				(unsigned long)(Xres*Yres),
				colormap, Entries, Bits, RGBmap, Fast);
	if (Verbose) {
		times(&time);
		printf("Total time: %.2fu  %.2fs\n",
			(float)time.tms_utime / (float)HZ,
			(float)time.tms_stime / (float)HZ);
		fprintf(stderr,"Quantized to %d colors.\n",colors);
	}

#ifdef IRIS
	prefsize(Xres, Yres);
	winopen("QuantDemo");
	unqdevice(INPUTCHANGE);
	qdevice(REDRAW);
	qdevice(ESCKEY);
	qdevice(WINQUIT);
	/*
	 * Load hardware colormap with computed values.
	 */
	for (x = 0; x < colors; x++)
		mapcolor(x+MAP_OFFSET, (short)(colormap[0][x]),
				(short)(colormap[1][x]),
				(short)(colormap[2][x]));
	scanline = (unsigned short *)malloc(Xres * sizeof(unsigned short));

	drawimage();

	for (;;) {
		switch(qread(&val)) {
			case WINQUIT:
			case ESCKEY:
				gexit(0);
				exit(0);
				break;
			case REDRAW:
				reshapeviewport();
				drawimage();
				break;
		}
	}
#else
	exit(0);
#endif
}

#ifdef IRIS
/*
 * Display quantized image.
 */
drawimage()
{
	int x, y;
	unsigned char *rp, *gp, *bp;

	rp = Pixels[0];
	gp = Pixels[1];
	bp = Pixels[2];

	for (y = 0; y < Yres; y++) {
		for (x = 0; x < Xres; x++, rp++, gp++, bp++)
			scanline[x] = MAP_OFFSET + (unsigned short)
				RGBmap[(((*rp<<Bits)|*gp)<<Bits)|*bp];
		cmov2i(0, y);
		writepixels(Xres, scanline);
	}
}
#endif
usage()
{
	fprintf(stderr,"usage: quantdemo [-c colors] [-b bits] [-f] [filename]\n");
}
