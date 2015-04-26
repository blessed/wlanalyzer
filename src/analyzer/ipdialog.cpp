/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Samsung Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ipdialog.h"
#include "ipdialog.moc"

IpDialog::IpDialog(QWidget *parent) : QDialog(parent)
{
	ipfield = new IpField;
	okButton = new QPushButton(tr("Ok"));
	okButton->setEnabled(false);
	cancelButton = new QPushButton(tr("Cancel"));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(ipfield);
	layout->addWidget(okButton);
	layout->addWidget(cancelButton);

	connect(okButton, SIGNAL(clicked(bool)),
			this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked(bool)),
			this, SLOT(reject()));
	connect(ipfield, SIGNAL(filled(bool)),
			okButton, SLOT(setEnabled(bool)));

	setLayout(layout);
}

IpDialog::~IpDialog()
{
}

QHostAddress IpDialog::getAddress() const
{
	return ipfield->getAddress();
}

quint16 IpDialog::getPort() const
{
	return ipfield->getPort();
}
