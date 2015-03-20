#ifndef IPDIALOG_H
#define IPDIALOG_H

#include <QFrame>
#include <QWidget>
#include <QLineEdit>

class IpDialog : public QFrame
{
	Q_OBJECT

public:
	IpDialog(QWidget *parent = 0);
	~IpDialog();

	virtual bool eventFilter( QObject *obj, QEvent *event );

public slots:
	void slotTextChanged(QLineEdit* pEdit);

signals:
	void signalTextChanged(QLineEdit* pEdit);

private:
	void moveNextLineEdit (int i);
	void movePrevLineEdit (int i);

private:
	QLineEdit *ipLine[4];
	QLineEdit *portLine;
};

#endif // IPDIALOG_H
