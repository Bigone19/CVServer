#pragma once
#ifndef __YOLOCONFIG_H__
#define __YOLOCONFIG_H__

#if 0
#undef slots
#include <torch/torch.h>
#include <torch/script.h>
#define slots Q_SLOTS
#endif

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <string>

using namespace cv;
using std::string;

struct Detection {
	int s_clsID;			// 结果类别id
	float s_confidence;	// 结果置信度
	cv::Rect s_box;		// 矩形框
};

struct Config
{
	bool s_isGPU;				// 是否显卡运行
	float s_confThreshold;		// 置信度阈值
	float s_iouThreshold;		// 非极大值抑制阈值
	string s_weightPath;		// YOLOv5模型路径
	//string s_classNamePath;		// 类别名称文件路径
	Size s_inputSize;			// 图片大小规格

	Config& operator=(const Config& cfg)
	{
		this->s_isGPU = cfg.s_isGPU;
		this->s_confThreshold = cfg.s_confThreshold;
		this->s_iouThreshold = cfg.s_iouThreshold;
		this->s_weightPath = cfg.s_weightPath;
		this->s_inputSize = cfg.s_inputSize;
		return *this;
	}
};

#endif // !__YOLOCONFIG_H__