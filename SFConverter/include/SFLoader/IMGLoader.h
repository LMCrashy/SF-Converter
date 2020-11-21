#pragma once

struct IMGLoader
{
	unsigned int				width;
	unsigned int				height;
	std::vector<unsigned int>	imgdata;

	void load(const char* name, const PALLoader &pal = PALLoader());
};

