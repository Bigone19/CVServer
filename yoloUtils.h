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
	// 文本缩放比例
	const vector<double> g_scaleVec = { 0.32, 0.64, 1, 1.5, 2, 3, 4, 5, 6 };
	/**
	* @brief: 获取传感器大小
	* @param: 
	*/
	size_t vecProductSize(const vector<int64_t>& vec);
	/**
	* @brief: 加载检测类名
	* @param: 
	*/
	vector<string> loadClsNames(const string& path);
	/**
	* @brief: 检测结果可视化
	* @param: 
	*/
	void visualizeDetection(Mat& srcImage, vector<Detection>& vecDetection,
		const vector<std::string>& vecClsName);
	/**
	* @brief: 绘制文本标签
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
