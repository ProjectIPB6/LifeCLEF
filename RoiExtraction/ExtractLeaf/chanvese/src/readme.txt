Chan-Vese Segmentation
Pascal Getreuer, getreuer@cmla.ens-cachan.fr, CMLA, ENS Cachan
Version 20130706 (July 6, 2013)

  *** Please cite IPOL article "Chan-Vese Segmentation" if you ***
  *** publish results obtained with this software.             ***
  

== Overview ==

This C source code is a revision of the code accompanying Image Processing On
Line (IPOL) article "Chan-Vese Segmentation" at

  http://dx.doi.org/10.5201/ipol.2012.g-cv

The original peer-reviewed IPOL version of the code is available from the
article page.

Future software releases and updates will be posted at

  http://dev.ipol.im/~getreuer/code

Compared to the IPOL version, this revision differs in the following ways:

    * Included zlib.h in imageio.c for compatibility with libpng 1.5 and later


== License (BSD) ==

All files are distributed according to the simplified BSD license.  You
should have received a copy of this license along this program.  If not, see
<http://www.opensource.org/licenses/bsd-license.html>.


== Program Usage ==

This source code produces a command line program chanvese, which performs
Chan-Vese active contours segmentation.

Usage: chanvese [param:value ...] input animation final

where "animation" is a GIF file and "input" and "final" are BMP files (JPEG,
PNG, or TIFF files can also be used if the program is compiled with libjpeg,
libpng, and/or libtiff).

Parameters:

   mu:<number>           length penalty (default 0.25)
   nu:<number>           area penalty (default 0.0)
   lambda1:<number>      fit weight inside the cuve (default 1.0)
   lambda2:<number>      fit weight outside the curve (default 1.0)
   phi0:<file>           read initial level set from an image or text file
   tol:<number>          convergence tolerance (default 1e-4)
   maxiter:<number>      maximum number of iterations (default 500)
   dt:<number>           time step (default 0.5)

   iterperframe:<number> iterations per frame (default 10)

Example (performed by the BASH script example.sh):

    # Perform Chan-Vese segmentation on wrech.bmp with edge penalty mu = 0.2.
    # The output animation.gif shows an animation of the curve evolution, and
    # final.bmp shows the final segmentation.
    ./chanvese mu:0.2 wrench.bmp animation.gif final.bmp

The chanvese program prints detailed usage information when executed
without arguments or "--help".


== Compiling ==

Instructions are included below for compiling on Linux sytems with GCC, on
Windows with MinGW+MSYS, and on Windows with MSVC.


== Compiling (Linux) ==

To compile this software under Linux, first install the development files for
libjpeg, libpng, and libtiff.  On Ubuntu and other Debian-based systems, enter
the following into a terminal:
    sudo apt-get install build-essential libjpeg8-dev libpng-dev libtiff-dev
On Redhat, Fedora, and CentOS, use
    sudo yum install make gcc libjpeg-turbo-devel libpng-devel libtiff-devel

Then to compile the software, use make with makefile.gcc:

    tar -xf chanvese_20130706.tgz
    cd chanvese_20130706
    make -f makefile.gcc

This should produce the chanvese executable.

Source documentation can be generated with Doxygen (www.doxygen.org).

    make -f makefile.gcc srcdoc


== Compiling (Windows with MinGW+MSYS) ==

The MinGW+MSYS is a convenient toolchain for Linux-like development under
Windows.  MinGW and MSYS can be obtained from

    http://downloads.sourceforge.net/mingw/


--- Building with BMP only ---

The simplest way to build the chanvese program is with support for only BMP
images. In this case, no external libraries are required.  Edit makefile.gcc
and comment the LDLIB lines to disable use of libjpeg, libpng, and libtiff:

    #LDLIBJPEG=-ljpeg
    #LDLIBPNG=-lpng -lz
    #LDLIBTIFF=-ltiff

Then open an MSYS terminal and compile the program with

    make CC=gcc -f makefile.gcc

This should produce the chanvese executable.


--- Building with PNG, JPEG, and/or TIFF support ---

To use the chanvese program with PNG, JPEG, and/or TIFF images, the
following libraries are needed.

    For PNG:    libpng and zlib
    For JPEG:   libjpeg
    For TIFF:   libtiff

These libraries can be obtained at
    
    http://www.libpng.org/pub/png/libpng.html
    http://www.zlib.net/
    http://www.ijg.org/
    http://www.remotesensing.org/libtiff/

It is not necessary to include support for all of these libraries, for
example, you may choose to support only PNG by building zlib and libpng
and commenting the LDLIBJPEG and LDLIBTIF lines in makefile.gcc.

Instructions for how to build the libraries with MinGW+MSYS are provided at

    http://permalink.gmane.org/gmane.comp.graphics.panotools.devel/103
    http://www.gaia-gis.it/spatialite-2.4.0/mingw_how_to.html

Once the libraries are installed, build the chanvese program with the
makefile.gcc included in this archive.

    make CC=gcc -f makefile.gcc

This should produce the chanvese executable.


== Compiling (Windows with MSVC) ==

The express version of the Microsoft Visual C++ (MSVC) compiler can be
obtained for free at

    http://www.microsoft.com/visualstudio/en-us/products/2010-editions/express


--- Building with BMP only ---

For simplicity, the makefile will build the program with only BMP image
support by default.  Open a Visual Studio Command Prompt (under Start Menu >
Programs > Microsoft Visual Studio > Visual Studio Tools > Visual Studio
Command Prompt), navigate to the folder containing the sources, and enter

    nmake -f makefile.vc all

This should produce the chanvese executable.


--- Building with PNG and/or JPEG support ---

To include support for PNG and/or JPEG images, the libpng and libjpeg
libraries are needed.  Edit the LIB lines at the top of makefile.vc to
tell where each library is installed, e.g.,

    LIBJPEG_DIR     = "C:/libs/jpeg-8b"
    LIBJPEG_INCLUDE = -I$(LIBJPEG_DIR)
    LIBJPEG_LIB     = $(LIBJPEG_DIR)/libjpeg.lib

Then compile using

    nmake -f makefile.vc all


== Compiling (Mac OSX) ==

The following instructions are untested and may require adaptation, but
hopefully they provide something in the right direction.

First, install the XCode developer tools.  One way to do this is from
the OSX install disc, in which there is a folder of optional installs
including a package for XCode.


--- Building with BMP only ---

For simplicity, it is possible to build the chanvese program with only
BMP image support so that no external libraries are needed.  Edit
makefile.gcc and comment the LDLIB lines to disable use of libjpeg,
libpng, and libtiff:

    #LDLIBJPEG=-ljpeg
    #LDLIBPNG=-lpng -lz
    #LDLIBTIFF=-ltiff

Open the Console from the Utilities folder.  Use the "cd" command to
navigate to the chanvese sources folder, and compile the program with

    make -f makefile.gcc

This should produce the chanvese executable.


--- Building with PNG, JPEG, and/or TIFF support ---

The program can optionally use the libpng, libjpeg, and libtiff libraries
to support more image formats.  These libraries can be obtained on Mac
OSX from the Fink project

    http://www.finkproject.org/

Go to the Download -> Quick Start page for instructions on how to get
started with Fink.  The Fink Commander program may then be used to
download and install the packages libpng, libjpeg, and libtiff.  It may
be necessary to install libpng-dev, libjpeg-dev, and libtiff-dev as well.

Once the libraries are installed, compile the program using the included
makefile "makefile.gcc":

    make -f makefile.gcc

This should produce the chanvese executable.


== Acknowledgements ==

This material is based upon work supported by the National Science
Foundation under Award No. DMS-1004694.  Any opinions, findings, and
conclusions or recommendations expressed in this material are those of
the author(s) and do not necessarily reflect the views of the National
Science Foundation.
