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
		uint32_t x = (d & 0x000003FF);
		uint32_t y = (d & 0x000FFC00) >> 10U;
		uint32_t z = (d & 0x3FF00000) >> 20U;
		file << x << " " << y << " " << z << std::endl;
	}

	file.close();
}

void dump_node_idx(uint32_t* data, uint32_t num, const std::string& path)
{
	std::ofstream file(path);
	for (int i = 0; i < num; i++)
	{
		uint32_t d = data[i];
		uint32_t f = (d & 0x80000000) >> 31;
		d = (d & 0x7FFFFFFF);
		file << f << " " << d << std::endl;
	}

	file.close();
}

int convVec4ToRGBA8(const glm::vec4& val)
{
	return (int(val.w) & 0x000000FF) << 24U | (int(val.z) & 0x000000FF) << 16U | (int(val.y) & 0x000000FF) << 8U | (int(val.x) & 0x000000FF);
}