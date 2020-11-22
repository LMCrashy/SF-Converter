
#include "SFLoader/IMBLoader.h"
#include "SFLoader/R16Loader.h"
#include "SFLoader/MODLoader.h"
#include "SFLoader/PCXLoader.h"
#include "SFLoader/MVILoader.h"
#include "PCMToWave.h"
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

    if (!fs::exists("../output/"))
    {
        fs::create_directory("../output/");
    }

    for (auto& entry : fs::recursive_directory_iterator(srcPath))
    {
        if (!fs::is_directory(entry.status()))
        {
            std::string extension = entry.path().extension().string();
            std::string stem = entry.path().stem().string();
            std::string filename = entry.path().filename().string();
            std::string directory = entry.path().parent_path().string();
            std::string directoryname = entry.path().parent_path().filename().string();

            std::string relPath = directory;
            relPath.erase(relPath.begin(), relPath.begin() + (srcPath.length()-1));

            std::string outPath = "..\\output" + relPath + "\\" ;

            if (extension == ".IMB")
            {
                if (!fs::exists(outPath + stem + ".png"))
                {
                    printf("%s\n", filename.c_str());
                    bool hasPalette = false;
                    std::string fullpath = directory;
                    fullpath += "\\" + stem;

                    //if (fs::exists(fullpath + ".PAL"))
                    //{
                    //    hasPalette = true;
                    //}

                    IMBLoader loader;
                    loader.load(fullpath.c_str(), hasPalette);
                    void* data = loader.imgdata.data();
                    stbi_write_png((outPath + stem + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
                }
            }
            else if (extension == ".R16")
            {
                if (!fs::exists(outPath + stem + ".png"))
                {
                    printf("%s\n", filename.c_str());
                    std::string fullpath = directory;
                    fullpath += "\\" + filename;

                    R16Loader loader;
                    loader.load(fullpath.c_str());
                    void* data = loader.imgdata.data();
                    stbi_write_png((outPath + stem + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
                }
            }
            else if (extension == ".PCX")
            {
                if (!fs::exists(outPath + stem + ".png"))
                {
                    printf("%s\n", filename.c_str());
                    std::string fullpath = directory;
                    fullpath += "\\" + filename;

                    PCXLoader loader;
                    loader.load(fullpath.c_str());
                    void* data = loader.imgdata.data();
                    stbi_write_png((outPath + stem + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
                }
            }
            else if (extension == ".MOD")
            {
                if (!fs::exists(outPath + stem + ".gltf"))
                {
                    std::string fullpath = directory;
                    fullpath += "\\" + filename;

                    MODLoader loader;
                    loader.load(fullpath.c_str());
                    printf("%s, scale : %f\n", filename.c_str(), loader.scale);

                    tinygltf::TinyGLTF tinyGLTF;

                    tinyGLTF.WriteGltfSceneToFile(&loader.toGlTF2("../../"), outPath + stem + ".gltf", false, false);
                }
                
            }
            else if (extension == ".MVI")
            {
                if (entry.path().stem().string() == "12-4-ANF") continue;//skip this one, corrupt ?

                if (directoryname == "ROOM") continue;//skip small room animatons
                if (directoryname == "DESKTOP") continue;//skip small background animatons

                printf("%s\n", filename.c_str());
                std::string fullpath = directory;
                fullpath += "\\" + stem;
                if (!fs::exists(outPath + stem + ".mp4") && !fs::exists(outPath + stem + "_0.png"))
                {
                    MVILoader loader;
                    loader.load(fullpath.c_str());
                    for (int i = 0; i < loader.frames; ++i)
                    {
                        loader.decode_next_frame();
                        void* data = loader.imgdata.data();
                        stbi_write_png((outPath + stem + "_" + std::to_string(i) + ".png").c_str(), loader.width, loader.height, 4, data, loader.width * 4);
                    }

                    //for movies in THING and SFLIP, keep it in png
                    if (   directoryname != "ROOM"
                        && directoryname != "DESKTOP"
                        && directoryname != "THING"
                        && directoryname != "SFLIP")
                    {
                        //launch ffmpeg
                        std::string pattern = outPath + stem + "_%d.png";
                        std::string command = "ffmpeg -r " + std::to_string(loader.fps) + " -i " + pattern + " -c:v libx264rgb -crf 0 " + outPath + stem + ".mp4";
                        system(command.c_str());

                        //remove temp files
                        for (int i = 0; i < loader.frames; ++i)
                        {
                            fs::remove(outPath + stem + "_" + std::to_string(i) + ".png");
                        }
                    }
                }
            }
            else if (extension == ".PCM")
            {
                if (!fs::exists(outPath + stem + ".wav"))
                {
                    printf("%s\n", filename.c_str());
                    std::string fullpath = directory;

                    fullpath += "\\" + stem;

                    PCMToWave loader;
                    loader.load(fullpath.c_str(), true);
                    loader.saveToWave((outPath + stem + ".wav").c_str());
                }
            }
            else if (extension == ".PCL")
            {
                if (!fs::exists(outPath + stem + ".wav"))
                {
                    printf("%s\n", filename.c_str());
                    std::string fullpath = directory;

                    fullpath += "\\" + stem;

                    PCMToWave loader;
                    loader.load(fullpath.c_str(), false);
                    loader.saveToWave((outPath + stem + ".wav").c_str());
                }
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