#pragma once
#ifndef __YOLOUTILS_H__
#define __YOLOUTILS_H__

#include <fstream>
#include <vector>
#include <math.h>

#include "yoloConfig.h"

using std::vector;
using std::wstring;
using std::ifstream;
using std::round;

namespace yoloUtils
{
	// �ı����ű���
	const vector<double> g_scaleVec = { 0.32, 0.64, 1, 1.5, 2, 3, 4, 5, 6 };
	/**
	* @brief: ��ȡ��������С
	* @param: 
	*/
	size_t vecProductSize(const vector<int64_t>& vec);
	/**
	* @brief: ���ؼ������
	* @param: 
	*/
	vector<string> loadClsNames(const string& path);
	/**
	* @brief: ��������ӻ�
	* @param: 
	*/
	void visualizeDetection(Mat& srcImage, vector<Detection>& vecDetection,
		const vector<std::string>& vecClsName);
	/**
	* @brief: �����ı���ǩ
	* @param: 
	*/
	void letterbox(const Mat& srcImage, Mat& outImage,
		const Size& newShape,
		const Scalar& color,
		bool auto_,
		bool scaleFill,
		bool scaleUp,
		int stride);

	void scaleCoords(const Size& imageShape, Rect& coords, const Size& imageOriginalShape);

	template <typename T>
	inline T clip(const T& n, const T& lower, const T& upper);
}

#endif // !__YOLOUTILS_H__
