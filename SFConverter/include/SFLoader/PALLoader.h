#pragma once

struct PALLoader
{
	unsigned int palette[256];

	PALLoader(void);
	void load(const char* name);
	void read(const char* data, unsigned int offset, bool alpha=true);

	unsigned int operator[](int pos) const	{ return palette[pos]; }
};