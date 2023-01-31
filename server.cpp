#include "server.h"
#include "ui_server.h"

CServer::CServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CServer)
{
    ui->setupUi(this);
    m_fileRecv = CFileTransRecevicer::GetInstance();
}

CServer::~CServer()
{
    delete ui;
}
