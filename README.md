# BlockIO
A library that allows you to perfrom block IO on posix systems. Api is catered to only allowing you to read and write blocks on data randomly. You can specify flags that you like.

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
   * `#include<block_io.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd BlockIO`
 * `sudo make uninstall`
