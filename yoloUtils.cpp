#include "yoloUtils.h"

size_t yoloUtils::vecProductSize(const vector<int64_t>& vec)
{
	if (vec.empty())
	{
		return 0;
	}
	size_t res = 1;
	for (const int64_t& item : vec)
	{
		res *= item;
	}
	return res;
}

vector<string> yoloUtils::loadClsNames(const string& path)
{
	vector<string> vecClsName;
	ifstream inFile(path);
	if (inFile.is_open())
	{
		string line;
		while (getline(inFile, line))
		{
			if (line.back() == '\r')
			{
				line.pop_back();
			}
			vecClsName.emplace_back(line);
		}
	}
	else
	{
		std::cerr << "ERROR: Failed to access class name path: " << path << std::endl;
	}
	return vecClsName;
}

void yoloUtils::visualizeDetection(Mat& srcImage, vector<Detection>& vecDetection, 
	const vector<std::string>& vecClsName)
{
	// <------------- 设置文本缩放比例
	double fontScale = 1.0f;
	double scaleFlag = min(srcImage.cols, srcImage.rows) / 1000.0f;
	for (double scale : g_scaleVec)
	{
		if (scaleFlag < scale)
		{
			fontScale *= scale;
			break;
		}
	}

	for (const Detection& detection : vecDetection)
	{
		int x = detection.s_box.x;
		int y = detection.s_box.y;

		int conf = (int)round(detection.s_confidence * 100);
		int clsId = detection.s_clsID;
		string label = vecClsName[clsId] + "0." + std::to_string(conf);

		int baseline = 0;
	
		Size textSize = getTextSize(label, FONT_ITALIC, fontScale, 3, &baseline);
		// <========= 调整文本标签
		rectangle(srcImage,
			Point(x, y), Point(x + textSize.width, y + textSize.height),
			Scalar(299, 160, 21), -1);
		putText(srcImage, label,
			Point(x, y + textSize.height - 3), FONT_ITALIC, fontScale,
			Scalar(255, 255, 255), 3);
	}
}

void yoloUtils::letterbox(const Mat& srcImage, Mat& outImage, const Size& newShape, 
	const Scalar& color, bool auto_, bool scaleFill, bool scaleUp, int stride)
{
	Size shape = srcImage.size();
	float r = min((float)newShape.height / (float)shape.height,
		(float)newShape.width / (float)shape.width);
	
	if (!scaleUp)
	{
		r = min(r, 1.0f);
	}

	float ratio[2]{ r, r };
	int newUnpad[2]{ (int)round((float)shape.width * r),
					 (int)round((float)shape.height * r) };

	auto dw = (float)(newShape.width - newUnpad[0]);
	auto dh = (float)(newShape.height - newUnpad[1]);

	if (auto_)
	{
		dw = (float)((int)dw % stride);
		dh = (float)((int)dh % stride);
	}
	else if (scaleFill)
	{
		dw = 0.0f;
		dh = 0.0f;
		newUnpad[0] = newShape.width;
		newUnpad[1] = newShape.height;
		ratio[0] = (float)newShape.width / (float)shape.width;
		ratio[1] = (float)newShape.height / (float)shape.height;
	}

	dw /= 2.0f;
	dh /= 2.0f;

	if (shape.width != newUnpad[0] && shape.height != newUnpad[1])
	{
		resize(srcImage, outImage, cv::Size(newUnpad[0], newUnpad[1]));
	}

	int top = int(round(dh - 0.1f));
	int bottom = int(round(dh + 0.1f));
	int left = int(round(dw - 0.1f));
	int right = int(round(dw + 0.1f));
	copyMakeBorder(outImage, outImage, top, bottom, left, right, BORDER_CONSTANT, color);
}

void yoloUtils::scaleCoords(const Size& imageShape, Rect& coords, const Size& imageOriginalShape)
{
	float gain = min((float)imageShape.height / (float)imageOriginalShape.height,
		(float)imageShape.width / (float)imageOriginalShape.width);

	int pad[2] = { (int)(((float)imageShape.width - (float)imageOriginalShape.width * gain) / 2.0f),
				  (int)(((float)imageShape.height - (float)imageOriginalShape.height * gain) / 2.0f) };

	coords.x = (int)round(((float)(coords.x - pad[0]) / gain));
	coords.y = (int)round(((float)(coords.y - pad[1]) / gain));

	coords.width = (int)round(((float)coords.width / gain));
	coords.height = (int)round(((float)coords.height / gain));
}

template<typename T>
T yoloUtils::clip(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}