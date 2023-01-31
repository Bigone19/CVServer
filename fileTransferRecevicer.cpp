#include "fileTransferRecevicer.h"
#include "resTransferSender.h"

#include<QtConcurrent/QtConcurrent>

CFileTransRecevicer* CFileTransRecevicer::m_recevicer = nullptr;

CFileTransRecevicer::CFileTransRecevicer(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, m_isCancel(false)
	, m_isRunning(false)
	, m_startTimeFlag(false)
	, m_fileSize(0)
	, m_recvSize(0)
	, m_recvSizeBefore(0)
{
	m_resSender = new CResTransSender(parent);

	setCacheDir(m_resSender->getRawImageDir());
	m_commSocket = new QTcpSocket(this);
	m_fileSocket = new QTcpSocket(this);
	m_tcpServerComm = new QTcpServer(this);
	m_tcpServerFile = new QTcpServer(this);
	m_fileSocket->setReadBufferSize(128 * 1024 * 1024);

	m_tcpServerComm->listen(QHostAddress::Any, PORT_COMMAND);
	m_tcpServerFile->listen(QHostAddress::Any, PORT_FILE);

	m_timerProgress = new QTimer(this);
	connect(m_timerProgress, &QTimer::timeout, this, &CFileTransRecevicer::progress);
	m_timeOutProgress = new QTimer(this);
	connect(m_timeOutProgress, &QTimer::timeout, this, &CFileTransRecevicer::timeOutCheck);

	connect(m_tcpServerComm, SIGNAL(newConnection()), this, SLOT(on_connect_c()));
	connect(m_tcpServerFile, SIGNAL(newConnection()), this, SLOT(on_connect_f()));
}

CFileTransRecevicer::~CFileTransRecevicer()
{
	m_cacheList.clear();
}

void CFileTransRecevicer::send_command(SEND_TYPE code, int ret, QString additional /*= ""*/)
{
	QByteArray data;
	QDataStream in(&data, QIODevice::WriteOnly);
	in.setVersion(QDataStream::Qt_5_9);
	QString head = QString("%1" + QString(SPLIT_TYPE_INFO_MAEK) + "%2" + SPLIT_TYPE_INFO_MAEK + "%3").arg((int)code).arg(ret).arg(additional);
	in << qint64(2) << head.toUtf8();
	in.device()->seek(0);

	in << qint64(data.size() - sizeof(qint64));
	qint64 len = m_commSocket->write(data);
	m_commSocket->waitForBytesWritten(-1);
	if (len <= 0)
	{
		qDebug() << QString::fromLocal8Bit("命令发送失败 ") << (int)code;
	}
	else
	{
		qDebug() << QString::fromLocal8Bit("发送命令: ") << (int)code << QString::fromLocal8Bit("执行结果: ") << ret << QString::fromLocal8Bit("附加信息: ") << additional;
	}
}

void CFileTransRecevicer::progress()
{
	send_command(SEND_TYPE::FILE_REC_CODE, K_SUCCEED, QString::number(m_recvSize));
	if (m_fileSize > 0)
	{
		qDebug() << QString::fromLocal8Bit("******已接收数据百分比:") << (m_recvSize * 100) / m_fileSize;
	}
	if (m_recvSize >= m_fileSize)
	{
		send_command(SEND_TYPE::FILE_TRANSFER_FINISH_CODE, K_SUCCEED, QString::number(m_recvSize));
		m_timerProgress->stop();
		m_timeOutProgress->stop();
		qDebug() << __FUNCTION__ << QString::fromLocal8Bit("文件") << m_fileName << QString::fromLocal8Bit("接收完成");
		// 向结果接收端返回图片 <--- 后期修改
		m_resSender->sendFile();
	}
}

void CFileTransRecevicer::timeOutCheck()
{
	if (m_timerProgress->isActive() && (m_recvSize != m_fileSize) 
		&& (m_recvSize != 0) && (m_recvSize == m_fileSize))
	{
		send_command(SEND_TYPE::FILE_TRANSFER_TIMEOUT_CODE, K_ERROR, ERROR_CODE_11);
		m_timeOutProgress->stop();
	}
	m_recvSizeBefore = m_recvSize;
}

void CFileTransRecevicer::on_connect_c()
{
	m_commSocket = m_tcpServerComm->nextPendingConnection();
	QString ip = m_commSocket->peerAddress().toString();
	qint64 port = m_commSocket->peerPort();

	qDebug() << QString::fromLocal8Bit("[on_connect_c()==>%1:%2]成功连接 ").arg(ip).arg(port);
	// 连接接收端
	m_resSender->connectToServer();

	m_commSocket->disconnect();
	connect(m_commSocket, SIGNAL(readyRead()), this, SLOT(on_read_command()));
	connect(m_commSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onCommandError(QAbstractSocket::SocketError)));
}

void CFileTransRecevicer::on_connect_f()
{
	m_fileSocket = m_tcpServerFile->nextPendingConnection();
	QString ip = m_fileSocket->peerAddress().toString();
	qint64 port = m_fileSocket->peerPort();

	qDebug() << QString::fromLocal8Bit("[on_connect_f()==>%1:%2]成功连接 ").arg(ip).arg(port);
	m_fileSocket->disconnect();
	connect(m_fileSocket, SIGNAL(readyRead()), this, SLOT(on_read_file()));
	connect(m_fileSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onFileError(QAbstractSocket::SocketError)));
	m_startTimeFlag = true;
}

void CFileTransRecevicer::on_read_command()
{
	QByteArray buf = m_commSocket->readAll();
	SEND_TYPE code = (SEND_TYPE)QString(buf).section(SPLIT_TYPE_INFO_MAEK, 0, 0).toInt();
	QString additionalInfo = QString(buf).section(SPLIT_TYPE_INFO_MAEK, 1, 1);
	switch (code)
	{
	case SEND_TYPE::FILE_HEAD_CODE:
	{
		quitFileTransferThread();
		if (additionalInfo.isEmpty())
		{
			qDebug() << ERROR_CODE_4;
			return;
		}
		m_absFilePathInHost = additionalInfo.section(SPLIT_ADDITION_INFO_MARK, 0, 0);
		m_fileName = m_absFilePathInHost.right(m_absFilePathInHost.size() - m_absFilePathInHost.lastIndexOf('/') - 1);
		m_fileSize = additionalInfo.section(SPLIT_ADDITION_INFO_MARK, 1, 1).toLongLong();
		m_recvSize = 0;
		m_recvSizeBefore = 0;
		if (m_file.isOpen())
		{
			m_file.close();
		}
		QDir::setCurrent(m_fileCacheDir);
		m_file.setFileName(m_fileName);
		bool isOk = m_file.open(QIODevice::WriteOnly);
		if (false == isOk)
		{
			qDebug() << __FUNCTION__ << __LINE__ << "writeonly error.";
			m_commSocket->disconnectFromHost();
			m_commSocket->close();
			return;
		}
		qDebug() << QString::fromLocal8Bit("文件名：%1\n大小:%2 kb").arg(m_fileName).arg(m_fileSize / 1024);
		//send_command(SEND_TYPE::FILE_HEAD_REC_CODE, K_SUCCEED, SUCCEED_CODE_1);
		break;
	}
	case SEND_TYPE::FILE_CANCEL:
	{
		if (!m_isCancel)
		{
			if (m_file.isOpen())
			{
				QDir::setCurrent(m_fileCacheDir);
				m_file.close();
				m_file.remove();
			}
			m_isCancel = true;
			m_timerProgress->stop();
			m_timeOutProgress->stop();
			send_command(SEND_TYPE::FILE_REC_CANCEL, K_SUCCEED, SUCCEED_CODE_3);
			qDebug() << __FUNCTION__ << __LINE__ << QString::fromLocal8Bit("已取消********");
		}
		else
		{
			send_command(SEND_TYPE::FILE_REC_CANCEL, K_ERROR, ERROR_CODE_7);
		}
		break;
	}
	case SEND_TYPE::FILE_CODE:
	{
		qDebug() << QString::fromLocal8Bit("开始接收文件数据信息 ") << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
		break;
	}
	default:
		break;
	}
}

void CFileTransRecevicer::on_read_file()
{
	if (m_startTimeFlag)
	{
		m_startTime = QDateTime::currentDateTime();
		m_startTimeFlag = false;
		m_cacheListLock.lockForWrite();
		m_cacheList.clear();
		m_cacheListLock.unlock();
		m_timerProgress->start(500);
		m_timeOutProgress->start(3000);
		qDebug() << __FUNCTION__ << QString::fromLocal8Bit("***开始读取文件数据***") << "\m_isCancel===>" << m_isCancel;
		m_isRunning = true;
		m_isCancel = false;
		QtConcurrent::run([this] {beginSave(); });
	}
	if (m_fileSocket->bytesAvailable())
	{
		QByteArray buf = m_fileSocket->readAll();
		m_cacheListLock.lockForWrite();
		m_cacheList.push_back(buf);
		m_cacheListLock.unlock();
	}
}

void CFileTransRecevicer::onFileError(QAbstractSocket::SocketError)
{
	qDebug() << "file_socket error: " << m_fileSocket->errorString();
}

void CFileTransRecevicer::onCommandError(QAbstractSocket::SocketError)
{
	qDebug() << "command_socket error:" << m_commSocket->errorString();
}

void CFileTransRecevicer::beginSave()
{
	while (true)
	{
		if (m_isCancel)
		{
			break;
		}
		m_cacheListLock.lockForWrite();
		if (m_cacheList.size() > 0 && m_file.isOpen())
		{
			QByteArray buf = m_cacheList.front();
			m_cacheList.pop_front();
			m_cacheListLock.unlock();
			qint64 len = m_file.write(buf);
			if (len > 0)
			{
				m_recvSize += len;
			}
		}
		else
		{
			m_cacheListLock.unlock();
			if (m_recvSize == m_fileSize)
			{
				qint64 msec = m_startTime.msecsTo(QDateTime::currentDateTime());
				if (msec)
				{
					qDebug() << QString::fromLocal8Bit("接收数据时间为:") << msec << QString::fromLocal8Bit("ms, \t速率:") << (m_fileSize * 1000) / (1024 * 1024 * msec) << "MB/S";
				}
				QDir::setCurrent(m_fileCacheDir);
				m_file.close();
				qDebug() << QString::fromLocal8Bit("文件缓存路径:") << m_fileCacheDir;
				break;
			}
			QThread::msleep(10);
		}
	}
	qDebug() << __FUNCTION__ << QString::fromLocal8Bit("****已中断文件传输线程***\t_bCancel===>") << m_isCancel;
	m_isRunning = false;
}

void CFileTransRecevicer::quitFileTransferThread()
{
	while (m_isRunning)
	{
		m_isCancel = true;
		QThread::msleep(10);
	}
}

void CFileTransRecevicer::setCacheDir(const QString& dir)
{
	QDir mdir(dir);
	if (!mdir.exists())
	{
		mdir.mkdir(dir);
		qDebug() << __FUNCTION__ << "缓存目录创建成功 ";
	}
	m_fileCacheDir = dir;
}
