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
	int s_clsID;			// ������id
	float s_confidence;	// ������Ŷ�
	cv::Rect s_box;		// ���ο�
};

struct Config
{
	bool s_isGPU;				// �Ƿ��Կ�����
	float s_confThreshold;		// ���Ŷ���ֵ
	float s_iouThreshold;		// �Ǽ���ֵ������ֵ
	string s_weightPath;		// YOLOv5ģ��·��
	//string s_classNamePath;		// ��������ļ�·��
	Size s_inputSize;			// ͼƬ��С���

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