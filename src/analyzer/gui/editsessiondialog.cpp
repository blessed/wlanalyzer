#include <QFileDialog>
#include <QCompleter>
#include <QDirModel>
#include <QDebug>
#include "gui/editsessiondialog.h"
#include "ui_editsessiondialog.h"
#include "gui/editsessiondialog.moc"

EditSessionDialog::EditSessionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSessionDialog)
{
    m_sessionInfo = EditSessionDialog::session_ptr::create();
    ui->setupUi(this);
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->socketpath_edit->setCompleter(completer);
    ui->binaryLocation_edit->setCompleter(completer);
    ui->coreProtocolLocation_edit->setCompleter(completer);
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

EditSessionDialog::session_ptr EditSessionDialog::getSessionInfo()
{
    m_sessionInfo->m_sessionName = ui->sessionName_edit->text();
    m_sessionInfo->m_binaryPath = ui->binaryLocation_edit->text();

    m_sessionInfo->m_commandLine = ui->commandLine_edit->toPlainText()
        .split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);

    m_sessionInfo->m_coreProtocolSpecPath = ui->coreProtocolLocation_edit->text();

    m_sessionInfo->m_protocolExtensionSpecPaths = ui->protocolExtensions_edit->toPlainText()
        .split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);

    return m_sessionInfo;
}

void EditSessionDialog::setSessionInfo(EditSessionDialog::session_ptr session)
{
    if(!session || session == m_sessionInfo)
        return;

    m_sessionInfo = session;
    ui->sessionName_edit->setText(m_sessionInfo->m_sessionName);
    ui->binaryLocation_edit->setText(m_sessionInfo->m_binaryPath);

    ui->commandLine_edit->setPlainText("");
    for(auto s: m_sessionInfo->m_commandLine)
        ui->commandLine_edit->appendPlainText(s);

    ui->coreProtocolLocation_edit->setText(m_sessionInfo->m_coreProtocolSpecPath);

    ui->protocolExtensions_edit->setPlainText("");
    for(auto s: m_sessionInfo->m_protocolExtensionSpecPaths)
        ui->protocolExtensions_edit->appendPlainText(s);
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

    QPalette palette = ui->socketpath_edit->palette();
    palette.setColor(QPalette::Active, QPalette::Text, Qt::red);
    palette.setColor(QPalette::Active, QPalette::Base, QColor("#FFFFD5"));
    ui->socketpath_edit->setPalette(palette);
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
