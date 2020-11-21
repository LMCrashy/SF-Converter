#include <map>
#include <cmath>
#include <limits>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Except.h"
#include "PALLoader.h"
#include "IMBLoader.h"
#include "PCXLoader.h"
#include "SURLoader.h"

void CopyImage(const std::vector<unsigned int> &in, std::vector<unsigned int> &out, unsigned des_x_offset, unsigned des_y_offset, unsigned src_x_offset, unsigned src_y_offset, unsigned des_width, unsigned des_height, unsigned src_width)
{
	for(unsigned y = 0; y < des_height; y++)
	{
		for(unsigned x = 0; x < des_width; x++)
		{
			out[x+des_x_offset + (y+des_y_offset) * des_width] = in[x+src_x_offset + (y+src_y_offset) * src_width];
		}
	}
}

void TransformImage(const std::vector<unsigned int> &in, unsigned in_width, unsigned in_height, std::vector<unsigned int> &out, unsigned out_width, unsigned out_x_offset, unsigned out_y_offset, int rotate, bool mirror)
{
	for(unsigned y = 0; y < in_height; y++)
	{
		for(unsigned x = 0; x < in_width; x++)
		{
			int newx=-1, newy=-1;
			switch(rotate)
			{
			case 0:// 0°
				newx = x;
				newy = (mirror ? in_height - 1 - y : y);
				break;

			case 1:// 90°
				newx = (mirror ? y : in_height - 1 - y);
				newy = x;
				break;

			case 2:// 180°
				newx = (mirror ? in_height - 1 - y : y);
				newy = in_width - 1 - x;
				break;

			case 3:// 270°
				newx = in_width - 1 - x;
				newy = (mirror ? y : in_height - 1 - y);
				break;
			}

			out[newx+out_x_offset + (newy+out_y_offset) * out_width] = in[x + y * in_width];
		}
	}
}

SURLoader::SURLoader(void)
	//there are 17 textures a 4 subtextures a 8 directions = 544 textures => next power of 2 is 32x32 textures
	: width(256), height(256), tile_width(128), tile_height(128), texture_count(17), tiles_in_a_row(32), tiles_in_a_column(32)
{}

void SURLoader::preload(std::string path)
{
	IMBLoader					imbloader;
	std::vector<unsigned int>	tmp_texture;

	texture.clear();
	texture.insert(texture.begin(), tile_width * tile_height * tiles_in_a_row * tiles_in_a_column, 0);
	tmp_texture.insert(tmp_texture.begin(), tile_width * tile_height, 0);

	unsigned tex_mapping[] = {0,1,2,3,4,5,6,7,8,15,10,11,12,13,14,9,16};
	unsigned offsets[6][2] = { {0,0},{tile_width,0},{0,tile_height},{tile_width,tile_height} };

	for(unsigned counter = 0; counter < texture_count; counter++)
	{
		std::stringstream str;
		str << path << "S000_";
		str << std::setfill('0') << std::setw(3) << tex_mapping[counter];
		imbloader.load(str.str().c_str(), false);

		//split the texture in four parts
		for(unsigned part = 0; part < 4; part++)
		{
			CopyImage(imbloader.imgdata, tmp_texture,0,0,offsets[part][0],offsets[part][1],tile_width,tile_height, imbloader.width);
			//create the rotated/mirrored textures
			for(unsigned i = 0; i < 8; i++)
			{
				unsigned current_tex	= counter*4*8 + part*8 + i;
				unsigned x_offset		= tile_width * (current_tex % tiles_in_a_row);
				unsigned y_offset		= tile_height * (tiles_in_a_row - 1- (current_tex / tiles_in_a_row));
				TransformImage(tmp_texture, tile_width, tile_height, texture, tile_width * tiles_in_a_row,x_offset,y_offset, i / 2, i % 2 != 0);
			}
		}
	}
}

void SURLoader::load(const char* pcx, const char* map, const char* dir, const char* xyz)
{
	std::fstream mapfile(map, std::ios::in | std::ios::binary);
	std::fstream dirfile(dir, std::ios::in | std::ios::binary);
	std::fstream xyzfile(xyz, std::ios::in | std::ios::binary);


	if(!mapfile.is_open())	throw Except(map);
	if(!dirfile.is_open())	throw Except(dir);
	if(!xyzfile.is_open())	throw Except(xyz);

	PCXLoader pcxloader;
	pcxloader.load(pcx);

	assert(pcxloader.height == height && pcxloader.width == width);
	assert(sizeof(heights) == pcxloader.imgdata.size());

	//size		= 65.560 => 24 overhead
	//header	= 11 bytes
	//footer	= 13 bytes

	mapfile.seekg(11, std::ios::cur);
	dirfile.seekg(11, std::ios::cur);

	mapfile.read((char*)mapdata, sizeof(mapdata));
	dirfile.read((char*)dirdata, sizeof(dirdata));
	xyzfile.read((char*)&scale, sizeof(scale));

	for(size_t counter = 0; counter < sizeof(heights); counter++)
		heights[counter] = static_cast<unsigned char>(pcxloader.imgdata[counter]);
}

void SURLoader::generate_terrain(void)
{
	std::vector<TerrainPoint>	points;
	short dir_mapping[] = {1,3,7,5,6,4,0,2};
	TerrainPoint point;

	indices.clear();
	vertices.clear();

	points.reserve(width*height);
	indices.reserve(width*height*6);
	vertices.reserve(width*height*4);
	//generate the vertices
	for(unsigned y = 0; y < height; y++)
	{
		for(unsigned x = 0; x < width; x++)
		{
			unsigned index				= x + y * width;
			point.vertex.x	= x * scale.x;
			point.vertex.y	= y * scale.y;
			point.vertex.z	= heights[index] * scale.z;
			point.texture	= mapdata[index] * 8 + dir_mapping[dirdata[index]];

			points.push_back(point);
		}
	}

	//compute the normales
	for(unsigned y = 0; y < height; y++)
	{
		for(unsigned x = 0; x < width; x++)
		{
			Vector norm;
			unsigned index	= x + y * width;
			if(!x || !y || x + 1 == width || y + 1 == height)
				norm.z = 1;
			else
			{
				Vector neighbours[4]	= {	points[index - 1].vertex,		// x - 1
											points[index - width].vertex,	// y - 1
											points[index + 1].vertex,		// x + 1
											points[index + width].vertex};	// y + 1

				const Vector &v			= points[index].vertex;

				for(unsigned counter = 0; counter < 4; counter++)
				{
					Vector temp = (neighbours[counter] - v).cross(neighbours[(counter+1)%4] - v);
					temp.normalize();
					norm += temp;
				}
				norm.normalize();
			}
			points[index].normal = norm;
		}
	}
	float rel_tex_x = 1.0f / tiles_in_a_column;
	float rel_tex_y = 1.0f / tiles_in_a_row;
	//generate vertices and indices
	for(unsigned y = 0; y < height-1; y++)
	{
		for(unsigned x = 0; x < width-1; x++)
		{
			TerrainVertex vertex[4];
			unsigned index	= x + y * width;
			unsigned tex_x	= points[index+1].texture % tiles_in_a_column;
			unsigned tex_y	= points[index+1].texture / tiles_in_a_column;

			vertex[0].texture_x =     (tex_x + 0) * rel_tex_x;
			vertex[0].texture_y = 1 - (tex_y + 1) * rel_tex_y;
			vertex[1].texture_x =     (tex_x + 1) * rel_tex_x;
			vertex[1].texture_y = 1 - (tex_y + 1) * rel_tex_y;
			vertex[2].texture_x =     (tex_x + 1) * rel_tex_x;
			vertex[2].texture_y = 1 - (tex_y + 0) * rel_tex_y;
			vertex[3].texture_x =     (tex_x + 0) * rel_tex_x;
			vertex[3].texture_y = 1 - (tex_y + 0) * rel_tex_y;

			vertex[0].vertex = points[index].vertex;
			vertex[1].vertex = points[index+1].vertex;
			vertex[2].vertex = points[index+1+width].vertex;
			vertex[3].vertex = points[index+width].vertex;

			vertex[0].normal = points[index].normal;
			vertex[1].normal = points[index+1].normal;
			vertex[2].normal = points[index+1+width].normal;
			vertex[3].normal = points[index+width].normal;

			unsigned tricount = (x + y * (width-1))*4;
			indices.push_back(tricount);
			indices.push_back(tricount+1);
			indices.push_back(tricount+3);
			indices.push_back(tricount+1);
			indices.push_back(tricount+2);
			indices.push_back(tricount+3);
			vertices.push_back(vertex[0]);
			vertices.push_back(vertex[1]);
			vertices.push_back(vertex[2]);
			vertices.push_back(vertex[3]);
		}
	}
}

float SURLoader::get_real_z(float x, float z)
{
	Vector terrainsize,terrainpos;
	terrainsize.x = 255 * scale.x;
	terrainsize.y = 0;
	terrainsize.z = 255 * scale.y;

	terrainpos.x = 0;
	terrainpos.y = 0;
	terrainpos.z = 0;

	float triwidth	= terrainsize.x / 255;
	float triheight	= terrainsize.z / 255;

	float relx = x/terrainsize.x;
	float relz = z/terrainsize.z;
	int correctx = static_cast<int>(floor(relx* 255));
	int correctz = static_cast<int>(floor(relz* 255));
	float height1 = get_z(correctx,correctz) * scale.z;
	float height2 = get_z(correctx+1,correctz) * scale.z;
	float height3 = get_z(correctx,correctz+1) * scale.z;
	float height4 = get_z(correctx+1,correctz+1) * scale.z;
	float trix = (x - (static_cast<int>(x / triwidth) * triwidth)) / triwidth;
	float triy = (z - (static_cast<int>(z / triheight) * triheight)) / triheight;

	if(trix+triy < 1)
		return height1 + (height2 - height1) * trix + (height3 - height1) * triy;
	else
		return height4 + (height2 - height4) * (1-triy) + (height3 - height4) * (1-trix);
}
