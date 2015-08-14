#ifndef VALIDATED_DELEGATE_H
#define VALIDATED_DELEGATE_H

#include <QStyledItemDelegate>

class QCompleter;

class ValidatedDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    typedef validation::StrCheckFn_t ValidateFunction_t;

    ValidatedDelegate(QObject *parent, const ValidateFunction_t &fn);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void setErrorColor(const QColor &color);
    void setErrorBgColor(const QColor &color);
    void setCompleter(QCompleter *completer);

private:
    QColor m_errorTextColor;
    QColor m_errorBgColor;
    ValidateFunction_t m_validateFn;
    QCompleter *m_completer;
};

#endif // VALIDATED_DELEGATE_H
