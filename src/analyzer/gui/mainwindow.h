#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void connectSlot();
	void openDumpFileSlot();
	void createSessionSlot();
	void startRecentSessionSlot();
	void aboutSlot();
	void clearRecentSlot();
	void addressSelected(qint64 addr);

    // dock widget pin to area logic
    void pinDockToTopSlot();
    void pinDockToLeftSlot();
    void pinDockToRightSlot();
    void pinDockToBottomSlot();

private:
    void pinDockToArea(QDockWidget* dock, Qt::DockWidgetArea area);
    void closeEvent(QCloseEvent *event);
    void readSettings();
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
