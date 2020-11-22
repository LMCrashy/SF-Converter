#pragma once

#include "Except.h"
#include <fstream>

class MVILoader
{
	struct Item
	{
		int	type;//1=video, 2=unknown(only films), 3=leftchannel, 4=rightchannel, 5=palette
		int	kind;
		unsigned int	length;
		int	offset;
	};

	std::vector<unsigned char>	out;
	std::vector<Item>			items;
	std::vector<Item>::iterator	current;
	std::ifstream				file;
	std::streampos				payload_offset;

public:
	MVILoader(void) : ignore_palettes(false)	{}
	MVILoader(const MVILoader&e)				{ throw Except("copy of MVILoader not allowed"); }

	unsigned					width;
	unsigned					height;
	int							fps;
	int							frames;
	bool						has_audio;
	bool						ignore_palettes;
	int							audio_num_channels;
	int							audio_samplerate;
	int							audio_bits;
	PALLoader					palette;
	std::vector<unsigned char>	indexdata;
	std::vector<unsigned int>	imgdata;
	std::vector<short>			sounddata;
	std::vector<char>			sounddata8bits;

	bool has_finished(void) const				{ return current == items.end(); }
	void set_to_frame(int frameoffset);
	void load(const char*);
	void decode_next_frame(void);
	void prefetch_sound();

    void writeAudioToWav(const char* out);
};
