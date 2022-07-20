# SimpleScreenShot - SSS

A simple screenshot program written in C + XCB api

## 📋 Features

+ 2 button program(hold left click to draw box, release to take pic, right click to quit)
+ Option to take screenshot of whole screen or drawn area
+ Minimal dependencies, no toolkits
+ Easily built from source, configurations made by editing source code

## 🚀 Download & Installation

Clone repository to directory of choice and enter cloned repository
```shell
git clone https://github.com/leath-dub/SimpleScreenShot.git
cd SimpleScreenShot
```
Run ``make`` to see build options
```shell
$ make

╭───┤ SSS ├──────────╮
│────────────────────│
│ sss build options: │
│   ├───> install    │
│   ├───> here       │
│   ├───> link       │
│   ├───> unlink     │
│   └───> uninstall  │
╰────────────────────╯

CFLAGS = -lxcb-image -lxcb -lxcb-shm -lpng16 -lz
CC     = gcc

```
To install normally run:
```shell
sudo make install
```
To uninstall normally run:
```shell
sudo make uninstall
```
❓Other build options:
    + here -> compiles to current directory
    + link -> compiles to current directory and creates symbolic link in binary directory
    + unlink -> removes symbolic link
