#include <QDebug>
#include "gui/validatedlineedit.h"
#include "gui/validatedlineedit.moc"

ValidatedLineEdit::ValidatedLineEdit(QWidget* parent)
    :QLineEdit(parent),
    m_errorTextColor(Qt::red),
    m_errorBgColor(QColor("#FFFFD5")),
    m_normalTextColor(textColor()),
    m_normalBgColor(backgroundColor()),
    m_state(Empty)
{
    connect(this, &QLineEdit::textChanged, this, &ValidatedLineEdit::onTextChanged);
}

ValidatedLineEdit::~ValidatedLineEdit()
{}

bool ValidatedLineEdit::isValid() const
{
    return m_state == Valid;
}

void ValidatedLineEdit::onTextChanged(const QString &text)
{
    InputState newstate = Invalid;
    if(text.isEmpty())
        newstate = Empty;
    else
        newstate = validate(text) ? Valid : Invalid;

    if(newstate != m_state)
    {
        m_state = newstate;
        setTextColor(m_state != Invalid ? m_normalTextColor : m_errorTextColor);
        setBackgroundColor(m_state != Invalid ? m_normalBgColor : m_errorBgColor);
        emit validityChanged(isValid());
    }
    setToolTip(toolTipText(text));
}

bool ValidatedLineEdit::validate(const QString &input) const
{
    return true;
}

QString ValidatedLineEdit::toolTipText(const QString &input) const
{
    QString ret;
    //TODO implement properly XXX FIXME
    switch(m_state) {
    case Empty:
        ret = tr("Please insert ...");
        break;
    case Valid:
        ret = QString();
        break;
    case Invalid:
        ret = QString("Input <b>%1</b> is not valid!").arg(input);
        break;
    }

    return ret;
}

QColor ValidatedLineEdit::textColor()
{
    return palette().color(QPalette::Active, QPalette::Text);
}

void ValidatedLineEdit::setTextColor(const QColor &color)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Active, QPalette::Text, color);
    this->setPalette(palette);
}

QColor ValidatedLineEdit::backgroundColor()
{
    return palette().color(QPalette::Active, QPalette::Base);
}

void ValidatedLineEdit::setBackgroundColor(const QColor &color)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Active, QPalette::Base, color);
    this->setPalette(palette);
}

QColor ValidatedLineEdit::errorColor() const
{
    return m_errorTextColor;
}

void ValidatedLineEdit::setErrorColor(const QColor &color)
{
    m_errorTextColor = color;
}

QColor ValidatedLineEdit::errorBgColor() const
{
    return m_errorBgColor;
}

void ValidatedLineEdit::setErrorBgColor(const QColor &color)
{
    m_errorBgColor = color;
}

