#pragma once

#include  "Vector.h"

struct TerrainPoint
{
	Vector vertex;
	Vector normal;
	short texture;
};

struct TerrainVertex
{
	Vector vertex;
	Vector normal;
	float texture_x;
	float texture_y;
};

struct SURLoader
{
	const unsigned int	width;
	const unsigned int	height;
	const unsigned int	tile_width;
	const unsigned int	tile_height;
	const unsigned int	texture_count;
	const unsigned int	tiles_in_a_row;
	const unsigned int	tiles_in_a_column;

	unsigned char mapdata[256*256];
	unsigned char dirdata[256*256];
	unsigned char heights[256*256];

	Vector scale;

	std::vector<unsigned int>	texture;
	std::vector<unsigned int>	indices;
	std::vector<TerrainVertex>	vertices;

	SURLoader(void);
	void preload(std::string path);
	void load(const char* pcx, const char* map, const char* dir, const char* xyz);
	void generate_terrain(void);
	float get_real_z(float x,float y);

	unsigned char get_z(unsigned int x,unsigned int y)				{ return heights[x+y*width]; }
	void set_z(unsigned int x, unsigned int y, unsigned char value)	{ heights[x+y*width] = value; }
};
