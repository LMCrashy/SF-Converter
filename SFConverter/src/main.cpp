
#include "SFLoader/IMBLoader.h"
#include "SFLoader/R16Loader.h"
#include "SFLoader/MODLoader.h"
#include "SFLoader/PCXLoader.h"
#include <experimental/filesystem>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

namespace fs = std::experimental::filesystem ;
int main()
{

    std::string srcPath = "F:/AD/CD/";
    for (auto& entry : fs::recursive_directory_iterator(srcPath))
    {
        if (!fs::is_directory(entry.status()))
        {
            std::string extension = entry.path().extension().string();
            std::string directory = entry.path().parent_path().string();

            std::string relPath = entry.path().parent_path().string();
            relPath.erase(relPath.begin(), relPath.begin() + (srcPath.length()-1));

            std::string outPath = "../output/" + relPath + "/" ;

            if (extension == ".IMB")
            {
                printf("%s\n", entry.path().filename().string().c_str());
                bool hasPalette = false;
                std::string fullpath = entry.path().parent_path().string();
                fullpath += "\\"+entry.path().stem().string();

                /*if (fs::exists(fullpath + ".PAL"))
                {
                    hasPalette = true;
                }*/

                IMBLoader loader;
                loader.load(fullpath.c_str(), hasPalette);
                void* data = loader.imgdata.data();
                stbi_write_png((outPath + entry.path().stem().string()+".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
            }
            else if (extension == ".R16")
            {
                printf("%s\n", entry.path().filename().string().c_str());
                std::string fullpath = entry.path().parent_path().string();
                fullpath += "\\" + entry.path().filename().string();

                R16Loader loader;
                loader.load(fullpath.c_str());
                void* data = loader.imgdata.data();
                stbi_write_png((outPath + entry.path().stem().string() + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
            }
            else if (extension == ".PCX")
            {
                printf("%s\n", entry.path().filename().string().c_str());
                std::string fullpath = entry.path().parent_path().string();
                fullpath += "\\" + entry.path().filename().string();

                PCXLoader loader;
                loader.load(fullpath.c_str());
                void* data = loader.imgdata.data();
                stbi_write_png((outPath + entry.path().stem().string() + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
            }
            else if (extension == ".MOD")
            {
                std::string fullpath = entry.path().parent_path().string();
                fullpath += "\\" + entry.path().filename().string();

                MODLoader loader;
                loader.load(fullpath.c_str());
                printf("%s, scale : %f\n", entry.path().filename().string().c_str(), loader.scale);

                tinygltf::TinyGLTF tinyGLTF;

                tinyGLTF.WriteGltfSceneToFile(&loader.toGlTF2("../../"), outPath + entry.path().stem().string() + ".gltf", false, false);
                
            }
        }
        else
        {
            if (!fs::exists("../output/" + entry.path().string()))
            {
                std::string relPath = entry.path().string();
                relPath.erase(relPath.begin(), relPath.begin() + srcPath.length());

                fs::create_directory("../output/" + relPath);
            }
        }
    }
	

    return 0;
}