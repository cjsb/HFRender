#pragma once
#include <string>

enum class RenderMode :int
{
	Forward = 0,
	Deferred
};

class Config
{
public:
	static Config* Instance()
	{
		static Config* config;
		if (config == nullptr)
		{
			config = new Config();
		}
		return config;
	}

	RenderMode render_mode = RenderMode::Forward;
	int width;
	int height;
	std::string project_path;
	int voxelSize = 256;
	int octreeLevel = 8;
	int brickPoolDim = 70 * 3;
	int shadowMapSize = 512;

private:
	Config() {}
};