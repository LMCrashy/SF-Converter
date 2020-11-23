#include <vector>
#include <fstream>
#include "Except.h"
#include "PALLoader.h"
#include "IMGLoader.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct IMGHeader
{
	unsigned short	info;
	unsigned int	width;
	unsigned int	height;
	unsigned int	skip1;
	unsigned int	skip2;
};

#pragma pack(pop)   /* restore original alignment from stack */

void IMGLoader::load(const char* name, const PALLoader &pal)
{
	IMGHeader header;
	std::fstream in(name, std::ios::in | std::ios::binary);

	if(!in.is_open())
		throw Except(name);

	in.read((char*)&header, sizeof(header));
	imgdata.clear();
	imgdata.reserve(header.height*header.width);

	width = header.width;
	height = header.height;

	switch(header.info)
	{
		case 0x400:
		{
			int zippedsize;
			in.read((char*)&zippedsize, sizeof(zippedsize));

			for(unsigned int y = 0; y < height; ++y)
			{
				unsigned int x = 0;
				unsigned short interleave;
				unsigned short readcount;

				do
				{
					in.read((char*)&interleave, sizeof(interleave));
					imgdata.insert(imgdata.end(), interleave, 0);

					in.read((char*)&readcount, sizeof(readcount));

					for(int counter = 0; counter < readcount; counter++)
						imgdata.push_back(pal.palette[in.get()]);

					x += interleave + readcount;
				}while(readcount);

				if(x != width)
					throw Except("total != header.width");
			}
		}break;

		case 0x401:
		{
            for (unsigned int counter = 0; counter < width * height; ++counter)
            {
                imgdata.push_back(pal.palette[in.get()]);
            }
		}break;

		case 0x413:
		{
			int zippedsize;
			in.read((char*)&zippedsize, sizeof(zippedsize));

			for(unsigned int y = 0; y < height; ++y)
			{
				unsigned int x = 0;
				unsigned short interleave;
				unsigned short readcount;

				do
				{
					in.read((char*)&interleave, sizeof(interleave));
					imgdata.insert(imgdata.end(), interleave, 0);

					in.read((char*)&readcount, sizeof(readcount));

					for(int counter = 0; counter < readcount; counter++)
					{
						unsigned short read;
						in.read((char*)&read, 2);

						unsigned char r = ((read >> 11) & 31) * 255 / 31;
						unsigned char g = ((read >> 5) & 63) * 255 / 63;
						unsigned char b = ((read >> 0) & 31) * 255 / 31;

						unsigned color = (b << 16) | (g << 8) | r;
						if(color) color |= 0xFF000000;

						imgdata.push_back(color);
					}

					x += interleave + readcount;
				}while(readcount);

				if(x != header.width)
					throw Except("startinterleave + readcount + endinterleave != header.width");
			}
		}break;

		default:
			throw Except("unexpected header.info");
	}
	if(imgdata.size() != header.height*header.width)
		throw Except("out.size() != header.height*header.width");
}
