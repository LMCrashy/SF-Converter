#pragma once

#include <vector>

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

struct PCMHeader
{
    unsigned int	length;
    unsigned char	skip1[11];
    unsigned int	skip2;
    unsigned int	samplerate;
    unsigned int	skip3;
    unsigned int	skip4;
};

#pragma pack(pop)   /* restore original alignment from stack */

struct PCMToWave
{
    PCMHeader           header1;
    PCMHeader           header2;
    bool				mono;
    unsigned int		samplerate;
    std::vector<char>	sounddata;  //8 for mono, 16bit for stereo, interleaved

    void load(const char* file, bool mono);
    void saveToWave(const char* out);
};