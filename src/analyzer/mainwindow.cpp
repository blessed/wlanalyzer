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
