#ifndef VALIDATED_LINEEDIT_H
#define VALIDATED_LINEEDIT_H

#include <QLineEdit>

class ValidatedLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor DESIGNABLE true)
    Q_PROPERTY(QColor errorBgColor READ errorBgColor WRITE setErrorBgColor DESIGNABLE true)

public:
    enum InputState {Empty, Invalid, Valid};

    explicit ValidatedLineEdit(QWidget *parent = 0);
    virtual ~ValidatedLineEdit();
    bool isValid() const;

    // properties
    QColor errorColor() const;
    void setErrorColor(const QColor &color);
    QColor errorBgColor() const;
    void setErrorBgColor(const QColor &color);

private:
    // internal color handling
    QColor textColor();
    void setTextColor(const QColor &color);
    QColor backgroundColor();
    void setBackgroundColor(const QColor &color);

    // overrides for inherited classes
    virtual bool validate(const QString &text) const;
    virtual QString toolTipText(const QString &input) const;

    QColor m_errorTextColor;
    QColor m_errorBgColor;
    QColor m_normalTextColor;
    QColor m_normalBgColor;
    InputState m_state;

signals:
    void validityChanged(bool);

private slots:
    void onTextChanged(const QString &text);
};

#endif // VALIDATED_LINEEDIT_H
