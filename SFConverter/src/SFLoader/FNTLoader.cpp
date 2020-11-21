#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "Except.h"
#include "PALLoader.h"
#include "FNTLoader.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct FNTHeader
{
	unsigned short	info;
	unsigned int	char_offset;
	unsigned int	glyphcount;
	unsigned int	max_width;
	unsigned int	max_height;
};

struct GlyphHeader
{
	unsigned int	width;
	unsigned int	height;
	unsigned int	skip1;
	unsigned int	skip2;
	unsigned int	size;
};

#pragma pack(pop)   /* restore original alignment from stack */

void FNTLoader::load(const char* name, const PALLoader &pal, int pal_offset)
{
	std::fstream in(name, std::ios::in | std::ios::binary);

	if(!in.is_open())
		throw Except(name);

	FNTHeader header;
	in.read((char*)&header, sizeof(header));
	glyphs.reserve(header.glyphcount);

	char_offset = header.char_offset;

	for(unsigned int counter = 0; counter < header.glyphcount; counter++)
	{
		GlyphHeader glyphe;
		in.read((char*)&glyphe, sizeof(glyphe));

		assert(glyphe.skip1 == 0);
		assert(glyphe.skip2 == 0);
		assert(header.max_width >= glyphe.width);
		assert(header.max_height >= glyphe.height);

		glyphs.push_back(Glyph(glyphe.width, glyphe.height));
		std::vector<unsigned int> &img = glyphs.back().imgdata;

		for(unsigned int y = 0; y < glyphe.height; y++)
		{
			unsigned short linetotal = 0;
			unsigned short interleave;
			unsigned short readcount;

			do
			{
				in.read((char*)&interleave, sizeof(interleave));
				img.insert(img.end(), interleave, 0);

				in.read((char*)&readcount, sizeof(readcount));

				for(int counter = 0; counter < readcount; counter++)
				{
					switch(header.info)
					{
						case 0x0402:
						{
							img.push_back(pal.palette[in.get()+pal_offset]);
						}break;

						case 0x0414:
						{
							unsigned short read;
							in.read((char*)&read, 2);

							unsigned char r = ((read >> 11) & 31) * 255 / 31;
							unsigned char g = ((read >> 5) & 63) * 255 / 63;
							unsigned char b = ((read >> 0) & 31) * 255 / 31;

							unsigned color = (r << 16) | (g << 8) | b;
							if(color) color |= 0xFF000000;

							img.push_back(color);
						}break;

						case 0x0412:
						case 0x0404:
						{
							img.push_back(0xFFFFFFFF);
						}break;

						default:
							throw Except("unkown file info");
					}
				}
				linetotal += interleave + readcount;
			}while(readcount);

			if(linetotal != glyphe.width)
				throw Except("failed: total != header.width");
		}
	}
}

void FNTLoader::renderText(const char* text, unsigned int width_limit, unsigned int offset)
{
	unsigned int lines			= 1;
	unsigned int line_width		= 0;
	unsigned int line_height	= 0;

	height	= 0;
	width	= 0;

	//if(offset != -1)
	//	char_offset = offset;
	//char_offset = 32;

	for(const unsigned char* ptr = (unsigned char*)text; *ptr; ptr++)
	{
		unsigned short value = *ptr;
		if(value == 13 || value == 10)
		{
			width = std::max(line_width, width);
			line_width = 0;
			lines++;
			continue;
		}

		if(value - char_offset > glyphs.size())
		{
			std::stringstream str;
			str << "char out of allowed range value: " << (int)*ptr <<"\toffset: "<<char_offset<<"\tglyphcount: "<<glyphs.size();
			throw Except(str.str().c_str());
		}

		Glyph &g	= glyphs[*ptr - char_offset];
		line_height	= std::max(g.height, line_height);
		line_width += g.width;
		if(line_width > width_limit)
		{
			width = std::max(line_width, width);
			line_width = 0;
			lines++;
		}
	}

	width		= std::max(line_width, width);
	height		= line_height * lines;
	line_width	= 0;
	lines		= 0;

	imgdata.clear();
	imgdata.insert(imgdata.begin(), height*width, 0);

	for(const unsigned char* ptr = (unsigned char*)text; *ptr; ptr++)
	{
		if(*ptr == 13 || *ptr == 10)
		{
			line_width = 0;
			lines++;
			continue;
		}
		Glyph &g	= glyphs[*ptr - char_offset];

		for(unsigned int y = 0; y < g.height; y++)
			for(unsigned int x = 0; x < g.width; x++)
				imgdata[(x + line_width) + (y + lines * line_height) * width] = g.imgdata[x + y * g.width];

		line_width += g.width;
		if(line_width > width_limit)
		{
			line_width = 0;
			lines++;
		}
	}
}
