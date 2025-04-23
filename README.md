# BlockIO
A library that allows you to perform synchronous blocking, block IO on posix systems. Api is catered to only allowing you to read and write consecutive blocks of data blockingly to a persistent disk.

You can specify only selected few additional flags that you may need like, O_DIRECT, O_DSYNC or O_SYNC. You can also truncate a file while opening it using O_TRUNC flags.

It works with large file support. i.e. off_t is 64 bits wide allowing you to access large files.

## Setup instructions
**Install dependencies :**
 * [Cutlery](https://github.com/RohanVDvivedi/Cutlery)

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/BlockIO.git`

**Build from source :**
 * `cd BlockIO`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-lblockio -lcutlery` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<blockio/block_io.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd BlockIO`
 * `sudo make uninstall`
