#pragma once

#include <map>
#include <tiny_gltf.h>
#include <Vector.h>

struct TextCoord
{
	short x;
	short y;

	TextCoord(short x, short y) : x(x), y(y) {}
};


struct Index
{
	int x;
	int y;
	int z;
};

struct Face
{
	int						id;
	std::string				texture;
	std::vector<TextCoord>	texturecoords;
	std::vector<Index>		indices;
};

struct Vec2
{
    float x, y;
};
struct SubMesh
{
    std::string Material;
    std::vector<unsigned short> Indices;
    std::vector<Point> Vertices;
    std::vector<Point> Normals;
    std::vector<Vec2> Texcoords;

    unsigned short LastIndex = 0;
};

struct MODLoader
{
	float						scale;
	std::map<int, Point>		points;
	std::vector<Face>			faces;

    std::map<std::string, SubMesh> SubMeshes;


    int                         numIndices;

	void load(const char* name);

    std::string mName;

    tinygltf::Model toGlTF2(const std::string & relPathToRoot);
};
