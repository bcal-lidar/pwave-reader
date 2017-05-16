# pwave-reader
Access to pulsewaves files using PulseWaves Library

The PulseWaves library is required.  An updated version with minor bug fixes
and CMake is at http://github.com/ksshannon/PulseWaves in the cmake branch.

To fetch and build libpulsewaves and pwave-reader:

     $ export REPODIR=/path/to/src/
     $ mkdir -p $REPODIR
     $ cd $REPODIR
     $ git clone https://github.com/ksshannon/PulseWaves
     $ cd PulseWaves
     $ git checkout cmake
     $ mkdir build && cd build
     $ cmake ..
     $ make
     $ cd $REPODIR
     $ git clone https://github.com/bsurc/pwave-reader
     $ cd pwave-reader
     $ mkdir build && cd build
     $ cmake ../ -DPULSEWAVES\_INC=$REPODIR/PulseWaves/inc -DPULSEWAVES\_LIB=$REPODIR/PulseWaves/build/src/libpulsewaves.a
     $ make
     $ ./src/pw --help
