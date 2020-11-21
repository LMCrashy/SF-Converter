#pragma once

#include <vector>
struct IMBLoader
{
	unsigned int				width;
	unsigned int				height;
	std::vector<unsigned int>	imgdata;

	void load(const char* name, bool usepal = true);
};

