#ifndef SESSIONINFO_H
#define SESSIONINFO_H

#include<QSharedPointer>
#include<QString>
#include<QStringList>

class QSettings;

namespace Session {

class Info
{
public:
    typedef QSharedPointer<Info> ptr_t;
    void Save(QSettings& settings);
    bool isValid();

    static Info::ptr_t FromSettings(QSettings& settings);

    QString m_sessionName;
    QString m_socketPath;
    QString m_binaryPath;
    QStringList m_commandLine;
    QStringList m_environmentVars;
    QString m_coreProtocolSpecPath;
    QStringList m_protocolExtensionSpecPaths;
};


};
#endif // SESSIONINFO_H
