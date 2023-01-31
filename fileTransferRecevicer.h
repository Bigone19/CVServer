#pragma once
#ifndef FILETRANSFERRECEVICER_H
#define FILETRANSFERRECEVICER_H

#include "config.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

class CResTransSender;
class CFileTransRecevicer : public QObject
{
	Q_OBJECT
public:
	/**
	* @brief: 发送命令
	* @param: 
	*/
	void send_command(SEND_TYPE code, int ret, QString additional = "");
	static CFileTransRecevicer* GetInstance()
	{
		if (m_recevicer == nullptr)
		{
			m_recevicer = new CFileTransRecevicer;
		}
		return m_recevicer;
	}

public slots:
	void progress();
	void timeOutCheck();
	void on_connect_c();
	void on_connect_f();

	void on_read_command();
	void on_read_file();
	void onFileError(QAbstractSocket::SocketError);
	void onCommandError(QAbstractSocket::SocketError);

private:
	void beginSave();
	void quitFileTransferThread();
	void setCacheDir(const QString& dir);
	CFileTransRecevicer(QObject* parent = nullptr);
	~CFileTransRecevicer();

private:
	static CFileTransRecevicer* m_recevicer;

	QTcpServer* m_tcpServerComm;
	QTcpServer* m_tcpServerFile;
	QTcpSocket* m_commSocket;
	QTcpSocket* m_fileSocket;

	QFile m_file;
	QString m_absFilePathInHost;	// 文件在主机中的路径，包含文件名
	QString m_fileName;
	qint64 m_fileSize;
	qint64 m_recvSize;				// 接收数据大小
	QString m_fileCacheDir;			// 文件缓存路径

	bool m_isCancel;
	QDateTime m_startTime;			// 传输开始时间记录对象
	bool m_startTimeFlag;			// 传输开始时间记录标识
	QTimer* m_timerProgress;		// 定时获取已接收数据大小定时器
	bool m_isRunning;
	QByteArrayList m_cacheList;		// 接收数据缓存队列
	QReadWriteLock m_cacheListLock;	// 接收数据缓存队列读写锁

	QTimer* m_timeOutProgress;		// 传输超时监测定时器
	qint64 m_recvSizeBefore;		// 定时检测数据，记录上一次已接收数据大小

	CResTransSender* m_resSender;
};
#endif // !FILETRANSFERRECEVICER_H
