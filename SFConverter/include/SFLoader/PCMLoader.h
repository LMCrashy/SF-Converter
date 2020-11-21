
struct PCMLoader
{
	bool				mono;
	unsigned int		samplerate;
	std::vector<short>	sounddata;//16bit for stereo, 8 for mono, 1. word = left, 2. word = right, 3. word = left etc., mono = no mixing

	void load(const char* file, bool mono);
	size_t get_samples(void) const { return mono ? sounddata.size() : sounddata.size()/2; }
};