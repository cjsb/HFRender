#include "helpers.h"
#include <algorithm>

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