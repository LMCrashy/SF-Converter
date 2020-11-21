#pragma once

struct Glyph
{
	unsigned int				width;
	unsigned int				height;
	std::vector<unsigned int>	imgdata;

	Glyph(unsigned int w, unsigned int h) : width(w), height(h)	{ imgdata.reserve(w*h); }
};

struct FNTLoader
{
	std::vector<Glyph>	glyphs;

	unsigned int				width;
	unsigned int				height;
	unsigned int				char_offset;
	std::vector<unsigned int>	imgdata;

	void load(const char* name, const PALLoader &pal=PALLoader(), int pal_offset=0);
	void renderText(const char* text, unsigned int width_limit=-1, unsigned int offset=-1);
};