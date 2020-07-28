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