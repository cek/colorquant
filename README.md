# Variance-Based Color Quantization #
This repo contains an implementation of the quantization algorithm described in:

Wan, Wong, and Prusinkiewicz,
*An Algorithm for Multidimensional Data Clustering*
Transactions on Mathematical Software, Vol. 14 #2 (June, 1988), pp. 153-162.

This code was incorporated into the [Utah Raster Toolkit](https://www2.cs.utah.edu/gdc/projects/urt/index.html) and other
image processing packages. It is perserved here for historical disinterest.

## Original Notes ##

The colorquant() routine has been tested on an Iris 4D70 workstation,
a Sun 3/60 workstation, and (to some extent), a Macintosh.

Calls to bzero() may have to be replaced with the appropriate thing on
your system.  (bzero(ptr, len) writes 'len' 0-bytes starting at the location
pointed to by ptr.)

Although I've tried to avoid integer overflow problems where ever possible,
it's likely I've missed a spot where an 'int' should really be a 'long'.
(On the machine this was developed on, an int == long == 32 bits.)

Note that it's quite easy to optimize this code for a given value for
'bits'.  In addition, if you assume bits is small and
that the total number of pixels is relatively small, there are several
places that integer arithmetic may be substituted for floating-point.
One such place is the loop in BoxStats -- mean and var need not necessarily
be floats.

As things stand, the maximum number of colors to which an image may
be quantized is 256.  This limit may be overcome by changing rgbmap and
colormap from arrays of characters to arrays of something larger.

See 'quantdemo.c' for an example of how colorquant() is used.

Craig Kolb 8/25/89
<br>
kolb@yale.edu
