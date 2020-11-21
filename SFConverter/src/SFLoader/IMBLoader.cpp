#include <string>
#include <vector>
#include <fstream>
#include "Except.h"
#include "PALLoader.h"
#include "IMBLoader.h"

void IMBLoader::load(const char* _name, bool usepal)
{
	PALLoader pal;
	std::string name_pal(_name);
	std::string name_imb(_name);

	name_pal += (usepal ? ".PAL" : ".S16");
	name_imb += ".IMB";

	std::fstream imb(name_imb.c_str(), std::ios::in | std::ios::binary);

	if(!imb.is_open())	throw Except(name_imb.c_str());

	pal.load(name_pal.c_str());

	short t_1;
	int t_2;
	int t_3;
	imb.read((char*)&t_1, sizeof(t_1));
	imb.read((char*)&height, sizeof(height));
	imb.read((char*)&width, sizeof(width));
	imb.read((char*)&t_2, sizeof(t_2));
	imb.read((char*)&t_3, sizeof(t_3));

	imgdata.clear();
	imgdata.reserve(height * width);

	for(unsigned counter = 0; counter < height * width; counter++)
	{
		unsigned char value;
		imb.read((char*)&value, sizeof(value));
		imgdata.push_back(pal.palette[value]);
	}
}
