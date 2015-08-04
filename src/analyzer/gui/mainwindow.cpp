#include <QFileDialog>
#include <QMessageBox>
#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/mainwindow.moc"
#include "gui/ipdialog.h"
#include "gui/editsessiondialog.h"

namespace {
    const QString KEY_MAINWINDOW_GEOMETRY("mainwindow/geometry");
    const QString KEY_MAINWINDOW_STATE("mainwindow/state");
    const QString KEY_RECENT_SESSIONS("mainwindow/recent_sessions");

    QString sessionSummaryText(Session::Info::ptr_t session)
    {
        if(!session)
            return QString();

        if(!session->m_sessionName.isEmpty())
            return session->m_sessionName;

        return QFileInfo(session->m_binaryPath).fileName();
    }
};

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

    readSettings();
    updateRecents();
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

void MainWindow::openDumpFileSlot()
{
	QFileDialog fileDialog;

	if (fileDialog.exec())
	{
		qDebug("Opened");
	}
	else
		qDebug("Pressed cancel");
}

void MainWindow::createSessionSlot()
{
	qDebug("createSessionSlot");
    Session::Info::ptr_t info;

	EditSessionDialog sessionDialog;
	if (sessionDialog.exec())
	{
		qDebug("session info:");
        info = sessionDialog.getSessionInfo();
        m_recents.append(info);
        updateRecents();
        //TODO save or start the session here
	}
	else
	{
		qDebug("Pressed cancel");
	}

}

void MainWindow::startRecentSessionSlot()
{
	qDebug("startRecentSessionSlot");
    QAction* action = qobject_cast<QAction *>(sender());
    if(!action)
        return;

    Q_ASSERT(!action->data().isNull());
    bool ok;
    int i = action->data().toInt(&ok);
    Q_ASSERT(ok);
    Q_ASSERT(i >= 0 && m_recents.length() > i);

	EditSessionDialog sessionDialog;
	sessionDialog.setSessionInfo(m_recents[i]);
    if (sessionDialog.exec())
	{
		qDebug("session info:");
        // TODO REORDER recent sessions addRecentSession(info);
        //TODO start the session here
        //TODO update items edited tafter selection with recents
	}
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

void MainWindow::clearRecentSlot()
{
    qDebug() << "Clear Recents";
    m_recents.clear();
    updateRecents();
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    // TODO add application version and purge settings on incompatibilities
    settings.setValue(KEY_MAINWINDOW_GEOMETRY, saveGeometry());
    settings.setValue(KEY_MAINWINDOW_STATE, saveState());

    settings.beginWriteArray(KEY_RECENT_SESSIONS);
    for (int i = 0; i < m_recents.size(); ++i)
    {
        settings.setArrayIndex(i);
        m_recents[i]->Save(settings);
    }
    settings.endArray();
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value(KEY_MAINWINDOW_GEOMETRY).toByteArray());
    restoreState(settings.value(KEY_MAINWINDOW_STATE).toByteArray());
    int recents_size = settings.beginReadArray(KEY_RECENT_SESSIONS);
    for (int i = 0; i < recents_size; ++i)
    {
        settings.setArrayIndex(i);
        Session::Info::ptr_t session = Session::Info::FromSettings(settings);
        // TODO some validation and fallbacks
        m_recents.append(session);
    }
    settings.endArray();
}


void MainWindow::updateRecents()
{
    ui->menu_RecentSessions->clear();
    ui->menu_RecentSessions->setDisabled(m_recents.isEmpty());

    if(m_recents.isEmpty())
        return;

    QAction *act = nullptr;
    QMenu *menu = ui->menu_RecentSessions;
    for(int i=0; i < m_recents.length(); i++)
    {
        QString title = tr("&%1 %2").arg(i + 1).arg(sessionSummaryText(m_recents[i]));
        act = new QAction(title, this);
        act->setData(i);
        connect(act, SIGNAL(triggered(void)), this, SLOT(startRecentSessionSlot(void)));
        menu->addAction(act);
    }

    menu->addSeparator();
    act = new QAction(tr("&Clear Menu"), this);
    connect(act, SIGNAL(triggered(void)), this, SLOT(clearRecentSlot(void)));
    menu->addAction(act);
}
