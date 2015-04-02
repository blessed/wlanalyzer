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
	void openSlot();
	void aboutSlot();

    // dock widget pin to area logic
    void pinDockToTopSlot();
    void pinDockToLeftSlot();
    void pinDockToRightSlot();
    void pinDockToBottomSlot();

private:
    void pinDockToArea(QDockWidget* dock, Qt::DockWidgetArea area);
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
