#pragma once
#ifndef __YOLODETECTOR_H__
#define __YOLODETECTOR_H__

#include <QObject>
#include <QDebug>
#include <onnxruntime_cxx_api.h>
#include <utility>
#include <locale>
#include <codecvt>

#include "yoloUtils.h"

using namespace yoloUtils;

class CYoloDetector : public QObject
{
	Q_OBJECT
public:
	CYoloDetector(Config& config, QObject* parent = nullptr);
	~CYoloDetector();
	/**
	* @brief: Ŀ����
	* @param: 
	*/
	vector<Detection> detect(Mat& srcImage);
	/**
	* @brief: string ת wstring
	* @param:
	*/
	inline wstring stringToWstring(const string& str);

private:
	/**
	* @brief: ͼƬת����
	* @param: 
	*/
	void preprocessing(Mat& srcImage, float*& blob, vector<int64_t>& inputTensorShape);
	/**
	* @brief: �ύ�����
	* @param: 
	*/
	vector<Detection> postprocessing(const Size& resizedImageShape,
		const Size& originalImageShape,
		vector<Ort::Value>& outputTensors);
	/**
	* @brief: ��ȡ��������Ϣ
	* @param: 
	*/
	static void getBestClassInfo(vector<float>::iterator it, const int& numClasses,
		float& bestConf, int& bestClassId);

private:
	float m_confThreshold;
	float m_iouThreshold;
	bool m_isGPU;

	vector<const char*> m_vecInputName;
	vector<const char*> m_vecOutputName;

	Ort::Env m_env;
	Ort::SessionOptions m_sessionOptions;
	Ort::Session m_session;

	bool m_isDynamicInputShape;
	Size2f m_inputImgShape;
};

#endif // !__YOLODETECTOR_H__
