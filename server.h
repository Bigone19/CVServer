#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include "fileTransferRecevicer.h"

namespace Ui {
class CServer;
}

class CServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit CServer(QWidget *parent = nullptr);
    ~CServer();

private:
    Ui::CServer *ui;
    CFileTransRecevicer* m_fileRecv;
};

#endif // SERVER_H
