#include <vector>
#include <fstream>
#include "Except.h"
#include "R16Loader.h"

void R16Loader::load(const char* name)
{
	std::fstream file(name, std::ios::in | std::ios::binary);

	if(!file.is_open())
		throw Except(name);

	width	= 640;
	height	= 480;
	imgdata.clear();
	imgdata.reserve(width*height);

	for(unsigned int counter = 0; counter < width*height; counter++)
	{
		unsigned short in;
		file.read((char*)&in, 2);
        if (file.eof()) break;

		unsigned char r = ((in >> 11) & 31) * 255 / 31;
		unsigned char g = ((in >> 5) & 63) * 255 / 63;
		unsigned char b = ((in >> 0) & 31) * 255 / 31;

        unsigned color = r;
        color |= g << 8;
        color |= b << 16;
		if(color) color |= 0xFF000000;
		imgdata.push_back(color);
	}
    if (imgdata.size() < 640 * 480)
    {
        width = 304;
        height = 284;
    }
}
