#include <QSettings>
#include "sessioninfo.h"

namespace {
    const QString KEY_SESSION_NAME("session/name");
    const QString KEY_SOCKET_PATH("session/socket/path");
    const QString KEY_BINARY_PATH("session/binary/path");
    const QString KEY_BINARY_COMMAND_LINE("session/binary/command_line");
    const QString KEY_CORE_SPEC_PATH("session/protocol/core_spec_path");
    const QString KEY_EXTENSION_SPEC_PATHS("session/protocol/extension_spec_paths");
};

namespace Session {

bool Info::isValid()
{
//TODO Session::Info proper validation
    return true;
}

void Info::Save(QSettings& settings)
{
    settings.setValue(KEY_SESSION_NAME, m_sessionName);
    settings.setValue(KEY_SOCKET_PATH, m_socketPath);
    settings.setValue(KEY_BINARY_PATH, m_binaryPath);
    settings.setValue(KEY_BINARY_COMMAND_LINE, m_commandLine);
    settings.setValue(KEY_CORE_SPEC_PATH, m_coreProtocolSpecPath);
    settings.setValue(KEY_EXTENSION_SPEC_PATHS, m_protocolExtensionSpecPaths);
}

Info::ptr_t Info::FromSettings(QSettings& settings)
{
    auto ret = Info::ptr_t::create();
    ret->m_sessionName = settings.value(KEY_SESSION_NAME).toString();
    ret->m_socketPath = settings.value(KEY_SOCKET_PATH).toString();
    ret->m_binaryPath = settings.value(KEY_BINARY_PATH).toString();
    ret->m_commandLine = settings.value(KEY_BINARY_COMMAND_LINE).toStringList();
    ret->m_coreProtocolSpecPath = settings.value(KEY_CORE_SPEC_PATH).toString();
    ret->m_protocolExtensionSpecPaths = settings.value(KEY_EXTENSION_SPEC_PATHS).toStringList();

    return ret;
}

};
