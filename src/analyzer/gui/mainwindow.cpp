#include <QFileDialog>
#include <QMessageBox>
#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/ipdialog.h"

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

    connect(ui->packetHexEdit, SIGNAL(addressSelected(qint64)),
			this, SLOT(addressSelected(qint64)));
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

void MainWindow::addressSelected(qint64 addr)
{
#ifdef DEBUG_BUILD
    qDebug() << "address selected: " << addr;
    ui->packetHexEdit->highlight(addr, 11);
#else
    Q_UNUSED(addr)
#endif
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
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(dock->parentWidget());
    Q_ASSERT(mainWindow);
    // prevent slight gui movement when we try to pin to area in which the dock
    // already resides
    if(mainWindow->dockWidgetArea(dock) == area)
        return;
    mainWindow->addDockWidget(area, dock);
}
