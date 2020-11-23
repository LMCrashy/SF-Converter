
#include "SFLoader/IMBLoader.h"
#include "SFLoader/R16Loader.h"
#include "SFLoader/MODLoader.h"
#include "SFLoader/PCXLoader.h"
#include "SFLoader/MVILoader.h"
#include "SFLoader/IMGLoader.h"
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
int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("not enough arguments !");
        return 1;
    }
    std::string srcPath = "F:\\AD\\CD\\";
    std::string destination = "..\\output";

    srcPath = argv[1];
    destination = argv[2];
    destination += "\\";

    if (!fs::exists(destination))
    {
        fs::create_directory(destination);
    }

    std::map<std::string, std::string> palettesIndex;
    std::map<std::string, std::string> palettesIndexFolder;
    palettesIndex["MVILOGO"] = srcPath + "GFX/PAL/MOVIE/MOVIEPAL.PAL";
    palettesIndex["CUR_LOAD"] = srcPath + "GFX/PAL/GUI/CURSOR.PAL";
    palettesIndex["CUR_NORM"] = srcPath + "GFX/PAL/GUI/CURSOR.PAL";
    palettesIndex["APBG"] = srcPath + "GFX/PAL/NOTEBOOK/NOTEBOOK.PAL"; //Which palette ???

    palettesIndex["BIGBANG1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["BIGBANG2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["BIGBANG3"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["BULL1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["BULL2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["F04_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["F06_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["F08_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["F10_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["FLASH1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["FLASH2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["FLASH3"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["HAMMERHD"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["LEECH1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["LEECH2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["MANEATR1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["MANEATR2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["MELTER1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["MELTER2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["RIPPER"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["S02_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["S04_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["S06_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["S08_MAG"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["STANLEY1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["STANLEY2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["STANLEY3"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["SUBSONC1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["SUBSONC2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["THRESHR1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["THRESHR2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["TIGER1"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["TIGER2"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["ESTROKER"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";
    palettesIndex["MAGSYM"] = srcPath + "GFX/PAL/DEPOT/TORPEDO.PAL";

    palettesIndex["ARMOR1"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["ARMOR2"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["ARMOR3"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["ARMOR4"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["BOOSTER"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["BUZZER1"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["BUZZER2"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["CAPSULA"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["GENERAT1"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["GENERAT2"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["GENERAT3"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["NRSKIN"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["SILATOR"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["REPAIR"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["SONAR1"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["SONAR2"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["SONAR3"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";
    palettesIndex["SONAR4"] = srcPath + "GFX/PAL/DEPOT/SPECS.PAL";

    palettesIndexFolder["DEPOT"] = srcPath + "GFX/PAL/GUI/BORDER.PAL";
    palettesIndexFolder["CONNECT"] = srcPath + "GFX/PAL/CONNECT/AREAPOIN.PAL";
    palettesIndexFolder["ICON"] = srcPath + "GFX/PAL/DEPOT/GUN.PAL";
    palettesIndexFolder["GUI"] = srcPath + "GFX/PAL/GUI/BORDER.PAL";
    palettesIndexFolder["BGROUND"] = srcPath + "GFX/PAL/GUI/BORDER.PAL";
    palettesIndexFolder["TITLE"] = srcPath + "GFX/PAL/GUI/BORDER.PAL";
    palettesIndexFolder["NOTEBOOK"] = srcPath + "GFX/PAL/NOTEBOOK/NOTEBOOK.PAL";


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

            std::string outPath = destination + relPath + "\\" ;

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
            else if (extension == ".IMG")
            {
                if (entry.path().stem().string() == "COOKIEHC") continue;//skip this one, corrupt ?
                if (!fs::exists(outPath + stem + ".png"))
                {
                    printf("%s\n", filename.c_str());
                    std::string fullpath = directory;
                    fullpath += "\\" + filename;

                    IMGLoader loader;
                    if (palettesIndex.find(stem) != palettesIndex.end())
                    {
                        PALLoader palette;
                        palette.load(palettesIndex[stem].c_str());
                        loader.load(fullpath.c_str(), palette);
                    }
                    else if (palettesIndexFolder.find(directoryname) != palettesIndexFolder.end())
                    {
                        PALLoader palette;
                        palette.load(palettesIndexFolder[directoryname].c_str());
                        loader.load(fullpath.c_str(), palette);
                    }
                    else
                    {
                        loader.load(fullpath.c_str());
                    }
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

                if (directoryname == "ROOM") continue;      //skip small room animatons
                if (directoryname == "DESKTOP") continue;   //skip small background animatons

                printf("%s\n", filename.c_str());
                std::string fullpath = directory;
                fullpath += "\\" + stem;
                if (!fs::exists(outPath + stem + ".mp4") && !fs::exists(outPath + stem + "_0.png"))
                {
                    MVILoader loader;
                    loader.load(fullpath.c_str());

                    std::string audioString;
                    if (loader.has_audio)
                    {
                        loader.writeAudioToWav((outPath + stem + ".wav").c_str());
                        if (loader.sounddata8bits.size() > 0)
                        {
                            audioString = " -i " + outPath + stem + ".wav ";
                        }
                    }

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
                        std::string command = "ffmpeg -r " + std::to_string(loader.fps) + " -i " + pattern + audioString +" -c:v libx264rgb -crf 0 " + outPath + stem + ".mp4";
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
            if (!fs::exists(destination + entry.path().string()))
            {
                std::string relPath = entry.path().string();
                relPath.erase(relPath.begin(), relPath.begin() + srcPath.length());

                fs::create_directory(destination + relPath);
            }
        }
    }
	

    return 0;
}