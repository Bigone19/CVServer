#pragma once
#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

#include <fstream>
#include <vector>
#include <string>

#include "config.h"

using std::vector;
using std::string;
using std::ifstream;

// <============ 项目路径 =============>
const QString g_projectPath = QDir::currentPath();

namespace fileUtils
{
	/**
	* @brief: 读取初始化服务端各个文件夹初始化配置文档（init.cfg）
	* @param: 
	* @return: 
	* 格式: [0]: 原始图片路径;[1]处理完成后备份图片路径;
	* [2]: 权重文件夹路径;[3]: 处理结果路径;[4]: 类别名称路径
	*/
	vector<QString> loadInitFile(const string& initialFilePath);
}

#endif // !__FILEUTILS_H__
