#include "fileUtils.h"

vector<QString> fileUtils::loadInitFile(const string& initialFilePath)
{
	vector<QString> res;
	res.clear();
	ifstream ifs(initialFilePath);
	if (ifs.is_open())
	{
		string line;
		while (getline(ifs, line))
		{
			if (line.back() == '\r')
			{
				line.pop_back();
			}
			res.emplace_back(g_projectPath + QString::fromStdString(line));
		}
	}
	return res;
}
