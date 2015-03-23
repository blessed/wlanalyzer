#ifndef IPFIELD_H
#define IPFIELD_H

#include <QFrame>
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QHostAddress>

enum class IP_VERSION
{
	IPV4,
	IPV6
};

/**
 * @brief The IpField widget sports ipv4/ipv6 input fields
 */
class IpField : public QFrame
{
	Q_OBJECT

public:
	IpField(IP_VERSION ver = IP_VERSION::IPV4, QWidget *parent = 0);
	~IpField();

	virtual bool eventFilter( QObject *obj, QEvent *event );
	QHostAddress getAddress() const;
	quint16 getPort() const;

public slots:
	void slotTextChanged(QLineEdit* pEdit);

signals:
	void signalTextChanged(QLineEdit* pEdit);
	void rejected();
	void accepted();
	void filled(bool is_filled);

private:
	void moveNextLineEdit (int i);
	void movePrevLineEdit (int i);

private:
	int IP_SIZE;

	bool isFilled;
	QLineEdit **ipLine;
	QSpinBox *portLine;
};

#endif // IPFIELD_H
