#include <QLocale>
#include <QTranslator>
#include <QApplication>

#include "server.h"

#define DEBUG 1
#define __RESNET_TEST__ 1

#if DEBUG
#include "yoloDetector.h"
#endif 

int main(int argc, char *argv[])
{
#if not DEBUG
	QApplication a(argc, argv);

	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString& locale : uiLanguages) {
		const QString baseName = "server_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName)) {
			a.installTranslator(&translator);
			break;
		}
	}
	CServer w;
	return a.exec();

#elif __RESNET_TEST__
	QString nameDir = QDir::currentPath() + "/names/";
	string clsNamePath = nameDir.toStdString() + "sewer.names";

	QString imgDir = QDir::currentPath() + "/images/raw_images/";
	string imgPath = imgDir.toStdString() + "test.png";

	auto modelPath = L"D:\\qt_workspace\\yoloFileTransferSystem\\server\\weights\\model_troch_export.onnx";

	string resPath = QDir::currentPath().toStdString() + "/images/detects/";

	vector<string> labels = loadClsNames(clsNamePath);

	Mat srcImage = imread(imgPath);
	cv::cvtColor(srcImage, srcImage, COLOR_BGR2RGB);
	cv::resize(srcImage, srcImage, Size(224, 224));
	srcImage = srcImage.reshape(1, 1);

	vector<float> vec;
	srcImage.convertTo(vec, CV_32FC1, 1. / 255);
	vector<float> imageVec;
	for (size_t ch = 0; ch < 3; ++ch) 
	{
		for (size_t i = ch; i < vec.size(); i += 3) 
		{
			imageVec.emplace_back(vec[i]);
		}
	}

	Ort::Env env;
	Ort::RunOptions runOptions;
	Ort::Session session(nullptr);

	constexpr int64_t numChannels = 3;
	constexpr int64_t width = 224;
	constexpr int64_t height = 224;
	constexpr int64_t numClasses = 17;
	constexpr int64_t numInputElements = numChannels * height * width;

	session = Ort::Session(env, modelPath, Ort::SessionOptions{ nullptr });
	const std::array<int64_t, 4> inputShape = { 1, numChannels, height, width };
	const std::array<int64_t, 2> outputShape = { 1, numClasses };

	std::array<float, numInputElements> input;
	std::array<float, numClasses> results;

	auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	auto inputTensor = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), inputShape.data(), inputShape.size());
	auto outputTensor = Ort::Value::CreateTensor<float>(memory_info, results.data(), results.size(), outputShape.data(), outputShape.size());

	std::copy(imageVec.begin(), imageVec.end(), input.begin());

	// define names
	Ort::AllocatorWithDefaultOptions ort_alloc;
	const char* inputName = session.GetInputName(0, ort_alloc);
	const char* outputName = session.GetOutputName(0, ort_alloc);
	const std::array<const char*, 1> inputNames = { inputName };
	const std::array<const char*, 1> outputNames = { outputName };

	// run inference
	try 
	{
		session.Run(runOptions, inputNames.data(), &inputTensor, 1, outputNames.data(), &outputTensor, 1);
	}
	catch (Ort::Exception& e) 
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
	// sort results
	std::vector<std::pair<size_t, float>> indexValuePairs;

	for (size_t i = 0; i < results.size(); ++i) 
	{
		indexValuePairs.emplace_back(i, results[i]);
	}
	std::sort(indexValuePairs.begin(), indexValuePairs.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });

	for (size_t i = 0; i < 5; ++i) 
	{
		const auto& result = indexValuePairs[i];
		std::cout << i + 1 << ": " << labels[result.first] << " " << result.second << std::endl;
	}
#else

	QString nameDir = QDir::currentPath() + "/names/";
    string clsNamePath = nameDir.toStdString() + "coco.names";

    QString imgDir = QDir::currentPath() + "/images/raw_images/";
    string imgPath = imgDir.toStdString() + "LUD07821.jpg";

	QString weightDir = QDir::currentPath() + "/weights/";
    string weightPath = weightDir.toStdString() + "yolov5m.onnx";

    string resPath = QDir::currentPath().toStdString() + "/images/detects/";

	vector<string> classNames = loadClsNames(clsNamePath);

	Config cfg{false, 0.3f, 0.4f, weightPath, Size(640, 640)};
	Mat srcImage;
	vector<Detection> res;
	CYoloDetector* detector = nullptr;

	try
	{
		detector = new CYoloDetector(cfg);
		qDebug() << QString::fromLocal8Bit("初始化成功 ") << Qt::endl;

		try
		{
			srcImage = imread(imgPath);
			//resize(srcImage, srcImage, cv::Size(224, 224));
		}
		catch (cv::Exception& e)
		{
			qDebug() << QString::fromLocal8Bit("出现错误: ") << e.what() << Qt::endl;
			return -1;
		}

		res = detector->detect(srcImage);
	}
	catch (const std::exception& e)
	{
		qDebug() << QString::fromLocal8Bit("出现错误: ") << e.what() << Qt::endl;
		return -1;
	}

	visualizeDetection(srcImage, res, classNames);
	cv::imwrite(resPath + "res.jpg", srcImage);
	cv::waitKey(0);
	return 0;
#endif
}
