//#include <QLabel>
//#include <QSpinBox>
//#include <QRegExpValidator>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
#include <QtWidgets>
#include "ipdialog.h"
#include "ipdialog.moc"

IpDialog::IpDialog(QWidget *parent) : QFrame(parent)
{
	QHBoxLayout *hboxlayout = new QHBoxLayout(this);

	hboxlayout->setContentsMargins(0, 0, 0, 0);
	hboxlayout->setSpacing(0);

	for (int i = 0; i < 4; ++i)
	{
		if (i != 0)
		{
			QLabel *dot = new QLabel(".", this);
			dot->setStyleSheet("background: white");
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
		edit->setFixedWidth(5 * QFontMetrics(font).width("0"));
		QRegExp rx("^(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))$");
		QValidator *validator = new QRegExpValidator(rx, edit);
		edit->setValidator(validator);
	}

	QLabel *colon = new QLabel(":", this);
	colon->setStyleSheet("background: white");
	hboxlayout->addWidget(colon);
	hboxlayout->setStretch(hboxlayout->count(), 1);

	QSpinBox *spinBox = new QSpinBox(this);
	spinBox->setMinimum(0);
	spinBox->setMaximum(65535);

//	portLine = new QLineEdit(this);
//	portLine->installEventFilter(this);
	hboxlayout->addWidget(spinBox);
	hboxlayout->setStretch(hboxlayout->count(), 2);

	setMaximumWidth(120);

	connect(this, SIGNAL(signalTextChanged(QLineEdit*)),
			this, SLOT(slotTextChanged(QLineEdit*)),
			Qt::QueuedConnection);
}

IpDialog::~IpDialog()
{
}

bool IpDialog::eventFilter(QObject *obj, QEvent *event)
{
	bool res = QFrame::eventFilter(obj, event);

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
		if (event)
		{
			for (int i = 0; i < 4; ++i)
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
						if (edit->text().isEmpty() || edit->text().endsWith("0"))
						{
							edit->setText(edit->text() + "0");
							moveNextLineEdit(i);
						}
						break;

					case Qt::Key_Backspace:
						if (edit->text().isEmpty() || edit->cursorPosition() == 0)
							movePrevLineEdit(i);
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

void IpDialog::slotTextChanged(QLineEdit *pEdit)
{
	for (int i = 0; i < 4; ++i)
	{
		if (pEdit == ipLine[i])
		{
			if ((pEdit->text().size() == 3 && pEdit->text().size() == pEdit->cursorPosition()) ||
					(pEdit->text() == "0"))
			{
				if ( i + 1 < 4)
				{
					ipLine[i+1]->setFocus();
					ipLine[i+1]->selectAll();
				}
			}

		}
	}
}

void IpDialog::moveNextLineEdit(int i)
{
	if (i + 1 < 4)
	{
		ipLine[i+1]->setFocus();
		ipLine[i+1]->setCursorPosition(0);
		ipLine[i+1]->selectAll();
	}
}

void IpDialog::movePrevLineEdit(int i)
{
	if (i != 0)
	{
		ipLine[i-1]->setFocus();
		ipLine[i-1]->setCursorPosition(ipLine[i-1]->text().size());
	}
}

