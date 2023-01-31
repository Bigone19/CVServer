#pragma once
#ifndef __RESTRANSFERSENDER_H__
#define __RESTRANSFERSENDER_H__

#include "yoloDetector.h"
#include "fileUtils.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

using std::vector;

// <============ 文件夹路径初始化 ==============>
const string g_initFilePath = g_projectPath.toStdString() + "/init_dir.cfg";
const vector<QString> g_vecDirPath = fileUtils::loadInitFile(g_initFilePath);

class CResTransSender : public QObject
{
	Q_OBJECT
public:
	CResTransSender(QObject* parent = nullptr);
	~CResTransSender();
	/**
	* @brief: 发送文件
	* @param:
	*/
	void sendFile();
	/**
	* @brief: 连接到服务端
	* @param: 
	*/
	void connectToServer();
	/**
	* @brief:  获取原始图片文件夹路径
	* @param:
	*/
	QString getRawImageDir() const;


signals:
	/**
	* @brief: 错误状态
	* @param:
	*/
	void errorState(int code, int subCode);
	/**
	* @brief: 文件传输完成信号
	* @param:
	*/
	void fileTransferFinish();
	/**
	* @brief: 取消文件传输响应信号
	* @param:
	*/
	void cancelSendFileRspSig(int type, QVariant retValue);
	/**
	* @brief: 服务端已接受的数据进度
	* @param:
	*/
	void progressValue(const QString& fileName, int progressVal);

private slots:
	/**
	* @brief: 响应服务端信息
	* @param:
	*/
	void onReadComm();
	/**
	* @brief: 文件socket错误
	* @param:
	*/
	void onFileError(QAbstractSocket::SocketError);
	/**
	* @brief: 命令socket错误
	* @param:
	*/
	void onCommandError(QAbstractSocket::SocketError);

private:
	/**
	* @brief: 移动文件
	* @param isCover:  如果为 true，则将以覆盖的方式复制、移动文件，
	*				   即若目标文件存在，则会先删除后复制、移动。
	*/
	void moveFile(const bool& isCover);
	/**
	* @brief: 处理命令
	* @param: 
	*/
	void dealReadComm();
	/**
	* @brief: 退出文件传输线程
	* @param:
	*/
	void quitFileTransferThread();
	/**
	* @brief: 发送文件内容数据
	* @param:
	*/
	void send_file();
	/**
	* @brief: 设置原始图片路径
	* @param:
	*/
	void setRawImageDir(const QString& dirPath);
	/**
	* @brief: 设置图片备份目录
	* @param: 
	*/
	void setBakImageDir(const QString& dirPath);
	/**
	* @brief: 设置图片处理结果目录
	* @param: 
	*/
	void setResultDir(const QString& dirPath);
	/**
	* @brief: 设置图权重目录
	* @param: 
	*/
	void setWeightDir(const QString& dirPath);
	/**
	* @brief: 设置类别名称录
	* @param: 
	*/
	bool setClsNameDir(const QString& dirPath);
	/**
	* @brief: 配置发送文件
	* @param:
	*/
	bool setFileInfo();
	/**
	* @brief: 图片处理
	* @param: 
	*/
	bool imageDetection();
	/**
	* @brief: 发送文件信息命令
	* @param:
	*/
	bool sendCommand(SEND_TYPE type, QVariant additionalInfo = "");
	/**
	* @brief: 清空文件夹
	* @param: 
	*/
	void clearDir(const QString& dirPath);
private:

	QString m_ipAddress;
	QTcpSocket* m_commSocket;

	QString m_rawImgDir;		// 原始图片路径
	QString m_bakImgDir;		// 上传完毕后移动图片到该目录下
	QFileInfoList m_lstFile;	//<------- 后续可能需要修改容器类型
	QString m_fileName;
	qint64 m_fileSize;
	QFile m_file;

	qint64 m_sendSize;
	QThreadPool m_pool;
	QFuture<void> m_resTransFuture;
	QMutex m_cancelFileTransMutex;
	bool m_isCancel;
	bool m_isRunning;

	qint64 m_nextBlockSize;	// 下一块数据大小
	qint64 m_recvDataSize;	// 已接受数据大小
	QString m_cacheData;	// 缓存数据容器

	int m_timeoutMsec;		// 超时时间

	// <==================== ONNX ======================>
	Config m_config;			// onnx检测配置结构体
	string m_weightPath;		// 权重位置
	QString m_weightDirPath;	// 权重目录位置
	QString m_resultDirPath;	// 处理结果目录路径
	string m_clsNamePath;		// 类别名称路径
	QString m_clsNameDirPath;	// 类别名称目录路径

	CYoloDetector* m_detector;			// onnx检测器
	Mat m_srcImage;						// 原始检测图片
	vector<Detection> m_detectResVec;	// 检测结果
	vector<string> m_clsNameVec;		// 检测类别名称
	QMutex m_detectMutex;				// 检测结果互斥锁
};


#endif // !__RESTRANSFERSENDER_H__