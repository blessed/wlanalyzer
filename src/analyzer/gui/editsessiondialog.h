#ifndef EDITSESSIONDIALOG_H
#define EDITSESSIONDIALOG_H

#include <QDialog>

namespace Ui {
class EditSessionDialog;
}

class EditSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditSessionDialog(QWidget *parent = 0);
    ~EditSessionDialog();

private:
    Ui::EditSessionDialog *ui;
};

#endif // EDITSESSIONDIALOG_H
