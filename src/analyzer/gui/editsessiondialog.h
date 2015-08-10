#ifndef EDITSESSIONDIALOG_H
#define EDITSESSIONDIALOG_H

#include <QDialog>
#include "sessioninfo.h"

namespace Ui {
class EditSessionDialog;
}

class QStringListModel;

class EditSessionDialog : public QDialog
{
    Q_OBJECT

    typedef Session::Info::ptr_t session_ptr;
public:
    explicit EditSessionDialog(QWidget *parent = 0);
    ~EditSessionDialog();

    session_ptr getSessionInfo();
    void setSessionInfo(session_ptr session);
    bool isValid();

public slots:
    void browseSocketPathSlot();
    void browseBinaryLocationSlot();
    void browseCoreProtocolSlot();
    void addExtensionsSlot();
    void removeExtensionsSlot();
    void validityChangedSlot();

private:
    Ui::EditSessionDialog *ui;
    session_ptr m_sessionInfo;
    QStringListModel *m_extensions;
};

#endif // EDITSESSIONDIALOG_H
