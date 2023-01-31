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

// <============ ��Ŀ·�� =============>
const QString g_projectPath = QDir::currentPath();

namespace fileUtils
{
	/**
	* @brief: ��ȡ��ʼ������˸����ļ��г�ʼ�������ĵ���init.cfg��
	* @param: 
	* @return: 
	* ��ʽ: [0]: ԭʼͼƬ·��;[1]������ɺ󱸷�ͼƬ·��;
	* [2]: Ȩ���ļ���·��;[3]: ������·��;[4]: �������·��
	*/
	vector<QString> loadInitFile(const string& initialFilePath);
}

#endif // !__FILEUTILS_H__
