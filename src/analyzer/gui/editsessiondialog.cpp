#include "gui/editsessiondialog.h"
#include "ui_editsessiondialog.h"
#include "gui/editsessiondialog.moc"

EditSessionDialog::EditSessionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSessionDialog)
{
    ui->setupUi(this);
}

EditSessionDialog::~EditSessionDialog()
{
    delete ui;
}
