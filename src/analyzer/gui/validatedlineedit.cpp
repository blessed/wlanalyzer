#include <QDebug>
#include <QValidator>
#include "gui/validatedlineedit.h"
#include "gui/validatedlineedit.moc"

ValidatedLineEdit::ValidatedLineEdit(QWidget* parent)
    :QLineEdit(parent),
    m_errorTextColor(Qt::red),
    m_errorBgColor(QColor("#FFFFD5")),
    m_normalTextColor(textColor()),
    m_normalBgColor(backgroundColor()),
    m_state(Empty),
    m_validateFn(validateFallback())
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
    m_toolTipText.clear();

    newstate = m_validateFn(text, m_toolTipText) ? Valid : Invalid;

    if(newstate != m_state)
    {
        m_state = newstate;
        setTextColor(m_state != Invalid ? m_normalTextColor : m_errorTextColor);
        setBackgroundColor(m_state != Invalid ? m_normalBgColor : m_errorBgColor);
        emit validityChanged(isValid());
    }
    setToolTip(m_toolTipText);
}

void ValidatedLineEdit::setValidateFunction(const ValidateFunction_t &fn)
{
    m_validateFn = fn;
    // trigger validation on new validator
    emit textChanged(text());
}

ValidatedLineEdit::ValidateFunction_t ValidatedLineEdit::validateFallback()
{
    using namespace std::placeholders;
    return std::bind(&ValidatedLineEdit::validateWithQValidator, this, _1, _2);
}

bool ValidatedLineEdit::validateWithQValidator(const QString &input, QString &error) const
{
    Q_UNUSED(error);
    if(!this->validator())
        return true;

    auto v = this->validator();
    int pos = this->cursorPosition();
    QString tmp = input;
    return v->validate(tmp, pos) == QValidator::Acceptable;
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

