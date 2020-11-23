# SF-Converter

Licht !

This is a batch-converter for Archimedan Dynasty / Schleichfahrt resources.


Most of the file parsing is based on the SF-Reloaded loader https://sourceforge.net/p/schleichfahrt/code/ci/master/tree/
Please refer to the Jealous Jellyfish topic for more details on the file formats http://jealousjellyfish.de/viewtopic.php?f=8&t=241&start=70

## Usage

SFConverter [src folder] [dst folder]

### Example:

SFConverter f:\AD\CD\ f:\AD\Converted

## Epected result

+ R16, IMB, PCX, IMG files are converted into .PNG
+ PCM files are converted into .wav
+ MOD files are converted into glTF 2.0 format
+ MVI files
	+ Movies are converted into x246 .mp4
	+ I wasn't able to convert 12-4-ANF.MVI , maybe my own file copy is corrupt
	+ Inventory animations are converted into .PNG images
	+ ROOM / DESKTOP movies aren't converted, because it generates too many files

## Dependencies (included in this repo):

+ Tiny glTF : https://github.com/syoyo/tinygltf
+ STB : https://github.com/nothings/stb
+ ffmpeg : https://ffmpeg.org/

## Thanks

Thanks to the team that worked on SF-Reloaded, without their work I wouldn't have been able to do this.

