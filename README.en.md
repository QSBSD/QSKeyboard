<div align="center">
  <img src="https://avatars.githubusercontent.com/u/329755507?s=200&v=4" alt="QSKeyboard" width="120" height="120">

  # QSKeyboard

  On-screen keyboard for X11

  [![License: BSD 2-Clause](https://img.shields.io/badge/License-BSD%202--Clause-blue.svg)](./LICENSE)

  [![Русский](https://img.shields.io/badge/lang-Русский-lightgrey)](README.md)
  [![English](https://img.shields.io/badge/lang-English-blue)](README.en.md)
</div>

## About

QSKeyboard is an on-screen keyboard for X11 that appears on a touchscreen gesture. Supported platforms: Linux, FreeBSD.

## Features

**Show and hide**
 - Show: two quick taps with two fingers on a touchscreen
 - Hide: a touch or click outside the keyboard, or the input focus moving to another window

**!AT key (Alt+Tab)**
 - Alt stays held for 3 s after the last press (each press taps Tab and restarts the 3 s)
 - Alt is released by the timeout, by any other key, when the keyboard hides and when the program exits

**!HK key (hotkeys)**
 - Switches to a hotkey layout: one button per line of `~/.config/QSKeyboard/hotkey.conf` (created on the first start)
 - 5 rows x 6 slots, the last slot is `ABC`
 - Pressing a button emulates that combination and hides the keyboard; `ABC` goes back to letters

**Close key**
 - Quits the program

`~/.config/QSKeyboard/hotkey.conf` format: one combination per line, keys joined with `+`, `#` starts a comment, up to 29 lines are used.

Modifiers: `ctrl`, `alt`, `shift`, `super`. Other keys are X11 keysym names.

## Build / Run

**FreeBSD**

```
doas pkg install gtk3 libXi libXtst libX11 pkgconf

make
./QSKeyboard
```

**Ubuntu / Debian**

```
sudo apt install build-essential pkg-config libgtk-3-dev libxi-dev libxtst-dev libx11-dev

make
./QSKeyboard
```

## Technologies

 - C, GTK3, XInput2, XTest, Xlib

## License

Distributed under the terms of the BSD 2-Clause license, as stated
in the [LICENSE](./LICENSE) file.

```
Copyright (c) 2026, QSBSD Contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```