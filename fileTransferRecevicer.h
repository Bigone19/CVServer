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
	* @brief: ��������
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
	QString m_absFilePathInHost;	// �ļ��������е�·���������ļ���
	QString m_fileName;
	qint64 m_fileSize;
	qint64 m_recvSize;				// �������ݴ�С
	QString m_fileCacheDir;			// �ļ�����·��

	bool m_isCancel;
	QDateTime m_startTime;			// ���俪ʼʱ���¼����
	bool m_startTimeFlag;			// ���俪ʼʱ���¼��ʶ
	QTimer* m_timerProgress;		// ��ʱ��ȡ�ѽ������ݴ�С��ʱ��
	bool m_isRunning;
	QByteArrayList m_cacheList;		// �������ݻ������
	QReadWriteLock m_cacheListLock;	// �������ݻ�����ж�д��

	QTimer* m_timeOutProgress;		// ���䳬ʱ��ⶨʱ��
	qint64 m_recvSizeBefore;		// ��ʱ������ݣ���¼��һ���ѽ������ݴ�С

	CResTransSender* m_resSender;
};
#endif // !FILETRANSFERRECEVICER_H
