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

void dump_buffer_data(uint32_t* data, uint32_t num, const std::string& path)
{
	std::ofstream file(path);
	for (int i = 0;i < num;i++)
	{
		uint32_t d = data[i];
		uint32_t x = (d & 0x000000FF);
		uint32_t y = (d & 0x0000FF00) >> 8U;
		uint32_t z = (d & 0x00FF0000) >> 16U;
		file << float(x) * 128 / 255 << " " << float(y) * 128 / 255 << " " << float(z) * 128 / 255 << std::endl;
	}

	file.close();
}