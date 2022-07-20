# SimpleScreenShot - SSS

A simple screenshot program written in C + XCB api

## 📋 Features

+ 2 button program(hold left click to draw box, release to take pic, right click to quit)
+ Creates PNG format output screenshots
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
### ❓Other build options:

+ here 🠒 compiles to current directory
+ link 🠒 compiles to current directory and creates symbolic link in binary directory
+ unlink 🠒 removes symbolic link

## 🏃‍♂️ Usage

SS_DIR = ``$PWD`` (specified in ``config.h``)

Run with no arguments(creates ``SS_DIR/screenshot.png``):
```shell
sss
```
Run with `-n` option to specify a name(put in ``SS_DIR``):
```shell
sss -n test.png
```
Run with `-f` option alone(creates ``SS_DIR/fullscreen.png``):
```shell
sss -f
```
Run with `-f` option and specified name(put in ``SS_DIR``):
```shell
sss -f test.png
```

### 🗒️ Notes

+ the last box drawn will be the screenshot written
+ the program does not append numbers, e.g. ``ss-1.png, ss-2.png, etc``.
if there is a conflict in nameing the old version is overwritten

[Watch the video](https://imgur.com/a/8Ogb0IH)
## ⚙️ Configuration

Configuration is very minimal currently. It is done through the ``config.h``
source file, the contents are as follows:
```c
#define LINE_WIDTH 4 /* width of drawn box lines */
#define LINE_COLOR 0xEBDBB2 /* color of drawn box lines */
#define SS_DIR SRC_DIR "/screenshots/" /* directory you want screenshots to go in */
```

### 🗒️ Notes

+ The actual captured area will be bound to the size drawn by the user, excluding
the lines visable and what they obscure
+ You must recompile each time you make a change to configuration
+ If you wish to remove the source files after compilation, this is fine,
just make sure that you are happy with your configuration as you cannot change
it without recompiling
+ You must give ``SS_DIR`` an absolute path if you remove ``SRC_DIR``. E.g.
if I wanted to put screenshots in my home folder I would change:
```c
#define SS_DIR SRC_DIR "/screenshots/" /* directory you want screenshots to go in */
```
to:
```c
#define SS_DIR "/home/MYNAME/" /* directory you want screenshots to go in */
```
