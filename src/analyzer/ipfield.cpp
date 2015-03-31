//#include <QLabel>
//#include <QSpinBox>
//#include <QRegExpValidator>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
#include <QtWidgets>
#include "ipfield.h"
#include "ipfield.moc"

IpField::IpField(QWidget *parent, IP_VERSION ver) : QFrame(parent), isFilled(false)
{
	IP_SIZE = ver == IP_VERSION::IPV4 ? 4 : 6;
	ipLine = new QLineEdit*[IP_SIZE];

	QHBoxLayout *hboxlayout = new QHBoxLayout(this);
	hboxlayout->setContentsMargins(0, 0, 0, 0);
	hboxlayout->setSpacing(0);

	for (int i = 0; i < IP_SIZE; ++i)
	{
		if (i != 0)
		{
			QLabel *dot = new QLabel(".", this);
			hboxlayout->addWidget(dot);
			hboxlayout->setStretch(hboxlayout->count(), 0);
		}

		ipLine[i] = new QLineEdit(this);
		QLineEdit *edit = ipLine[i];
		edit->installEventFilter(this);

		hboxlayout->addWidget(edit);
		hboxlayout->setStretch(hboxlayout->count(), 2);

		edit->setFrame(false);
		edit->setAlignment(Qt::AlignCenter);

		QFont font = edit->font();
		font.setStyleHint(QFont::Monospace);
		font.setFixedPitch(true);
		// set reasonable lineedit width without subclassing equal to width of octet and one digit margin
		edit->setFixedWidth(6 * QFontMetrics(font).width("0"));
		QRegExp rx("^(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))$");
		QValidator *validator = new QRegExpValidator(rx, edit);
		edit->setValidator(validator);
	}

	QLabel *colon = new QLabel(":", this);
	hboxlayout->addWidget(colon);
	hboxlayout->setStretch(hboxlayout->count(), 2);

	portLine = new QSpinBox(this);
	portLine->setMinimum(0);
	portLine->setMaximum(65535);
	portLine->setValue(5000);
	portLine->setButtonSymbols(QSpinBox::NoButtons);

	hboxlayout->addWidget(portLine);
	hboxlayout->setStretch(hboxlayout->count(), 2);

	connect(this, SIGNAL(signalTextChanged(QLineEdit*)),
			this, SLOT(slotTextChanged(QLineEdit*)),
			Qt::QueuedConnection);
}

IpField::~IpField()
{
	if (ipLine)
		delete [] ipLine;
}

bool IpField::eventFilter(QObject *obj, QEvent *event)
{
	bool res = QFrame::eventFilter(obj, event);

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
		if (event)
		{
			for (int i = 0; i < IP_SIZE; ++i)
			{
				QLineEdit *edit = ipLine[i];
				if (edit == obj)
				{
					switch (key->key())
					{
					case Qt::Key_Left:
						if (edit->cursorPosition() == 0)
							movePrevLineEdit(i);
						break;

					case Qt::Key_Right:
						if (edit->text().isEmpty() || edit->text().size() == edit->cursorPosition())
							moveNextLineEdit(i);
						break;

					case Qt::Key_0:
						if (edit->text().isEmpty() || edit->text().endsWith("0") || edit->text().size() == 2 ||
								edit->text()[0] > '2')
							moveNextLineEdit(i);
						break;

					case Qt::Key_Backspace:
						if (edit->text().isEmpty() || edit->cursorPosition() == 0)
							movePrevLineEdit(i);
						emit signalTextChanged(edit);
						break;

					case Qt::Key_Comma:
					case Qt::Key_Period:
						moveNextLineEdit(i);
						break;

					default:
						emit signalTextChanged(edit);
						break;
					}
				}
			}
		}
	}

	return res;
}

void IpField::slotTextChanged(QLineEdit *pEdit)
{
	isFilled = true;

	for (int i = 0; i < IP_SIZE; ++i)
	{
		if (pEdit == ipLine[i])
		{
			if (((pEdit->text().size() == 3) && (pEdit->text().size() == pEdit->cursorPosition())) ||
					((pEdit->text()[0] != '1') && (pEdit->text()[0] != '2') && (pEdit->text().size() == 2)) ||
					(pEdit->text().endsWith("0")))
			{
				if ( i + 1 < IP_SIZE)
				{
					ipLine[i+1]->setFocus();
					ipLine[i+1]->selectAll();
				}
			}
		}

		if (ipLine[i]->text().isEmpty() || ipLine[i]->text() == "")
			isFilled = false;
	}

	emit filled(isFilled);
}

void IpField::moveNextLineEdit(int i)
{
	if (i + 1 < IP_SIZE)
	{
		ipLine[i+1]->setFocus();
		ipLine[i+1]->setCursorPosition(0);
		ipLine[i+1]->selectAll();
	}
}

void IpField::movePrevLineEdit(int i)
{
	if (i != 0)
	{
		ipLine[i-1]->setFocus();
		ipLine[i-1]->setCursorPosition(ipLine[i-1]->text().size());
	}
}

QHostAddress IpField::getAddress() const
{
	QString address;

	for (int i = 0; i < IP_SIZE; ++i)
	{
		address += ipLine[i]->text();

		if (i < IP_SIZE - 1)
			address += '.';
	}

	return QHostAddress(address);
}

quint16 IpField::getPort() const
{
	return portLine->value();
}
