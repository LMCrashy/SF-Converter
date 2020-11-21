#include <fstream>
#include <sstream>
#include "Except.h"
#include "PALLoader.h"

struct myexception: public std::exception
{
	std::string str;

	myexception(unsigned short i)				{ std::stringstream s; s << "unknown info-tag: " << i; str = s.str(); }
	virtual ~myexception() throw()              { }
	virtual const char* what() const throw()	{ return str.c_str(); }
};

PALLoader::PALLoader(void)
{
	for(unsigned counter = 0; counter < 256; counter++)
		palette[counter] = 0xFF000000 | counter;
	palette[0] = 0;
}

void PALLoader::load(const char* name)
{
	std::fstream pal(name, std::ios::in | std::ios::binary);

	if(!pal.is_open())
		throw Except(name);

	unsigned short info;
	pal.read((char*)&info, sizeof(info));

	if(info == 0)			pal.seekg(2, std::ios::cur);
	else if(info == 2050)	pal.seekg(12, std::ios::cur);
	else					throw myexception(info);

	char buffer[256*3];
	pal.read(buffer, sizeof(buffer));
	read(buffer,0);
}

void PALLoader::read(const char *data, unsigned int offset, bool alpha)
{
	for(unsigned counter = 0; counter < 256; counter++)
	{
		unsigned char r = (unsigned char)data[offset + counter * 3];
		unsigned char g = (unsigned char)data[offset + counter * 3 + 1];
		unsigned char b = (unsigned char)data[offset + counter * 3 + 2];

		palette[counter] = (b << 16) | (g << 8) | r;
		if(!alpha || palette[counter]) palette[counter] |= 0xFF000000;
	}
}
