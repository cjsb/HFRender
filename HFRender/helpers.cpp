#include "helpers.h"
#include <algorithm>
#include <fstream>

std::vector<std::string> Split(const std::string& s, const std::string& t)
{
	std::vector<std::string> res;
	std::string tmp = s;
	while (!tmp.empty())
	{
		size_t pos = tmp.find(t);
		if (pos == std::string::npos)
		{
			res.push_back(tmp);
			break;
		}
		else if (pos > 0)
		{
			res.push_back(tmp.substr(0, pos));
		}
		
		tmp = tmp.substr(pos + t.size());
	}
	return res;
}

void dump_data(float* data, uint32_t stride, uint32_t count, const std::string& path)
{
	std::ofstream file(path);
	for (uint32_t i = 0;i < count;i++)
	{
		for (uint32_t j = 0;j < stride;j++)
		{
			file << data[i * stride + j] << " ";
		}
		file << std::endl;
	}
	file.close();
}

void dump_3D_data(float* data, uint32_t stride, uint32_t width, uint32_t height, uint32_t depth, const std::string& path)
{
	std::ofstream file(path);
	for (uint32_t x = 0;x < width;x++)
	{
		for (uint32_t y = 0;y < height;y++)
		{
			for (uint32_t z = 0;z < depth;z++)
			{
				int id = x * height * depth + y * depth + z;
				for (uint32_t i = 0;i < stride - 1;i++)
				{
					if (data[id * stride + i] > 0)
					{
						file << x << " " << y << " " << z << std::endl;
						break;
					}
				}
			}
		}
	}
	file.close();
}