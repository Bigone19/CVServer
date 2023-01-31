/********************************************************************************
** Form generated from reading UI file 'server.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVER_H
#define UI_SERVER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CServer
{
public:
    QMenuBar *menubar;
    QWidget *centralwidget;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *CServer)
    {
        if (CServer->objectName().isEmpty())
            CServer->setObjectName("CServer");
        CServer->resize(800, 600);
        menubar = new QMenuBar(CServer);
        menubar->setObjectName("menubar");
        CServer->setMenuBar(menubar);
        centralwidget = new QWidget(CServer);
        centralwidget->setObjectName("centralwidget");
        CServer->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(CServer);
        statusbar->setObjectName("statusbar");
        CServer->setStatusBar(statusbar);

        retranslateUi(CServer);

        QMetaObject::connectSlotsByName(CServer);
    } // setupUi

    void retranslateUi(QMainWindow *CServer)
    {
        CServer->setWindowTitle(QCoreApplication::translate("CServer", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CServer: public Ui_CServer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVER_H
