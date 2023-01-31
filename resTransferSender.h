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

// <============ �ļ���·����ʼ�� ==============>
const string g_initFilePath = g_projectPath.toStdString() + "/init_dir.cfg";
const vector<QString> g_vecDirPath = fileUtils::loadInitFile(g_initFilePath);

class CResTransSender : public QObject
{
	Q_OBJECT
public:
	CResTransSender(QObject* parent = nullptr);
	~CResTransSender();
	/**
	* @brief: �����ļ�
	* @param:
	*/
	void sendFile();
	/**
	* @brief: ���ӵ������
	* @param: 
	*/
	void connectToServer();
	/**
	* @brief:  ��ȡԭʼͼƬ�ļ���·��
	* @param:
	*/
	QString getRawImageDir() const;


signals:
	/**
	* @brief: ����״̬
	* @param:
	*/
	void errorState(int code, int subCode);
	/**
	* @brief: �ļ���������ź�
	* @param:
	*/
	void fileTransferFinish();
	/**
	* @brief: ȡ���ļ�������Ӧ�ź�
	* @param:
	*/
	void cancelSendFileRspSig(int type, QVariant retValue);
	/**
	* @brief: ������ѽ��ܵ����ݽ���
	* @param:
	*/
	void progressValue(const QString& fileName, int progressVal);

private slots:
	/**
	* @brief: ��Ӧ�������Ϣ
	* @param:
	*/
	void onReadComm();
	/**
	* @brief: �ļ�socket����
	* @param:
	*/
	void onFileError(QAbstractSocket::SocketError);
	/**
	* @brief: ����socket����
	* @param:
	*/
	void onCommandError(QAbstractSocket::SocketError);

private:
	/**
	* @brief: �ƶ��ļ�
	* @param isCover:  ���Ϊ true�����Ը��ǵķ�ʽ���ơ��ƶ��ļ���
	*				   ����Ŀ���ļ����ڣ������ɾ�����ơ��ƶ���
	*/
	void moveFile(const bool& isCover);
	/**
	* @brief: ��������
	* @param: 
	*/
	void dealReadComm();
	/**
	* @brief: �˳��ļ������߳�
	* @param:
	*/
	void quitFileTransferThread();
	/**
	* @brief: �����ļ���������
	* @param:
	*/
	void send_file();
	/**
	* @brief: ����ԭʼͼƬ·��
	* @param:
	*/
	void setRawImageDir(const QString& dirPath);
	/**
	* @brief: ����ͼƬ����Ŀ¼
	* @param: 
	*/
	void setBakImageDir(const QString& dirPath);
	/**
	* @brief: ����ͼƬ������Ŀ¼
	* @param: 
	*/
	void setResultDir(const QString& dirPath);
	/**
	* @brief: ����ͼȨ��Ŀ¼
	* @param: 
	*/
	void setWeightDir(const QString& dirPath);
	/**
	* @brief: �����������¼
	* @param: 
	*/
	bool setClsNameDir(const QString& dirPath);
	/**
	* @brief: ���÷����ļ�
	* @param:
	*/
	bool setFileInfo();
	/**
	* @brief: ͼƬ����
	* @param: 
	*/
	bool imageDetection();
	/**
	* @brief: �����ļ���Ϣ����
	* @param:
	*/
	bool sendCommand(SEND_TYPE type, QVariant additionalInfo = "");
	/**
	* @brief: ����ļ���
	* @param: 
	*/
	void clearDir(const QString& dirPath);
private:

	QString m_ipAddress;
	QTcpSocket* m_commSocket;

	QString m_rawImgDir;		// ԭʼͼƬ·��
	QString m_bakImgDir;		// �ϴ���Ϻ��ƶ�ͼƬ����Ŀ¼��
	QFileInfoList m_lstFile;	//<------- ����������Ҫ�޸���������
	QString m_fileName;
	qint64 m_fileSize;
	QFile m_file;

	qint64 m_sendSize;
	QThreadPool m_pool;
	QFuture<void> m_resTransFuture;
	QMutex m_cancelFileTransMutex;
	bool m_isCancel;
	bool m_isRunning;

	qint64 m_nextBlockSize;	// ��һ�����ݴ�С
	qint64 m_recvDataSize;	// �ѽ������ݴ�С
	QString m_cacheData;	// ������������

	int m_timeoutMsec;		// ��ʱʱ��

	// <==================== ONNX ======================>
	Config m_config;			// onnx������ýṹ��
	string m_weightPath;		// Ȩ��λ��
	QString m_weightDirPath;	// Ȩ��Ŀ¼λ��
	QString m_resultDirPath;	// ������Ŀ¼·��
	string m_clsNamePath;		// �������·��
	QString m_clsNameDirPath;	// �������Ŀ¼·��

	CYoloDetector* m_detector;			// onnx�����
	Mat m_srcImage;						// ԭʼ���ͼƬ
	vector<Detection> m_detectResVec;	// �����
	vector<string> m_clsNameVec;		// ����������
	QMutex m_detectMutex;				// �����������
};


#endif // !__RESTRANSFERSENDER_H__