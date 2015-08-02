#include <QFileDialog>
#include <QDebug>
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

QString getDirPath(const QString& path)
{
    return path.left(path.lastIndexOf(QDir::separator()));
}

QStringList selectFilePath(const QString& title, const QString& directory,
                           const QString filter = QString(),
                           bool multipple_selection = false)
{
    QFileDialog dialog(nullptr, title);
    dialog.setFileMode(multipple_selection ? QFileDialog::ExistingFiles
                                           : QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::ReadOnly);
    dialog.setDirectory(getDirPath(directory));
    dialog.setNameFilter(filter);
    QStringList ret;
    if(dialog.exec())
    {
        ret = dialog.selectedFiles();
    }
    return ret;
}

void EditSessionDialog::browseSocketPathSlot()
{
    QStringList ret = selectFilePath(tr("Select Socket Location"),
            ui->socketpath_edit->text());
    if(!ret.empty())
    {
        qDebug() << "selected socket:" << ret[0];
        ui->socketpath_edit->setText(ret[0]);
    }
}

void EditSessionDialog::browseBinaryLocationSlot()
{
    QStringList ret = selectFilePath(tr("Select application"),
            ui->binaryLocation_edit->text());
    if(!ret.empty())
    {
        qDebug() << "selected application:" << ret[0];
        ui->binaryLocation_edit->setText(ret[0]);
    }
}

void EditSessionDialog::browseCoreProtocolSlot()
{
    QStringList ret = selectFilePath(tr("Select Core Protocol Specification"),
            ui->coreProtocolLocation_edit->text(), "*.xml");
    if(!ret.empty())
    {
        qDebug() << "selected core:" << ret[0];
        ui->coreProtocolLocation_edit->setText(ret[0]);
    }
}

void EditSessionDialog::browseExtensionsSlot()
{
    QStringList ret = selectFilePath(tr("Select Protocol Extensions"),
            ui->coreProtocolLocation_edit->text(), "*.xml", true);
    if(!ret.empty())
    {
        qDebug() << "selected extensions:" << ret;
        ui->protocolExtensions_edit->setPlainText("");
        for(auto s: ret)
            ui->protocolExtensions_edit->appendPlainText(s);
    }
}
