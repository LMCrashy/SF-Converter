
#include <string.h>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include "Except.h"
#include "IMBLoader.h"
#include "MODLoader.h"
#include "Vector.h"

#include <tiny_gltf.h>

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct MODMainHeader
{
  short			kind;		// Art des Files WORD
  int			numFace;	// Anzahl Faces
  float			zoom;
  int			numPoint;	// Anzahl Eckpunkte im Raum
  int			dwUnk1;
};

struct MODFaceHeader
{
	int		id;
	short	kind;
	char	skip1[4];
	int		alternativenumPoints;
	char	skip2[24];
};

struct MODFaceInfo
{
	int		buf1;
	int		buf2;
	int		numPoints;
	char	skip3[4];
};

struct MODFaceTexture
{
	short	skip1;
	short	x;
	short	skip2;
	short	y;
};

#pragma pack(pop)   /* restore original alignment from stack */


void MODLoader::load(const char* _name)
{
	std::string name_mod(_name);
	std::fstream file(name_mod.c_str(), std::ios::in | std::ios::binary);
	if(!file.is_open())
		throw Except(name_mod.c_str());

	points.clear();
	faces.clear();

	char			texture_name[9];
	MODMainHeader	header;
	MODFaceHeader	faceheader;
	MODFaceInfo		faceinfo;
	MODFaceTexture	texture;


	file.read((char*)&header, sizeof(header));

	scale = header.zoom;
    numIndices = 0;

	for(int counter = 0; counter < header.numPoint; counter++)
	{
		Point			point;
		unsigned int	id;
		file.read((char*)&id, sizeof(id));
		file.read((char*)&point, sizeof(point));
		points.insert(std::make_pair(id, point));
	}

	for(int counter = 0; counter < header.numFace; counter++)
	{
		file.read((char*)&faceheader, sizeof(faceheader));


		if(faceheader.kind != 1 && faceheader.kind != 0)
		{
			std::stringstream ss;
			ss << "unhandled faceheader.kind found: " << faceheader.kind << " Facecount: " << counter << "/" <<header.numFace;
			throw Except(ss.str());
		}

		if(faceheader.kind == 1)//0 == only new indices?
		{
			file.read((char*)&faceinfo, sizeof(faceinfo));

			faces.push_back(Face());
			Face &face = faces.back();
			//texturecoords
			for(int counter = 0; counter < faceinfo.numPoints; counter++)
			{
				file.read((char*)&texture, sizeof(texture));
				face.texturecoords.push_back(TextCoord(texture.x, texture.y));
			}

			int	str_len;
			char	skip6[4];

			memset(texture_name, 0, sizeof(texture_name));

			file.read((char*)&str_len, sizeof(str_len));
			file.read(skip6, sizeof(skip6));
			file.read(texture_name, str_len);

			face.id			= faceheader.id;
			face.texture	= texture_name;

			//indices
			for(int counter = 0; counter < faceinfo.numPoints; counter++)
			{
				Index index;
				file.read((char*)&index, sizeof(index));
				face.indices.push_back(index);

                numIndices++;
			}
		}
		else
		{
			//indices???
			for(int i = 0; i < faceheader.alternativenumPoints - 1; i++)
			{
				Index index;
				file.read((char*)&index, sizeof(index));
				//face.indices.push_back(index);
			}
		}
	}
}
tinygltf::Model MODLoader::toGlTF2(const std::string & relPathToRoot)
{
    tinygltf::Model model;

    for (const Face & face : faces)
    {
        std::vector<Point> vertices;
        std::vector<Point> normals;
        std::vector<Vec2>  texCoords;
        std::vector<unsigned short> indices;

        SubMesh & subMesh = SubMeshes[face.texture];
        subMesh.Material = face.texture;

        //compute face normal

        Vector A = points[face.indices[0].y];
        Vector B = points[face.indices[1].y];
        Vector C = points[face.indices[2].y];

        Vector normal = (C - A).cross(B - A);

        //add all vertices from this face
        short baseIndex = subMesh.LastIndex;
        for (int i = 0; i < face.indices.size(); ++i)
        {
            vertices.push_back(points[face.indices[i].y]);

            float u, v;
            u = (float)face.texturecoords[i].x / 256.f; //assume texture size is 256
            v = (float)face.texturecoords[i].y / 256.f;

            texCoords.push_back({ u,v });
            normals.push_back({ normal.x, normal.y, normal.z });

            subMesh.LastIndex++;
        }
        //now add triangles
        for (int i = 0; i < face.indices.size() - 1; ++i)
        {
            //we need to duplicate vertices, because texcoords are stored in face
            indices.push_back(baseIndex + i);
            indices.push_back(baseIndex + i + 1);
            indices.push_back(baseIndex + 0);
        }
 
        subMesh.Vertices.insert(subMesh.Vertices.end(),vertices.begin(), vertices.end());
        subMesh.Texcoords.insert(subMesh.Texcoords.end(),texCoords.begin(), texCoords.end());
        subMesh.Indices.insert(subMesh.Indices.end(), indices.begin(), indices.end());
        subMesh.Normals.insert(subMesh.Normals.end(), normals.begin(), normals.end());
    }

    tinygltf::Mesh mesh;

    tinygltf::Buffer buffer;

    int subMeshId = 0;

    unsigned int alignedOffset = 0;
    for (auto & subMesh : SubMeshes)
    {
        int baseIndex = subMeshId * 4;

        tinygltf::Primitive primitive;
        primitive.attributes["POSITION"]    = baseIndex + 1;
        primitive.attributes["TEXCOORD_0"]  = baseIndex + 2;
        primitive.attributes["NORMAL"]      = baseIndex + 3;
        primitive.indices = baseIndex + 0;
        primitive.material = subMeshId;
        primitive.mode = TINYGLTF_MODE_TRIANGLES;

        tinygltf::Accessor posAccessor;
        posAccessor.bufferView = baseIndex + 1;
        posAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        posAccessor.count = subMesh.second.Vertices.size();
        posAccessor.type = TINYGLTF_TYPE_VEC3;
        posAccessor.byteOffset = 0;
        posAccessor.minValues = { FLT_MAX,FLT_MAX,FLT_MAX };
        posAccessor.maxValues = { -FLT_MAX,-FLT_MAX,-FLT_MAX };

        tinygltf::Accessor texcoordAccessor;
        texcoordAccessor.bufferView = baseIndex + 2;
        texcoordAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        texcoordAccessor.count = subMesh.second.Texcoords.size();
        texcoordAccessor.type = TINYGLTF_TYPE_VEC2;
        texcoordAccessor.byteOffset = 0;

        tinygltf::Accessor normalAccessor;
        normalAccessor.bufferView = baseIndex + 3;
        normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        normalAccessor.count = subMesh.second.Normals.size();
        normalAccessor.type = TINYGLTF_TYPE_VEC3;
        normalAccessor.byteOffset = 0;

        tinygltf::Accessor indicesAccessor;
        indicesAccessor.bufferView = baseIndex + 0;
        indicesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        indicesAccessor.count = subMesh.second.Indices.size();
        indicesAccessor.type = TINYGLTF_TYPE_SCALAR;
        indicesAccessor.byteOffset = 0;

        mesh.primitives.push_back(primitive);


        int indexBufferOffset = alignedOffset;

        //write indices
        for (unsigned short index : subMesh.second.Indices)
        {
            char* t = static_cast<char*>((void*)&index);
            buffer.data.push_back(t[0]);
            buffer.data.push_back(t[1]);

            alignedOffset+=2;
        }

        int indexBufferSize = subMesh.second.Indices.size()  * 2;

        //align on 4 bytes
        while (alignedOffset % 4 != 0)
        {
            buffer.data.push_back('0');
            alignedOffset++;
        }

        int vertBufferOffset = alignedOffset;

        //convert vertices positions
        for (Point vertex : subMesh.second.Vertices)
        {
            char* x = static_cast<char*>((void*)&vertex.x);
            char* y = static_cast<char*>((void*)&vertex.z);
            char* z = static_cast<char*>((void*)&vertex.y); //we flip z and y to have y up
            
            posAccessor.minValues[0] = std::min((double)vertex.x, posAccessor.minValues[0]);
            posAccessor.minValues[1] = std::min((double)vertex.z, posAccessor.minValues[1]);
            posAccessor.minValues[2] = std::min((double)vertex.y, posAccessor.minValues[2]);

            posAccessor.maxValues[0] = std::max((double)vertex.x, posAccessor.maxValues[0]);
            posAccessor.maxValues[1] = std::max((double)vertex.z, posAccessor.maxValues[1]);
            posAccessor.maxValues[2] = std::max((double)vertex.y, posAccessor.maxValues[2]);

            buffer.data.push_back(x[0]);
            buffer.data.push_back(x[1]);
            buffer.data.push_back(x[2]);
            buffer.data.push_back(x[3]);

            buffer.data.push_back(y[0]);
            buffer.data.push_back(y[1]);
            buffer.data.push_back(y[2]);
            buffer.data.push_back(y[3]);

            buffer.data.push_back(z[0]);
            buffer.data.push_back(z[1]);
            buffer.data.push_back(z[2]);
            buffer.data.push_back(z[3]);

            alignedOffset += 12;
        }

        //align on 4 bytes
        while (alignedOffset % 4 != 0)
        {
            buffer.data.push_back('0');
            alignedOffset++;
        }

        int posBufferSize = subMesh.second.Vertices.size() * 3 * 4;

        int texcoordBufferOffset = alignedOffset;

        //convert texture coordinates positions
        for (Vec2 coord : subMesh.second.Texcoords)
        {
            char* x = static_cast<char*>((void*)&coord.x);
            char* y = static_cast<char*>((void*)&coord.y);

            buffer.data.push_back(x[0]);
            buffer.data.push_back(x[1]);
            buffer.data.push_back(x[2]);
            buffer.data.push_back(x[3]);

            buffer.data.push_back(y[0]);
            buffer.data.push_back(y[1]);
            buffer.data.push_back(y[2]);
            buffer.data.push_back(y[3]);

            alignedOffset += 8;
        }

        //align on 4 bytes
        while (alignedOffset % 4 != 0)
        {
            buffer.data.push_back('0');
            alignedOffset++;
        }

        int texcoordBufferSize = subMesh.second.Normals.size() * 2 * 4;

        int normalBufferOffset = alignedOffset;

        //convert texture coordinates positions
        for (Point normal : subMesh.second.Normals)
        {
            char* x = static_cast<char*>((void*)&normal.x);
            char* y = static_cast<char*>((void*)&normal.z);
            char* z = static_cast<char*>((void*)&normal.y); //we flip z and y to have y up

            buffer.data.push_back(x[0]);
            buffer.data.push_back(x[1]);
            buffer.data.push_back(x[2]);
            buffer.data.push_back(x[3]);

            buffer.data.push_back(y[0]);
            buffer.data.push_back(y[1]);
            buffer.data.push_back(y[2]);
            buffer.data.push_back(y[3]);

            buffer.data.push_back(z[0]);
            buffer.data.push_back(z[1]);
            buffer.data.push_back(z[2]);
            buffer.data.push_back(z[3]);

            alignedOffset += 12;
        }

        //align on 4 bytes
        while (alignedOffset % 4 != 0)
        {
            buffer.data.push_back('0');
            alignedOffset++;
        }

        int normalBufferSize = subMesh.second.Normals.size() * 3 * 4;

        tinygltf::BufferView indicesBufferView;
        indicesBufferView.buffer = 0;
        indicesBufferView.byteLength = indexBufferSize;
        indicesBufferView.byteOffset = indexBufferOffset;
        indicesBufferView.name = "indicesBufferView";
        indicesBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
        model.bufferViews.push_back(indicesBufferView);

        tinygltf::BufferView positionView;
        positionView.buffer = 0;
        positionView.byteLength = posBufferSize;
        positionView.byteOffset = vertBufferOffset;
        positionView.name = "positionBufferView";
        positionView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        model.bufferViews.push_back(positionView);

        tinygltf::BufferView texcoordView;
        texcoordView.buffer = 0;
        texcoordView.byteLength = texcoordBufferSize;
        texcoordView.byteOffset = texcoordBufferOffset;
        texcoordView.name = "texcoordBufferView";
        texcoordView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        model.bufferViews.push_back(texcoordView);

        tinygltf::BufferView normalView;
        normalView.buffer = 0;
        normalView.byteLength = normalBufferSize;
        normalView.byteOffset = normalBufferOffset;
        normalView.name = "normalBufferView";
        normalView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        model.bufferViews.push_back(normalView);

        tinygltf::Material material;
        material.name = subMesh.first;
        material.values["baseColorTexture"].json_double_value["index"] = subMeshId;
        model.materials.push_back(material);

        model.accessors.push_back(indicesAccessor);
        model.accessors.push_back(posAccessor);
        model.accessors.push_back(texcoordAccessor);
        model.accessors.push_back(normalAccessor);

        tinygltf::Image image;
        image.uri = relPathToRoot + "VFX/TEXTURE/" + subMesh.first + ".png";//todo
        model.images.push_back(image);

        tinygltf::Texture tex;
        tex.source = subMeshId;

        model.textures.push_back(tex);

        subMeshId += 1;

    }
    model.buffers.push_back(buffer);

    mesh.name = mName;
    model.meshes.push_back(mesh);


    tinygltf::Node node;
    node.mesh = 0;
    node.name = mName;
    float scaleF = 1.0f /*/ (scale / 100.0f)*/;
    node.scale = { scaleF, scaleF, scaleF };
    model.nodes.push_back(node);

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);

    tinygltf::Asset asset;
    asset.version = "2.0";
    asset.generator = "Schleichfarht model converter";

    model.asset = asset;

    return model;
}