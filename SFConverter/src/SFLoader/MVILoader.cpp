#include <vector>
#include <fstream>
#include <cassert>
#include <string.h>
#include "Except.h"
#include "PALLoader.h"
#include "MVILoader.h"
#include "PCMToWave.h"

struct Header
{
	int	major;
	int	minor;
	int	width;
	int	height;
	int	num_entries;
	int	num_audio;
	int	num_video;
	int	fps;
	int	has_audio;
	int	audio_num_channels;
	int	audio_samplerate;
	int	audio_bits;
};


struct WAVE_HEADER
{
    char Chunk[4];
    unsigned int ChunkSize;
    char Sub_chunk1ID[8];
    unsigned int Sub_chunk1Size;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    char Sub_chunk2ID[4];
    unsigned int Sub_chunk2Size;
};

struct VideoDiffHead
{
	unsigned int	num_payload1;
	unsigned int	num_colors;//muss nur ungleich 1 sein um änderungen anzuzeigen
	unsigned int	num_payload2;
	unsigned int	num_runlengths;//immer 9601
};

struct VideoDiffSubHeader
{
	unsigned int size_payload;
	unsigned int num_unzipped;
};

struct Decoder
{
	struct Entry
	{
		unsigned char	value;
		unsigned short	parent;
		unsigned short	count;

		Entry(unsigned char value=0, unsigned short parent=2, unsigned short count=1)
			: value(value), parent(parent), count(count)
		{}
	};

	const char*					in;
	size_t						in_offset;
	const unsigned char			EoiCode;
	unsigned short				CodeSize;
	std::vector<Entry>			dict;
	std::vector<unsigned char>&	out;

	Decoder(const char* in, std::vector<unsigned char> &out, size_t in_offset = 0)
		: in(in), in_offset(in_offset), EoiCode(2), CodeSize(9), out(out)
	{
		dict.reserve(1<<CodeSize);
		dict.insert(dict.begin(), 3, Entry());
		//fill the dict with 3 dummies and the rest
		for(unsigned short counter = 0; counter < 256; counter++)
			dict.push_back(Entry(static_cast<unsigned char>(counter)));
	}

	unsigned short GetNextCode(void)
	{
		//get the code from a big endian, bitwise read input
		unsigned int intCode;

		char tempbuf[] = { in[in_offset/8+3], in[in_offset/8+2], in[in_offset/8+1], in[in_offset/8+0] };

		memcpy((char*)&intCode, tempbuf, 4);

		unsigned int l = intCode >> (32 - CodeSize -(in_offset%8));//big endian -> little endian
		unsigned int Code = l & (0xFFFFFFFF >> (32 - CodeSize));//mask

		in_offset += CodeSize;					// Increase Bit Offset

		return static_cast<unsigned short>(Code);
	}

	unsigned char WriteOut(unsigned short in)
	{
		out.insert(out.end(),dict[in].count, 0);
		size_t offset = out.size();
		unsigned char last_written;

		while(in != EoiCode)
		{
			out[--offset]	= dict[in].value;
			last_written	= dict[in].value;
			in				= dict[in].parent;
		}
		return last_written;
	}

	void Decode(void)
	{
		unsigned short Code;
		unsigned short OldCode = GetNextCode();

		WriteOut(OldCode);

		while ((Code = GetNextCode()) != EoiCode)
		{
			if (Code == 0)//next mask
			{
				CodeSize++;
				dict.reserve(1<<CodeSize);
				if(in_offset%8) in_offset = (in_offset/8+1)*8;
			}
			else if(Code == 1)
			{
				assert(false);
			}
			else
			{
				unsigned char last_written = WriteOut(Code >= dict.size() ? OldCode : Code);

				if(Code >= dict.size())	out.push_back(last_written);

				dict.push_back(Entry(last_written, OldCode, dict[OldCode].count+1));
				OldCode = Code;
			}
		}
	}
};

void MVILoader::load(const char* _name)
{
	Item item;
	Header header;

	std::string name_mvi(_name);
	name_mvi += ".MVI";

	file.open(name_mvi.c_str(), std::ios::out | std::ios::in | std::ios::binary);

	if(!file.is_open())
		throw Except(name_mvi.c_str());

	file.read((char*)&header, sizeof(header));

	items.clear();
	items.reserve(header.num_entries);
	imgdata.clear();
	imgdata.insert(imgdata.end(), header.height*header.width, 0);

	for(int counter = 0; counter < header.num_entries; counter++)
	{
		file.read((char*)&item, sizeof(item));
		items.push_back(item);
	}

	payload_offset = ((file.tellg() / 2048) + 1) * 2048;
	file.seekg(payload_offset);

	current				= items.begin();
	width				= header.width;
	height				= header.height;
	frames				= header.num_video;
	fps					= header.fps;
	has_audio			= header.has_audio == 1;
	audio_num_channels	= header.audio_num_channels;
	audio_samplerate	= header.audio_samplerate;
	audio_bits			= header.audio_bits;
}

void MVILoader::prefetch_sound()
{
	sounddata.clear();
    sounddata8bits.clear();

	if(!has_audio)
		return;

	std::vector<unsigned char> left;
	std::vector<unsigned char> right;

	const std::streampos old = file.tellg();

	for(std::vector<Item>::iterator	it = current; it != items.end(); it++)
	{
		switch(it->type)
		{
			case 5: //palette
			case 2: //video
			case 1: //video
			break;

			case 3: //leftchannel
			{
				char *in = new char[it->length];
				file.seekg(it->offset + payload_offset);
				file.read(in, it->length);
				left.insert(left.end(),in, in+it->length);
				delete[] in;
			}break;

			case 4: //rightchannel
			{
				char *in = new char[it->length];
				file.seekg(it->offset + payload_offset);
				file.read(in, it->length);
				right.insert(right.end(),in, in+it->length);
				delete[] in;
			}break;

			default:
				assert(false);
		}
	}

	std::vector<unsigned char>::iterator it_left	= left.begin();
	std::vector<unsigned char>::iterator it_right	= right.begin();

	sounddata.reserve((left.size() + right.size()) * 2);
    sounddata8bits.reserve((left.size() + right.size()) * 2);

	bool l,r;
	while((l = (it_left != left.end())) && (r = (it_right != right.end())) && (l || r))
	{
		if(l)
		{
			char m = (char)*it_left++;
			short value = m * 256;
			sounddata.push_back(value);
            sounddata8bits.push_back(m);
		}
		if(r)
		{
			char m = (char)*it_right++;
			short value = m * 256;
			sounddata.push_back(value);
            sounddata8bits.push_back(m);
		}
	}
	file.seekg(old);
}

void MVILoader::decode_next_frame(void)
{
	bool finished = false;
	for(; !finished && current != items.end(); current++)
	{
		char *in = new char[current->length];
		file.read(in, current->length);

		switch(current->type)
		{
			case 5: //palette
			{
				if(!ignore_palettes)
					palette.read(in, current->length == 772 ? 4 : 0, false);
				
				indexdata.clear();
				indexdata.reserve(current->length - 256 * 3);
				for(size_t counter = 256 * 3; counter < current->length; counter++)
					indexdata.push_back(in[counter]);
			}break;

			case 1: //video
			{
				out.clear();
				out.reserve(height * width);

				Decoder decoder(in, out, 8*8);
				decoder.Decode();

				assert(decoder.in_offset/8 <= current->length && current->length <= (decoder.in_offset+decoder.CodeSize)/8);
				finished = true;
			}break;

			case 2: //video
			{
				VideoDiffHead		head;
				VideoDiffSubHeader	subhead1;//wird ignoriert;
				VideoDiffSubHeader	subhead2;//wird ignoriert;
				std::vector<unsigned char> data1;
				std::vector<unsigned char> data2;

				size_t byte_offset = 0;

				memcpy(&head, in, sizeof(head));	byte_offset += sizeof(head);

				if(current->length == 16)
				{
					finished = true;
					break;
				}
				memcpy(&subhead1, in + byte_offset, sizeof(subhead1));	byte_offset += sizeof(subhead1);

				{
					size_t length = head.num_payload1 - sizeof(subhead1);

					data1.reserve(length);
					Decoder decoder(in + byte_offset, data1);
					decoder.Decode();

					assert(decoder.in_offset/8 <= length && length <= (decoder.CodeSize+decoder.in_offset)/8);
					byte_offset += length;
				}

				if(head.num_payload2 < head.num_runlengths)
				{
					memcpy(&subhead2, in+byte_offset, sizeof(subhead2));	byte_offset += sizeof(subhead2);

					size_t length = head.num_payload2 - sizeof(subhead2);

					data2.reserve(length);
					Decoder decoder(in + byte_offset, data2);
					decoder.Decode();

					assert(decoder.in_offset/8 <= length && length <= (decoder.CodeSize+decoder.in_offset)/8);
					byte_offset += length;
				}
				else
				{
					data2.insert(data2.end(), in + byte_offset, in + byte_offset + head.num_payload2);
					byte_offset += head.num_payload2;
				}

				assert(current->length >= sizeof(head) + sizeof(subhead1));
				assert(current->length == sizeof(head) + head.num_payload1 + head.num_payload2);
				assert(head.num_payload1 == sizeof(subhead1) + subhead1.size_payload + 8);
				assert(head.num_payload2 == head.num_runlengths || head.num_payload2 == sizeof(subhead2) + subhead2.size_payload + 8);
				assert(head.num_payload2 == head.num_runlengths || head.num_runlengths == subhead2.num_unzipped);
				assert(head.num_colors == subhead1.num_unzipped);
				assert(head.num_colors == data1.size());
				assert(head.num_runlengths == data2.size());

				int color_offset	= 0;
				int pixel_step		= (height * width) / out.size();

				for(unsigned int counter = 0; counter < head.num_runlengths-1; counter++)
				{
					unsigned char bitfield = data2[counter];

					for(int shift = 0; bitfield;shift++)
					{
						if(bitfield & 128)
							out[counter*8 + shift]	= data1[color_offset++];
						bitfield = bitfield << 1;
					}
				}
				if(pixel_step != 1)
					for(size_t counter = 0; counter < out.size() - 1; counter++)
						imgdata[counter*pixel_step+1] = palette[out[indexdata[out[counter] * 256 + out[counter+1]]]];


				finished = true;
			}break;

			case 3: //leftchannel
			case 4: //rightchannel
				break;

			default:
				assert(false);
		}
		delete[] in;
	}
	
	if(finished)
	{
		int pixel_step = (height * width) / out.size();

		if(pixel_step == 1)
			for(size_t counter = 0; counter < out.size(); counter++)
				imgdata[counter*pixel_step] = palette[out[counter]];
		else
			for(size_t y = 0; y < height; y++)
			{
				size_t off = y % 2;
				if(off)
					imgdata[y * width] = out[y * width / pixel_step];
				for(size_t x = off; x < width-1; x++)
				{
					size_t p = x + y * width;
					size_t i = p / pixel_step;
					imgdata[p+off] = palette[(x % 2 == 0 ? out[i] : indexdata[out[i] * 256 + out[i+1]])];
				}
			}
	}
}


void MVILoader::set_to_frame(int frameoffset)
{
	if(frames < frameoffset)
		throw Except("frames < frameoffset");

	current = items.begin() + frameoffset;
	if(frameoffset && current->offset == 0)
	{
		file.seekg(payload_offset);
		current = items.begin();
		for(int counter = 0; counter < frameoffset; counter++)
			decode_next_frame();
	}
	else
		file.seekg(current->offset + payload_offset);
}

void MVILoader::writeAudioToWav(const char* out)
{
    prefetch_sound();
    if (sounddata8bits.size() == 0) return;
    PCMHeader pcmHeader;
    pcmHeader.length = sounddata8bits.size();
    pcmHeader.samplerate = audio_samplerate / 2;

    WAVE_HEADER waveheader;
    strcpy(waveheader.Chunk, "RIFF");
    waveheader.ChunkSize = sounddata8bits.size() + sizeof(PCMHeader) + 36;

    strncpy(waveheader.Sub_chunk1ID, "WAVEfmt ", strlen("WAVEfmt "));
    waveheader.Sub_chunk1Size = 16;
    waveheader.AudioFormat = 1;
    waveheader.NumChannels = 2;
    waveheader.SampleRate = audio_samplerate / 2;
    waveheader.BitsPerSample = 16;
    waveheader.BlockAlign = waveheader.NumChannels * waveheader.BitsPerSample / 8;
    waveheader.ByteRate = waveheader.SampleRate * waveheader.NumChannels * waveheader.BitsPerSample / 8;

    // data sub-chunk
    strncpy(waveheader.Sub_chunk2ID, "data", strlen("data"));
    waveheader.Sub_chunk2Size = sounddata8bits.size() + sizeof(PCMHeader);


    std::fstream file1(out, std::ios::out | std::ios::binary);

    file1.write((char*)&waveheader, sizeof(waveheader));
    file1.write((char*)&pcmHeader, sizeof(pcmHeader));
    file1.write((char*)sounddata8bits.data(), waveheader.Sub_chunk2Size);

    file1.close();
}