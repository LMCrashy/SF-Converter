#include "PCMToWave.h"
#include <fstream>
#include "Except.h"

struct WAVE_HEADER 
{
    char Chunk[4];
    unsigned int ChunkSize;
    char Sub_chunk1ID[8];
    unsigned int Sub_chunk1Size;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    char Sub_chunk2ID[4];
    unsigned int Sub_chunk2Size;
};

void PCMToWave::load(const char* file, bool _mono)
{
    mono = _mono;
    if (_mono)
    {
        std::string name1 = file;
        name1 += ".PCM";
        std::fstream file1(name1.c_str(), std::ios::in | std::ios::binary);

        if (!file1.is_open())	throw Except(name1.c_str());

        file1.read((char*)&header1, sizeof(header1));

        samplerate = header1.samplerate;
        sounddataMono.clear();
        sounddataMono.reserve(header1.length * 2);

        for (unsigned int counter = 0; counter < header1.length; counter++)
        {
            unsigned char m;
            file1.read((char*)&m, 1);
            sounddataMono.push_back(m);
        }
    }
}

void PCMToWave::saveToWave(const char* outpath)
{
    if (mono)
    {
        WAVE_HEADER waveheader;
        strcpy(waveheader.Chunk, "RIFF");
        waveheader.ChunkSize = sounddataMono.size() + sizeof(PCMHeader) + 36;

        strncpy(waveheader.Sub_chunk1ID, "WAVEfmt ", strlen("WAVEfmt "));
        waveheader.Sub_chunk1Size = 16;
        waveheader.AudioFormat = 1;
        waveheader.NumChannels = mono ? 1 : 2;
        waveheader.SampleRate = samplerate;
        waveheader.BitsPerSample = 8;
        waveheader.BlockAlign = waveheader.NumChannels * waveheader.BitsPerSample / 8;
        waveheader.ByteRate = waveheader.SampleRate * waveheader.NumChannels * waveheader.BitsPerSample / 8;

        // data sub-chunk
        strncpy(waveheader.Sub_chunk2ID, "data", strlen("data"));
        waveheader.Sub_chunk2Size = sounddataMono.size() + sizeof(PCMHeader);


        std::fstream file1(outpath, std::ios::out | std::ios::binary);

        file1.write((char*)&waveheader, sizeof(waveheader));
        file1.write((char*)&header1, sizeof(header1));
        file1.write((char*)sounddataMono.data(), waveheader.Sub_chunk2Size);

        file1.close();
    }
}