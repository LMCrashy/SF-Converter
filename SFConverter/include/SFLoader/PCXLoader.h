#pragma once

#include "PALLoader.h"

struct PCXLoader
{
	PALLoader					palette;
	unsigned int				width;
	unsigned int				height;
	std::vector<unsigned int>	imgdata;

	void load(const char* name);
};
