#include "resTransferSender.h"

#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
#include <QDebug>

CResTransSender::CResTransSender(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, m_isCancel(false)
	, m_isRunning(false)
	, m_nextBlockSize(0)
	, m_recvDataSize(0)
	, m_timeoutMsec(3000)
	, m_sendSize(0)
	, m_fileSize(0)
	, m_detector(nullptr)
{
	m_detectResVec.clear();
	// <============= 原始图片路径 =============>
	setRawImageDir(g_vecDirPath.at(0));
	// <============ 处理完成后备份图片路径 ==============>
	setBakImageDir(g_vecDirPath.at(1));
	// <============ 权重路径 ==============>
	setWeightDir(g_vecDirPath.at(2));
	m_weightPath = m_weightDirPath.toStdString() + "yolov5m.onnx";
	// <============ 处理结果路径 ==============>
	setResultDir(g_vecDirPath.at(3));
	// <============ 类别名称路径 ==============>
	if (setClsNameDir(g_vecDirPath.at(4)))
	{
		// <---------------- 后续修改类别名称文件
		m_clsNamePath = m_clsNameDirPath.toStdString() + "coco.names";
		m_clsNameVec = loadClsNames(m_clsNamePath);	// 加载类别名称
	}

	// <============ onnx配置结构 ==============> <---- 后期写入文本配置文件内
	m_config = { false, 0.3f, 0.4f, m_weightPath, Size(640, 640) };

	m_commSocket = new QTcpSocket(this);
	m_pool.setMaxThreadCount(1);
}

CResTransSender::~CResTransSender()
{
	m_lstFile.clear();
	m_detectResVec.clear();
	m_clsNameVec.clear();
}

void CResTransSender::onReadComm()
{
	QDataStream out(m_commSocket);
	out.setVersion(QDataStream::Qt_5_9);
	if (m_nextBlockSize == 0)
	{
		if (m_commSocket->bytesAvailable() >= sizeof(qint64))
		{
			out >> m_nextBlockSize;
		}
	}
	if (m_commSocket->bytesAvailable() >= m_nextBlockSize)
	{
		QByteArray array;
		out >> array;
		m_cacheData = QString(array);
		dealReadComm();
		m_cacheData.clear();
		m_nextBlockSize = 0;
	}
}

void CResTransSender::moveFile(const bool& isCover)
{
	QFileInfo info(m_fileName);
	QString fileName = info.fileName();

	QString dstFilePath = m_bakImgDir + fileName;
	qDebug() << QString::fromLocal8Bit("移动文件：%1至%2 ").arg(fileName).arg(dstFilePath);

	// 允许覆盖，如果文件存在，则删除目标路径文件
	if (isCover)
	{
		QFile dstFile(dstFilePath);
		if (dstFile.exists())
		{
			if (!dstFile.remove())
			{
				qDebug() << QString::fromLocal8Bit("删除已存在图片失败 ");
			}
		}
	}
	m_file.rename(dstFilePath);
}

void CResTransSender::dealReadComm()
{
	auto code = (SEND_TYPE)m_cacheData.section(SPLIT_TYPE_INFO_MAEK, 0, 0).toInt();
	int ret = m_cacheData.section(SPLIT_TYPE_INFO_MAEK, 1, 1).toInt();
	QString additionalInfo = m_cacheData.section(SPLIT_TYPE_INFO_MAEK, 2, 2);
	switch (code)
	{
	case SEND_TYPE::FILE_HEAD_REC_CODE: // 发送文件头响应处理
	{
		if (ret)
		{
			quitFileTransferThread();
			m_resTransFuture = QtConcurrent::run(&m_pool, [this]() {send_file(); });
			m_isRunning = true;
			m_cancelFileTransMutex.lock();
			m_isCancel = false;
			m_cancelFileTransMutex.unlock();
		}
		else
		{
			qDebug() << ERROR_CODE_4;
		}
		break;
	}
	case SEND_TYPE::FILE_REC_CODE: // 接收文件数据完成响应处理
	{
		if (ret)
		{
			bool isOk;
			qint64 recv = additionalInfo.toLongLong(&isOk, 10);
			if (m_fileSize > 0)
			{
				int progressVal = (recv * 100) / m_fileSize;
				emit progressValue(m_fileName, progressVal);
			}
			qDebug() << QString::fromLocal8Bit("文件大小: ") << m_fileSize / 1024 << QString::fromLocal8Bit("KB,已接收数据大小: ") << recv / 1024 << "KB";
		}
		else
		{
			qDebug() << ERROR_CODE_5;
		}
		break;
	}
	case SEND_TYPE::FILE_CANCEL:
	{
		m_cancelFileTransMutex.lock();
		m_isCancel = true;
		m_cancelFileTransMutex.unlock();
		m_sendSize = 0;
		if (m_file.isOpen())
		{
			m_file.close();
		}
		emit progressValue(m_fileName, 0);
		emit cancelSendFileRspSig((int)code, QVariant(ret));
		break;
	}
	case SEND_TYPE::FILE_TRANSFER_TIMEOUT_CODE:
	{
		errorState(ERROR_CLASSIFY_EM::K_RESPONSE_TIMEOUT, K_RESPONSE_TIMEOUT_EM::K_RESPONSE_FILE_TRANSFER_TIMEOUT);
		break;
	}
	case SEND_TYPE::FILE_TRANSFER_FINISH_CODE:
	{
		emit fileTransferFinish();
		qDebug() << __FUNCTION__ << m_fileName << QString::fromLocal8Bit("文件传输完成 ");
		break;
	}
	default:
		break;
	}
}

void CResTransSender::quitFileTransferThread()
{
	while (m_isRunning)
	{
		m_cancelFileTransMutex.lock();
		m_isCancel = true;
		m_cancelFileTransMutex.unlock();
		QThread::msleep(10);
	}
}

void CResTransSender::send_file()
{
	qint64 len = 0;
	QTcpSocket fileSocket;
	fileSocket.connectToHost(QHostAddress(IP_ADDR), PORT_RESFILE);
	bool ret = fileSocket.waitForConnected(m_timeoutMsec);
	if (ret)
	{
		connect(&fileSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onFileError(QAbstractSocket::SocketError)));
	}
	else
	{
		emit errorState(ERROR_CLASSIFY_EM::K_NETWORK_ERROR, K_NETWORK_ERROR_EM::K_NETWORK_DISCONNECTED_FILE_ERROR);
	}

	if (!m_file.isOpen())
	{
		bool isOk = m_file.open(QIODevice::ReadOnly);
		if (isOk)
		{
			qDebug() << QString::fromLocal8Bit("开始发送文件数据信息 ") << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
		}
		else
		{
			qDebug() << ERROR_CODE_2;
		}
	}

#if 1
	do
	{
		QByteArray buf;
		buf = m_file.read(SEND_BLOCK_SIZE);
		len = fileSocket.write(buf);
		fileSocket.waitForBytesWritten();
		m_sendSize += len;

		m_cancelFileTransMutex.lock();
		if (m_isCancel)
		{
			m_cancelFileTransMutex.unlock();
			break;
		}
		m_cancelFileTransMutex.unlock();

		qDebug() << QString::fromLocal8Bit("====>发送的数据数据大小:") << len / 1024 << "KB" << QString::fromLocal8Bit("\t已发送数据大小:") << m_fileSize / 1024 << "KB";

	} while (len > 0);
#endif

#if 0
	QByteArray buf;
	buf = m_file.readAll();
	len = fileSocket.write(buf);
	fileSocket.waitForBytesWritten();
	m_sendSize = len;
#endif

	if (m_sendSize == m_fileSize)
	{
		moveFile(true);		// <======= 添加移动文件函数入口 ========>
		m_sendSize = 0;
		m_file.close();
		fileSocket.disconnect();
		fileSocket.close();
		qDebug() << QString::fromLocal8Bit("文件 ") << m_file.fileName() << QString::fromLocal8Bit("发送完毕 ")
			<< QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
	}
	m_isRunning = false;
}

void CResTransSender::onFileError(QAbstractSocket::SocketError socketError)
{
	qDebug() << __FUNCTION__ << socketError;
}

void CResTransSender::onCommandError(QAbstractSocket::SocketError)
{
	qDebug() << __FUNCTION__ << m_commSocket->errorString();
}

void CResTransSender::setRawImageDir(const QString& dirPath)
{
	if (dirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
	}
	QDir tmpDir(dirPath);
	if (!tmpDir.exists())
	{
		qDebug() << __FUNCTION__ << "读取原始图片路径失败 ";
		return;
	}
	m_rawImgDir = dirPath;
	clearDir(m_rawImgDir);
}

QString CResTransSender::getRawImageDir() const
{
	return this->m_rawImgDir;
}

void CResTransSender::setBakImageDir(const QString& dirPath)
{
	if (dirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
	}
	QDir tmpDir(dirPath);
	if (!tmpDir.exists())
	{
		tmpDir.mkdir(dirPath);
		qDebug() << __FUNCTION__ << "已设置备份图片路径 ";
	}
	m_bakImgDir = dirPath;
}

void CResTransSender::setResultDir(const QString& dirPath)
{
	if (dirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
	}
	QDir tmpDir(dirPath);
	if (!tmpDir.exists())
	{
		tmpDir.mkdir(dirPath);
		qDebug() << __FUNCTION__ << "已设置备份图片路径 ";
	}
	m_resultDirPath = dirPath;
	clearDir(m_resultDirPath);
}

void CResTransSender::setWeightDir(const QString& dirPath)
{
	if (dirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
	}
	QDir tmpDir(dirPath);
	if (!tmpDir.exists())
	{
		qDebug() << __FUNCTION__ << "读取onnx权重路径失败 ";
		return;
	}
	m_weightDirPath = dirPath;
}

bool CResTransSender::setClsNameDir(const QString& dirPath)
{
	if (dirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
		return false;
	}
	QDir tmpDir(dirPath);
	if (!tmpDir.exists())
	{
		qDebug() << __FUNCTION__ << "读取类别名称路径失败 ";
		return false;
	}
	m_clsNameDirPath = dirPath;
	return true;
}

bool CResTransSender::setFileInfo()
{
	// <---------- 目录不存在报错
	if (m_resultDirPath.isEmpty())
	{
		qDebug() << ERROR_CODE_1;
	}
	// <========== 检测结果锁
	m_detectMutex.lock();
	QDir detectedDir(m_resultDirPath);
	m_lstFile = detectedDir.entryInfoList(QDir::Files);
	m_detectMutex.unlock();

#if 0
	/*<-------- 测试目录读取 ---------->*/
	foreach(auto & info, m_lstFile)
	{
		qDebug() << info.absoluteFilePath() << Qt::endl;
	}
#endif

	return true;
}

bool CResTransSender::imageDetection()
{
	try
	{
		m_detector = new CYoloDetector(m_config);
		qDebug() << QString::fromLocal8Bit("ONNX检测器初始化成功 ") << Qt::endl;

		if (m_rawImgDir.isEmpty())
		{
			qDebug() << ERROR_CODE_1;
		}
		QDir rawDir(m_rawImgDir);
		QFileInfoList fileList = rawDir.entryInfoList(QDir::Files);
		// <------------ 循环处理源图片文件夹下图片
		for (QFileInfo& info : fileList)
		{
			// 原始图片文件
			QFile rawImg(info.absoluteFilePath());
			// 文件绝对路径
			string imgABSName = info.absoluteFilePath().toStdString();
			// 检测标注后图片绝对路径
			string dstImgName = m_resultDirPath.toStdString() + info.fileName().toStdString();

			m_detectMutex.lock();
			try
			{
				m_srcImage = imread(imgABSName);
				m_detectResVec = m_detector->detect(m_srcImage);
				visualizeDetection(m_srcImage, m_detectResVec, m_clsNameVec);
				// <============ 自适应比例缩放写入图片
				double w = m_srcImage.cols / 1.f;
				double h = m_srcImage.rows / 1.f;
				Size resizeScale(640, 640);
				double imgScale = w >= h ? (h / w) : (w / h);
				w >= h ? resizeScale.height *= imgScale : resizeScale.width *= imgScale;

				resize(m_srcImage, m_srcImage, resizeScale);
				imwrite(dstImgName, m_srcImage);
				rawImg.remove();
				m_detectMutex.unlock();
			}
			catch (const cv::Exception& e)
			{
				m_detectMutex.unlock();
				qDebug() << QString::fromLocal8Bit("图片读取出现错误: ") << e.what() << Qt::endl;
				continue;
			}
		}
	}
	catch (const std::exception& e)
	{
		qDebug() << QString::fromLocal8Bit("出现错误: ") << e.what() << Qt::endl;
		return false;
	}
	return true;
}

bool CResTransSender::sendCommand(SEND_TYPE type, QVariant additionalInfo /*= ""*/)
{
	QString comm;
	switch (type)
	{
	case SEND_TYPE::FILE_HEAD_CODE:
		comm = QString("%1" + QString(SPLIT_TYPE_INFO_MAEK) + "%2").arg((int)type).arg(m_fileName + SPLIT_ADDITION_INFO_MARK + QString::number(m_fileSize));
		break;
	case SEND_TYPE::FILE_CANCEL:
		comm = QString("%1" + QString(SPLIT_TYPE_INFO_MAEK) + "%2").arg((int)type).arg("");
		break;
	case SEND_TYPE::FILE_CODE:
		comm = QString("%1" + QString(SPLIT_TYPE_INFO_MAEK) + "%2").arg((int)type).arg("");
		break;
	default:
		break;
	}
	qint64 len = m_commSocket->write(comm.toUtf8().data());
	m_commSocket->waitForBytesWritten();
	if (len <= 0)
	{
		qDebug() << QString::fromLocal8Bit("命令代号: ") << (int)type << ERROR_CODE_3;
		m_file.close();
		return false;
	}
	return true;
}

void CResTransSender::clearDir(const QString& dirPath)
{
	QDir tmpDir(dirPath);
	if (!tmpDir.isEmpty())
	{
		QDirIterator dirIter(m_resultDirPath, QDir::Files, QDirIterator::NoIteratorFlags);
		while (dirIter.hasNext())
		{
			if (tmpDir.remove(dirIter.next()))
			{
				QDir(dirIter.filePath()).removeRecursively();
			}
		}
	}
	return;
}

void CResTransSender::connectToServer()
{
	m_commSocket->connectToHost(IP_ADDR, PORT_RESCOMM);

	bool isConnect = m_commSocket->waitForConnected(m_timeoutMsec);
	if (isConnect)
	{
		connect(m_commSocket, SIGNAL(readyRead()), this, SLOT(onReadComm()));
		connect(m_commSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onCommandError(QAbstractSocket::SocketError)));
		qDebug() << (isConnect ? QString::fromLocal8Bit("<============连接结果接收端成功=============> ") : QString::fromLocal8Bit("连接结果接收端失败 "));
	}
	else
	{
		emit errorState(ERROR_CLASSIFY_EM::K_NETWORK_ERROR, K_NETWORK_ERROR_EM::K_NETWORK_DISCONNECTED_COMMAND_ERROR);
	}
}

/**
* @brief: 发送经过检测处理完成后的图片
* @param: 
*/
void CResTransSender::sendFile()
{
	// <==================== 图片处理入口 ====================>
	if (!imageDetection())
	{
		qDebug() << QString::fromLocal8Bit("检测图片失败 ") << Qt::endl;
		return;
	}
	// <==================== 加载图片文件信息入口 ====================>
	if (!setFileInfo())
	{
		qDebug() << QString::fromLocal8Bit("加载图片文件信息失败 ") << Qt::endl;
		return;
	}
	for (QFileInfo& info : m_lstFile/* /detects/ */)
	{
		// <---------- 暂时修改为读取一个文件、读取前清空之前文件
		// @date: 2023/1/16
		m_fileName = info.absoluteFilePath();
		m_fileSize = info.size();
		m_sendSize = 0;

		if (0 == info.size())
		{
			emit progressValue(m_fileName, 100);
			emit fileTransferFinish();
		}
		if (m_file.isOpen())
		{
			m_file.close();
		}
		m_file.setFileName(m_fileName);
		bool isOk = m_file.open(QIODevice::ReadOnly);
		if (isOk == false)
		{
			qDebug() << ERROR_CODE_2 << Qt::endl;
			qDebug() << QString::fromLocal8Bit("发送文件失败: %1").arg(m_fileName) << Qt::endl;
			continue;
		}
		if (sendCommand(SEND_TYPE::FILE_HEAD_CODE))
		{
			QtConcurrent::run(&m_pool, [this]() {send_file(); });
		}
	}

}
