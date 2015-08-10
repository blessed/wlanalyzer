#include <QFileDialog>
#include <QCompleter>
#include <QDirModel>
#include <QStringListModel>
#include <QDebug>
#include "gui/editsessiondialog.h"
#include "ui_editsessiondialog.h"
#include "gui/editsessiondialog.moc"

namespace {
    QStringList linesFromTextEdit(const QPlainTextEdit* edit)
    {
        static QRegExp reg("\n|\r\n|\r");
        return edit->toPlainText().split(reg, QString::SkipEmptyParts);
    }

    void setLinesToTextEdit(QStringList& lines, QPlainTextEdit* edit)
    {
        edit->setPlainText("");
        for(auto s: lines)
            edit->appendPlainText(s);
    }
};

EditSessionDialog::EditSessionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSessionDialog)
{
    m_sessionInfo = EditSessionDialog::session_ptr::create();
    m_extensions = new QStringListModel(this);

    ui->setupUi(this);
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->sessionName_edit->setValidateFunction([](const QString &text, QString &error)
    {
        bool valid = !text.trimmed().isEmpty();
        if(!valid)
            error = QString(tr("Session name cannot be empty"));
        return valid;
    });
    ui->socketpath_edit->setCompleter(completer);
    ui->binaryLocation_edit->setCompleter(completer);
    ui->coreProtocolLocation_edit->setCompleter(completer);
    ui->protocolExtensions_listView->setModel(m_extensions);

    ui->socketpath_edit->setValidateFunction([](const QString &text, QString &error)
    {
        auto fi = QFileInfo(text.trimmed());
        bool valid = fi.isReadable() && fi.isWritable() && fi.isFile();
        if(!valid)
            error = QString(tr("<b>%1</b> is not an RW socket")).arg(text.trimmed());
        return valid;
    });

    ui->binaryLocation_edit->setValidateFunction([](const QString &text, QString &error)
    {
        auto fi = QFileInfo(text.trimmed());
        bool valid = fi.isExecutable() && fi.isFile();
        if(!valid)
            error = QString(tr("<b>%1</b> is not an executable file")).arg(text.trimmed());
        return valid;
    });

    ui->coreProtocolLocation_edit->setValidateFunction([](const QString &text, QString &error)
    {
        auto fi = QFileInfo(text.trimmed());
        bool valid = fi.isReadable() && fi.isFile() && fi.suffix().toLower() == "xml";
        if(!valid)
            error = QString(tr("<b>%1</b> is not a readable xml file")).arg(text.trimmed());
        return valid;
    });
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
    m_sessionInfo->m_socketPath = ui->socketpath_edit->text();
    m_sessionInfo->m_binaryPath = ui->binaryLocation_edit->text();

    m_sessionInfo->m_commandLine = linesFromTextEdit(ui->commandLine_edit);
    m_sessionInfo->m_environmentVars = linesFromTextEdit(ui->environmentVars_edit);
    m_sessionInfo->m_coreProtocolSpecPath = ui->coreProtocolLocation_edit->text();
    m_sessionInfo->m_protocolExtensionSpecPaths = m_extensions->stringList();

    return m_sessionInfo;
}

void EditSessionDialog::setSessionInfo(EditSessionDialog::session_ptr session)
{
    if(!session || session == m_sessionInfo)
        return;

    m_sessionInfo = session;
    ui->sessionName_edit->setText(m_sessionInfo->m_sessionName);
    ui->socketpath_edit->setText(m_sessionInfo->m_socketPath);
    ui->binaryLocation_edit->setText(m_sessionInfo->m_binaryPath);

    setLinesToTextEdit(m_sessionInfo->m_commandLine, ui->commandLine_edit);
    setLinesToTextEdit(m_sessionInfo->m_environmentVars, ui->environmentVars_edit);

    ui->coreProtocolLocation_edit->setText(m_sessionInfo->m_coreProtocolSpecPath);

    m_extensions->setStringList(m_sessionInfo->m_protocolExtensionSpecPaths);
}

bool EditSessionDialog::isValid()
{
    return ui->sessionName_edit->isValid()
        && ui->socketpath_edit->isValid()
        && ui->binaryLocation_edit->isValid()
        && ui->coreProtocolLocation_edit->isValid();
        // TODO add validation for ui->protocolExtensions_listView;
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

void EditSessionDialog::addExtensionsSlot()
{
    QStringList ret = selectFilePath(tr("Select Protocol Extensions"),
            ui->coreProtocolLocation_edit->text(), "*.xml", true);
    if(!ret.empty())
    {
        qDebug() << "selected extensions:" << ret;
        int num_rows = m_extensions->rowCount();
        m_extensions->insertRows(num_rows, ret.length());
        for(int i=0; i < ret.length(); ++i)
        {
            m_extensions->setData(m_extensions->index(num_rows + i), ret[i]);
        }
    }
}

void EditSessionDialog::removeExtensionsSlot()
{
    auto selection_model = ui->protocolExtensions_listView->selectionModel();
    auto indexes = selection_model->selectedIndexes();
    while(!indexes.isEmpty())
    {
        m_extensions->removeRow(indexes.first().row());
        indexes = selection_model->selectedIndexes();
    }
}

void EditSessionDialog::validityChangedSlot()
{
    ui->start_button->setEnabled(isValid());
}
