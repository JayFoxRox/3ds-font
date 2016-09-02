3DS-Font
========

This is a tool to extract and re-create 3DS-Font files so they can be used in the Citra emulator or homebrew projects.

Development happens on GitHub. It's also where [the central repository](https://github.com/JayFoxRox/3ds-font) is hosted.

3DS-Font is licensed under the GPLv2 (or any later version). Refer to the license.txt file included.

### Building

Standard CMake:
```
cmake ..
make -j 4
```

### Running

* extract

`extract` is a tool to extract the texture data from the shared-font.
To run it use `mkdir tmp; ./extract <path>`.
It will generate textures in the tmp directory.

* generate

`generate` is a script to generate a new font texture map and glyph information.
Run it using `./generate.py`

