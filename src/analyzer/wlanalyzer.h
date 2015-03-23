#ifndef WLANALYZER_H
#define WLANALYZER_H

#include <QtWidgets>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private:
	void createActions();
	void createMenus();

private slots:
	void connectSlot();
	void openSlot();

private:

	QMenu *fileMenu;
	QMenu *helpMenu;

	QAction *connectAct;
	QAction *openAct;
	QAction *exitAct;
	QAction *aboutAct;
};

#endif // WLANALYZER_H

