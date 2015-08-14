#include "validatingproxymodel.h"
#include "validatingproxymodel.moc"

ValidatingProxyModel::ValidatingProxyModel(QObject *parent) :
    QIdentityProxyModel(parent),
    m_state(validation::Empty)
{
    // default pass-through validation function
    m_validateFn = [] (const QString& in, QString& err) -> bool
    {
        Q_UNUSED(in); Q_UNUSED(err);
        return true;
    };

    connect(this, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)),
            this, SLOT(modelUpdatedSlot()));
    connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(modelUpdatedSlot()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(modelUpdatedSlot()));
}

QVariant ValidatingProxyModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::TextColorRole &&
       role != Qt::BackgroundColorRole &&
       role != Qt::ToolTipRole)
            return QIdentityProxyModel::data(index, role);

    const QString data = sourceModel()->data(index).toString();
    QString errorText;
    bool valid = m_validateFn(data, errorText);

    if(role == Qt::ToolTipRole)
        return errorText;

    if(!valid)
    {
        if(role == Qt::ForegroundRole)
            return m_errorTextColor;

        if(role == Qt::BackgroundColorRole)
            return m_errorBgColor;
    }
    return QIdentityProxyModel::data(index, role);
}

bool ValidatingProxyModel::isValid()
{
    return m_state != validation::Invalid;
}

void ValidatingProxyModel::setErrorColor(const QColor &color)
{
    m_errorTextColor = color;
}

void ValidatingProxyModel::setErrorBgColor(const QColor &color)
{
    m_errorBgColor = color;
}

void ValidatingProxyModel::setValidateFunction(const ValidateFunction_t &fn)
{
    m_validateFn = fn;
}

void ValidatingProxyModel::modelUpdatedSlot()
{
    validation::State_t newState = validation::Valid;

    int rowCount = sourceModel()->rowCount();
    if(rowCount == 0)
    {
        newState = validation::Empty;
    }
    else
    {
        QString err;
        newState = validation::Valid;
        for(int i = 0; i < rowCount; ++i)
        {
            QString data = sourceModel()->index(i, 0).data().toString();
            if(!m_validateFn(data, err))
            {
                newState = validation::Invalid;
                break;
            }
        }
    }

    if(m_state != newState)
    {
        m_state = newState;
        emit validityChanged();
    }
}
