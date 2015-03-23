#ifndef IPDIALOG_H
#define IPDIALOG_H

#include <QHostAddress>
#include <QtWidgets>
#include "ipfield.h"

class IpDialog : public QDialog
{
	Q_OBJECT

public:
	IpDialog(QWidget *parent = 0);
	~IpDialog();

	QHostAddress getAddress() const;
	quint16 getPort() const;

private:
	IpField *ipfield;
	QPushButton *okButton;
	QPushButton *cancelButton;
};

#endif // IPDIALOG_H

