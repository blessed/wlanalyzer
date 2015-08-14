#ifndef VALIDATINGPROXYMODEL_H
#define VALIDATINGPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QColor>
#include "validation.h"

class ValidatingProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    typedef validation::StrCheckFn_t ValidateFunction_t;

    explicit ValidatingProxyModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const override;

    bool isValid();
    void setErrorColor(const QColor &color);
    void setErrorBgColor(const QColor &color);
    void setValidateFunction(const ValidateFunction_t &fn);

signals:
    void validityChanged();

private slots:
    void modelUpdatedSlot();

private:
    QColor m_errorTextColor;
    QColor m_errorBgColor;
    ValidateFunction_t m_validateFn;
    validation::State_t m_state;
};

#endif // VALIDATINGPROXYMODEL_H
