#pragma once

struct R16Loader
{
	unsigned int				width;
	unsigned int				height;
	std::vector<unsigned int>	imgdata;

	void load(const char* name);
};
