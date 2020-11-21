#include <vector>
#include <cassert>
#include <fstream>
#include "Except.h"
#include "PALLoader.h"
#include "PCXLoader.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct PCXHeader
{
	unsigned char	Manufacturer;
	unsigned char	Version;
	unsigned char	Encoding;
	unsigned char	BitsPerPixel;
	unsigned short	XMin;
	unsigned short	YMin;
	unsigned short	XMax;
	unsigned short	YMax;
	unsigned short	HorizDPI;
	unsigned short	VertDPI;
	unsigned char	Palette[48];
	unsigned char	Reserved;
	unsigned char	Planes;
	unsigned short	BytesPerLine;
	unsigned short	PaletteType;
	unsigned short	HScrSize;
	unsigned short	VScrSize;
	unsigned char	Filler[54];
};

#pragma pack(pop)   /* restore original alignment from stack */


void PCXLoader::load(const char* name)
{
	std::fstream in(name, std::ios::in | std::ios::binary);

	if(!in.is_open())
		throw Except(name);

	PCXHeader header;
	in.read((char*)&header, sizeof(header));

	if (header.Manufacturer != 0x0a && header.Encoding != 0x01)
		throw Except("header.Manufacturer != 0x0a && header.Encoding != 0x01");

	if (header.BitsPerPixel != 8 || header.Planes != 1)
		throw Except("header.BitsPerPixel != 8 || header.Planes != 1");

	// the palette indicator (usually a 0x0c is found infront of the actual palette data)
	// is ignored because some exporters seem to forget to write it. This would result in
	// no image loaded before, now only wrong colors will be set.
	char buffer[256*3];
	const std::streampos pos = in.tellg();
	in.seekg(-256*3, std::ios::end);
	in.read(buffer, sizeof(buffer));
	in.seekp(pos);
	palette.read(buffer,0,false);

	// read image data
	width = header.XMax - header.XMin + 1;
	height = header.YMax - header.YMin + 1;
	const int imagebytes = header.BytesPerLine * height;
	imgdata.clear();
	imgdata.reserve(width*height);

	unsigned char count, value;
	for(int offset = 0; offset < imagebytes; offset += count)
	{
		in.read((char*)&count, 1);
		if((count & 0xc0) != 0xc0)
		{
			value = count;
			count = 1;
		}
		else
		{
			count &= 0x3f;
			in.read((char*)&value, 1);
		}
		imgdata.insert(imgdata.end(), count, palette[value]);
	}
	if(imgdata.size() != width * height)
		throw Except("imgdata.size() != width * height");
}

