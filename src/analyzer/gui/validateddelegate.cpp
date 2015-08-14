#include "validatedlineedit.h"
#include "gui/validateddelegate.h"
#include "gui/validateddelegate.moc"

ValidatedDelegate::ValidatedDelegate(QObject *parent, const ValidateFunction_t &fn)
   : QStyledItemDelegate(parent), m_validateFn(fn), m_completer(nullptr)
{}

QWidget *ValidatedDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    ValidatedLineEdit *editor = new ValidatedLineEdit(parent);
    editor->setErrorColor(m_errorTextColor);
    editor->setErrorBgColor(m_errorBgColor);
    editor->setValidateFunction(m_validateFn);
    editor->setCompleter(m_completer);

    return editor;
}

void ValidatedDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString data = index.model()->data(index, Qt::EditRole).toString();
    ValidatedLineEdit *lineEdit = static_cast<ValidatedLineEdit*>(editor);

    lineEdit->setText(data);
}

void ValidatedDelegate::setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const
{
    ValidatedLineEdit *lineEdit = static_cast<ValidatedLineEdit*>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void ValidatedDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void ValidatedDelegate::setErrorColor(const QColor &color)
{
    m_errorTextColor = color;
}

void ValidatedDelegate::setErrorBgColor(const QColor &color)
{
    m_errorBgColor = color;
}

void ValidatedDelegate::setCompleter(QCompleter *completer)
{
    m_completer = completer;
}
