#include "yoloDetector.h"

CYoloDetector::CYoloDetector(Config& config, QObject* parent /*= nullptr*/)
	: QObject(parent)
	, m_isGPU(config.s_isGPU)
	, m_confThreshold(config.s_confThreshold)
	, m_iouThreshold(config.s_iouThreshold)

{
	m_env = Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "ONNX_DETECTION");
	m_sessionOptions = Ort::SessionOptions();

	vector<string> availableProviders = Ort::GetAvailableProviders();
	auto cudaAvailable = std::find(availableProviders.begin(), availableProviders.end(), "CUDAExecutionProvider");
	OrtCUDAProviderOptions cudaOption;
	if (m_isGPU && (cudaAvailable == availableProviders.end()))
	{
		qDebug() << QString::fromLocal8Bit("不支持GPU加速 ");
		qDebug() << QString::fromLocal8Bit("使用设备：CPU ");
	}
	else if (m_isGPU && (cudaAvailable != availableProviders.end()))
	{
		qDebug() << QString::fromLocal8Bit("使用设备：GPU ");
		m_sessionOptions.AppendExecutionProvider_CUDA(cudaOption);
	}
	else
	{
		qDebug() << QString::fromLocal8Bit("使用设备：CPU ");
	}

#ifdef _WIN32
	wstring w_modelPath = stringToWstring(config.s_weightPath);
	m_session = Ort::Session(m_env, w_modelPath.c_str(), m_sessionOptions);
#else
	m_session = Ort::Session(m_env, config.s_weightPath.c_str(), m_sessionOptions);
#endif

	Ort::AllocatorWithDefaultOptions allocator;
	Ort::TypeInfo inputTypeInfo = m_session.GetInputTypeInfo(0);
	vector<int64_t> inputTensorShape = inputTypeInfo.GetTensorTypeAndShapeInfo().GetShape();
	m_isDynamicInputShape = false;
	// checking if width and height are dynamic
	if (inputTensorShape[2] == -1 && inputTensorShape[3] == -1)
	{
		qDebug() << QString::fromLocal8Bit("Dynamic input shape");
		m_isDynamicInputShape = true;
	}

	m_vecInputName.emplace_back(m_session.GetInputName(0, allocator));
	m_vecOutputName.emplace_back(m_session.GetOutputName(0, allocator));

	m_inputImgShape = Size2f(config.s_inputSize);
}

CYoloDetector::~CYoloDetector()
{
	m_vecInputName.clear();
	m_vecOutputName.clear();
}

vector<Detection> CYoloDetector::detect(Mat& srcImage)
{
	float* blob = nullptr;
	std::vector<int64_t> inputTensorShape{ 1, 3, -1, -1 };
	this->preprocessing(srcImage, blob, inputTensorShape);

	size_t inputTensorSize = vecProductSize(inputTensorShape);

	std::vector<float> inputTensorValues(blob, blob + inputTensorSize);

	std::vector<Ort::Value> inputTensors;

	Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
		OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

	inputTensors.push_back(Ort::Value::CreateTensor<float>(
		memoryInfo, inputTensorValues.data(), inputTensorSize,
		inputTensorShape.data(), inputTensorShape.size()
		));

	std::vector<Ort::Value> outputTensors = m_session.Run(Ort::RunOptions{ nullptr },
		m_vecInputName.data(),
		inputTensors.data(),
		1,
		m_vecOutputName.data(),
		1);

	cv::Size resizedShape = cv::Size((int)inputTensorShape[3], (int)inputTensorShape[2]);

	std::vector<Detection> result = this->postprocessing(resizedShape,
		srcImage.size(), outputTensors);

	delete[] blob;

	return result;
}

wstring CYoloDetector::stringToWstring(const string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

void CYoloDetector::preprocessing(Mat& srcImage, float*& blob, vector<int64_t>& inputTensorShape)
{
	Mat resizedImage, floatImage;
	cvtColor(srcImage, resizedImage, COLOR_BGR2RGB);
	letterbox(resizedImage, resizedImage, m_inputImgShape,
		Scalar(114, 114, 114), m_isDynamicInputShape,
		false, true, 32);

	inputTensorShape[2] = resizedImage.rows;
	inputTensorShape[3] = resizedImage.cols;

	resizedImage.convertTo(floatImage, CV_32FC3, 1 / 255.0);
	blob = new float[floatImage.cols * floatImage.rows * floatImage.channels()];
	Size floatImageSize{ floatImage.cols, floatImage.rows };

	// hwc -> chw
	vector<Mat> chw(floatImage.channels());
	for (int i = 0; i < floatImage.channels(); ++i)
	{
		chw[i] = Mat(floatImageSize, CV_32FC1, blob + i * floatImageSize.width * floatImageSize.height);
	}
	split(floatImage, chw);
}

vector<Detection> CYoloDetector::postprocessing(const Size& resizedImageShape, 
	const Size& originalImageShape, vector<Ort::Value>& outputTensors)
{
	vector<Rect> boxes;
	vector<float> confs;
	vector<int> classIds;

	auto* rawOutput = outputTensors[0].GetTensorData<float>();
	vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
	size_t count = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
	vector<float> output(rawOutput, rawOutput + count);

	// first 5 elements are box[4] and obj confidence
	int numClasses = (int)outputShape[2] - 5;
	int elementsInBatch = (int)(outputShape[1] * outputShape[2]);

	// only for batch size = 1
	for (auto iter_output = output.begin(); iter_output != output.begin() + elementsInBatch; iter_output += outputShape[2])
	{
		float clsConf = iter_output[4];

		if (clsConf > m_confThreshold)
		{
			int centerX = (int)(iter_output[0]);
			int centerY = (int)(iter_output[1]);
			int width = (int)(iter_output[2]);
			int height = (int)(iter_output[3]);
			int left = centerX - width / 2;
			int top = centerY - height / 2;

			float objConf;
			int classId;
			this->getBestClassInfo(iter_output, numClasses, objConf, classId);

			float confidence = clsConf * objConf;

			boxes.emplace_back(left, top, width, height);
			confs.emplace_back(confidence);
			classIds.emplace_back(classId);
		}
	}

	vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confs, m_confThreshold, m_iouThreshold, indices);
	// std::cout << "amount of NMS indices: " << indices.size() << std::endl;

	vector<Detection> detections;

	for (int idx : indices)
	{
		Detection det;
		det.s_box = cv::Rect(boxes[idx]);
		scaleCoords(resizedImageShape, det.s_box, originalImageShape);

		det.s_confidence = confs[idx];
		det.s_clsID = classIds[idx];
		detections.emplace_back(det);
	}

	return detections;
}

void CYoloDetector::getBestClassInfo(vector<float>::iterator it, const int& numClasses, 
	float& bestConf, int& bestClassId)
{
	// 前5个检测到的目标进行标注
	bestClassId = 5;
	bestConf = 0;

	for (int i = 5; i < numClasses + 5; i++)
	{
		if (it[i] > bestConf)
		{
			bestConf = it[i];
			bestClassId = i - 5;
		}
	}

}
