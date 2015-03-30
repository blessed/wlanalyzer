#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ipdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QMainWindow acting as dockarea is asked to acta as a widget instead of
    //top level window. There is no way to set it from designer
    ui->log_page_dockarea->setWindowFlags(Qt::Widget);

    //Show toggle menu actions for all docks in main window menubar
    ui->menu_Logs->addAction(ui->dockPacketList->toggleViewAction());
    ui->menu_Logs->addAction(ui->dockPacketDissection->toggleViewAction());
    ui->menu_Logs->addAction(ui->dockPacketHex->toggleViewAction());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSlot()
{
	IpDialog ip;
	if (ip.exec())
	{
		qDebug("connect to %s:%d", ip.getAddress().toString().toStdString().c_str(), ip.getPort());
	}
	else
	{
		qDebug("Pressed cancel");
	}
}

void MainWindow::openSlot()
{
	QFileDialog fileDialog;

	if (fileDialog.exec())
	{
		qDebug("Opened");
	}
	else
		qDebug("Pressed cancel");
}

void MainWindow::aboutSlot()
{
    QMessageBox::about(this, tr("About this application"),
            tr("This is a GUI analyzer that can analyze acquired wayland traffic.\n"
               "The data can originate either from a pre-recorded file gathered "
               "in offline mode or in online mode by connecting the wldumper"));
}

void MainWindow::pinDockToTopSlot()
{
    pinDockToArea(qobject_cast<QDockWidget *>(sender()->parent()), Qt::TopDockWidgetArea);
}

void MainWindow::pinDockToLeftSlot()
{
    pinDockToArea(qobject_cast<QDockWidget *>(sender()->parent()), Qt::LeftDockWidgetArea);
}

void MainWindow::pinDockToRightSlot()
{
    pinDockToArea(qobject_cast<QDockWidget *>(sender()->parent()), Qt::RightDockWidgetArea);
}

void MainWindow::pinDockToBottomSlot()
{
    pinDockToArea(qobject_cast<QDockWidget *>(sender()->parent()), Qt::BottomDockWidgetArea);
}

void MainWindow::pinDockToArea(QDockWidget* dock, Qt::DockWidgetArea area)
{
    Q_ASSERT(dock);
    qDebug()<< "pin:" << dock << " to:" << area;
}
