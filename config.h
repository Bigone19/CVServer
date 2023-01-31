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

// 错误类型
#define ERROR_CODE_1    QString::fromLocal8Bit("设置的文件路径错误 ") 
#define ERROR_CODE_2    QString::fromLocal8Bit("要上传的文件打开失败 ") 
#define ERROR_CODE_3    QString::fromLocal8Bit("命令发送失败 ") 
#define ERROR_CODE_4    QString::fromLocal8Bit("文件头部信息接收失败 ") 
#define ERROR_CODE_5    QString::fromLocal8Bit("文件数据接收失败 ") 
#define ERROR_CODE_6    QString::fromLocal8Bit("磁盘空间已不足,无法存储该文件 ") 
#define ERROR_CODE_7	QString::fromLocal8Bit("cannot repeat cancel ")
//#define ERROR_CODE_7    QString::fromLocal8Bit("不能再次点击取消操作 ") 
#define ERROR_CODE_8    QString::fromLocal8Bit("查询文件是否存在，附加信息错误 ") 
#define ERROR_CODE_9    QString::fromLocal8Bit("设置的缓存目录错误 ") 
#define ERROR_CODE_10   QString::fromLocal8Bit("文件在缓存中不存在 ") 
#define ERROR_CODE_11   QString::fromLocal8Bit("文件传输超时 ") 

// 成功消息
#define SUCCEED_CODE_1    QString::fromLocal8Bit("文件头部信息接收成功 ")
#define SUCCEED_CODE_2    QString::fromLocal8Bit("接收端文件数据接收完成 ")
#define SUCCEED_CODE_3    QString::fromLocal8Bit("接收端文件取消完成 ")
#define SUCCEED_CODE_4    QString::fromLocal8Bit("文件在缓存中存在 ")
#define SUCCEED_CODE_5    QString::fromLocal8Bit("磁盘空间大小足够存储该文件 ")

// 发送的命令类型
enum class SEND_TYPE
{
	FILE_HEAD_CODE = 77,				// 发送文件头信息
	FILE_HEAD_REC_CODE,					// 接收文件头响应
	FILE_CANCEL,						// 发送取消文件传输信息
	FILE_REC_CANCEL,					// 接收取消传输响应
	FILE_CODE,							// 发送文件数据
	FILE_REC_CODE,						// 接收文件响应
	FILE_IS_EXIST_CODE,					// 发送查询文件是否存在请求
	FILE_IS_EXIST_REC_CODE,				// 接收查询文件是否存在响应
	FILE_IS_DISK_FREE_SPACE_CODE,		// 发送查询磁盘空间是否不足
	FILE_IS_DISK_FREE_SPACE_REC_CODE,	// 接收查询磁盘空间是否不足响应
	FILE_TRANSFER_TIMEOUT_CODE,			// 文件传输超时,服务端主动通知客户端
	FILE_TRANSFER_FINISH_CODE			// 文件传输完成
};

//错误分类
enum ERROR_CLASSIFY_EM
{
	K_FILE_TRANSFER_ERROR = 100,
	K_NETWORK_ERROR = K_FILE_TRANSFER_ERROR + 200,
	K_RESPONSE_TIMEOUT = K_NETWORK_ERROR + 200
};

//网络错误
enum K_NETWORK_ERROR_EM
{
	K_NETWORK_DISCONNECTED_COMMAND_ERROR = ERROR_CLASSIFY_EM::K_NETWORK_ERROR + 1,		//网络连接命令socket错误
	K_NETWORK_DISCONNECTED_FILE_ERROR													//网络连接文件socket错误
};

enum K_RESPONSE_TIMEOUT_EM
{
	K_RESPONSE_CANCEL_FILE_TIMEOUT = ERROR_CLASSIFY_EM::K_RESPONSE_TIMEOUT + 1,			//取消文件超时
	K_RESPONSE_IS_DISK_FREE_SPACE_FILE_TIMEOUT,											//判断磁盘空间是否不足超时
	K_RESPONSE_IS_EXIST_FILE_TIMEOUT,													//判断文件是否存在超时
	K_RESPONSE_FILE_TRANSFER_TIMEOUT													//文件传输超时
};

#endif // !CONFIG_H
