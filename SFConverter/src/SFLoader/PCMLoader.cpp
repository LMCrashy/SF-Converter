#include <vector>
#include <string>
#include <fstream>
#include "Except.h"
#include "PCMLoader.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct PCMHeader
{
	unsigned int	length;
	unsigned char	skip1[11];
	unsigned int	skip2;
	unsigned int	samplerate;
	unsigned int	skip3;
	unsigned int	skip4;
};

#pragma pack(pop)   /* restore original alignment from stack */

void PCMLoader::load(const char* file, bool _mono)
{
	mono = _mono;
	if(_mono)
	{
		std::string name1 = file;
		name1 += ".PCM";
		std::fstream file1(name1.c_str(), std::ios::in | std::ios::binary);

		if(!file1.is_open())	throw Except(name1.c_str());

		PCMHeader header1;
		file1.read((char*)&header1, sizeof(header1));

		samplerate = header1.samplerate;
		sounddata.clear();
		sounddata.reserve(header1.length * 2);

		for(unsigned int counter = 0; counter < header1.length; counter++)
		{
			unsigned char m;
			file1.read((char*)&m, 1);
			short value = m * 256 - 65536/2;
			sounddata.push_back(value);
		}
	}
	else
	{
		std::string name1 = file;
		std::string name2 = file;
		name1 += ".PCL";
		name2 += ".PCR";
		std::fstream file1(name1.c_str(), std::ios::in | std::ios::binary);
		std::fstream file2(name2.c_str(), std::ios::in | std::ios::binary);

		if(!file1.is_open())	throw Except(name1.c_str());
		if(!file2.is_open())	throw Except(name2.c_str());

		PCMHeader header1;
		PCMHeader header2;
		file1.read((char*)&header1, sizeof(header1));
		file2.read((char*)&header2, sizeof(header2));

		if(header1.length != header2.length)
			throw Except("header1.length != header2.length");
		if(header1.samplerate != header2.samplerate)
			throw Except("header1.samplerate != header2.samplerate");

		samplerate	= header1.samplerate;
		sounddata.clear();
		sounddata.reserve(header1.length * 4);

		for(unsigned int counter = 0; counter < header1.length; counter++)
		{
			unsigned char m1 = file1.get();
			unsigned char m2 = file2.get();
			short value1 = m1 * 256 - 65536/2;
			short value2 = m2 * 256 - 65536/2;
			sounddata.push_back(value1);
			sounddata.push_back(value2);
		}
	}
}
//
//void PCMLoader::add_linear_fades(unsigned int fadein, unsigned int fadeout)
//{
//	size_t samples = get_samples();
//	if(fadein >= samples)
//		throw Except("fadein is greater than the samplecount");
//	if(fadeout >= samples)
//		throw Except("fadeout is greater than the samplecount");
//
//	size_t offset = (samples - fadeout)*2;//stereo
//
//	for(unsigned int counter = 0; counter < fadein; counter++)
//	{
//		sounddata[counter+0] = (sounddata[counter+0] * counter) / fadein;
//		sounddata[counter+1] = (sounddata[counter+1] * counter) / fadein;
//	}
//	for(unsigned int counter = 0; counter < fadeout; counter++)
//	{
//		sounddata[counter+offset+0] = (sounddata[counter+offset+0] * counter) / fadeout;
//		sounddata[counter+offset+1] = (sounddata[counter+offset+1] * counter) / fadeout;
//	}
//}
