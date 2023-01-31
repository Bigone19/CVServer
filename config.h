#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>

#define IP_ADDR "127.0.0.1"
#define PORT_COMMAND 42000
#define PORT_FILE 41000
#define PORT_RESCOMM 42001
#define PORT_RESFILE 41001

#define SEND_BLOCK_SIZE (qint64)(16 * 1024 * 1024)

#define SPLIT_TYPE_INFO_MAEK        "##"
#define SPLIT_ADDITION_INFO_MARK    ",,"

#define K_ERROR           false
#define K_SUCCEED         true

// ��������
#define ERROR_CODE_1    QString::fromLocal8Bit("���õ��ļ�·������ ") 
#define ERROR_CODE_2    QString::fromLocal8Bit("Ҫ�ϴ����ļ���ʧ�� ") 
#define ERROR_CODE_3    QString::fromLocal8Bit("�����ʧ�� ") 
#define ERROR_CODE_4    QString::fromLocal8Bit("�ļ�ͷ����Ϣ����ʧ�� ") 
#define ERROR_CODE_5    QString::fromLocal8Bit("�ļ����ݽ���ʧ�� ") 
#define ERROR_CODE_6    QString::fromLocal8Bit("���̿ռ��Ѳ���,�޷��洢���ļ� ") 
#define ERROR_CODE_7	QString::fromLocal8Bit("cannot repeat cancel ")
//#define ERROR_CODE_7    QString::fromLocal8Bit("�����ٴε��ȡ������ ") 
#define ERROR_CODE_8    QString::fromLocal8Bit("��ѯ�ļ��Ƿ���ڣ�������Ϣ���� ") 
#define ERROR_CODE_9    QString::fromLocal8Bit("���õĻ���Ŀ¼���� ") 
#define ERROR_CODE_10   QString::fromLocal8Bit("�ļ��ڻ����в����� ") 
#define ERROR_CODE_11   QString::fromLocal8Bit("�ļ����䳬ʱ ") 

// �ɹ���Ϣ
#define SUCCEED_CODE_1    QString::fromLocal8Bit("�ļ�ͷ����Ϣ���ճɹ� ")
#define SUCCEED_CODE_2    QString::fromLocal8Bit("���ն��ļ����ݽ������ ")
#define SUCCEED_CODE_3    QString::fromLocal8Bit("���ն��ļ�ȡ����� ")
#define SUCCEED_CODE_4    QString::fromLocal8Bit("�ļ��ڻ����д��� ")
#define SUCCEED_CODE_5    QString::fromLocal8Bit("���̿ռ��С�㹻�洢���ļ� ")

// ���͵���������
enum class SEND_TYPE
{
	FILE_HEAD_CODE = 77,				// �����ļ�ͷ��Ϣ
	FILE_HEAD_REC_CODE,					// �����ļ�ͷ��Ӧ
	FILE_CANCEL,						// ����ȡ���ļ�������Ϣ
	FILE_REC_CANCEL,					// ����ȡ��������Ӧ
	FILE_CODE,							// �����ļ�����
	FILE_REC_CODE,						// �����ļ���Ӧ
	FILE_IS_EXIST_CODE,					// ���Ͳ�ѯ�ļ��Ƿ��������
	FILE_IS_EXIST_REC_CODE,				// ���ղ�ѯ�ļ��Ƿ������Ӧ
	FILE_IS_DISK_FREE_SPACE_CODE,		// ���Ͳ�ѯ���̿ռ��Ƿ���
	FILE_IS_DISK_FREE_SPACE_REC_CODE,	// ���ղ�ѯ���̿ռ��Ƿ�����Ӧ
	FILE_TRANSFER_TIMEOUT_CODE,			// �ļ����䳬ʱ,���������֪ͨ�ͻ���
	FILE_TRANSFER_FINISH_CODE			// �ļ��������
};

//�������
enum ERROR_CLASSIFY_EM
{
	K_FILE_TRANSFER_ERROR = 100,
	K_NETWORK_ERROR = K_FILE_TRANSFER_ERROR + 200,
	K_RESPONSE_TIMEOUT = K_NETWORK_ERROR + 200
};

//�������
enum K_NETWORK_ERROR_EM
{
	K_NETWORK_DISCONNECTED_COMMAND_ERROR = ERROR_CLASSIFY_EM::K_NETWORK_ERROR + 1,		//������������socket����
	K_NETWORK_DISCONNECTED_FILE_ERROR													//���������ļ�socket����
};

enum K_RESPONSE_TIMEOUT_EM
{
	K_RESPONSE_CANCEL_FILE_TIMEOUT = ERROR_CLASSIFY_EM::K_RESPONSE_TIMEOUT + 1,			//ȡ���ļ���ʱ
	K_RESPONSE_IS_DISK_FREE_SPACE_FILE_TIMEOUT,											//�жϴ��̿ռ��Ƿ��㳬ʱ
	K_RESPONSE_IS_EXIST_FILE_TIMEOUT,													//�ж��ļ��Ƿ���ڳ�ʱ
	K_RESPONSE_FILE_TRANSFER_TIMEOUT													//�ļ����䳬ʱ
};

#endif // !CONFIG_H
