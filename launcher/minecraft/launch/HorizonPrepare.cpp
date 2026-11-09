// SPDX-License-Identifier: GPL-3.0-only

#include "HorizonPrepare.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include "FileSystem.h"
#include "launch/LaunchTask.h"

QDir HorizonPrepare::directory()
{
    return QDir(QDir::current().absoluteFilePath("horizon/26.3"));
}

HorizonPrepare::HorizonPrepare(LaunchTask* parent, bool online)
    : LaunchStep(parent), m_source(QDir::temp().filePath("Svin-Horizon-XXXXXX.java")), m_root(directory()), m_online(online)
{
    connect(&m_process, &QProcess::finished, this, &HorizonPrepare::finished);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &HorizonPrepare::readProgress);
    connect(&m_process, &QProcess::readyReadStandardError, this, [this] {
        const auto output = QString::fromUtf8(m_process.readAllStandardError()).trimmed();
        if (!output.isEmpty() && m_phase != Phase::Check) {
            emit logLine(output, MessageLevel::Warning);
            if (m_error.isEmpty())
                m_error = output.section('\n', 0, 0);
        }
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            emitFailed(tr("Could not start Horizon preparation: %1").arg(m_process.errorString()));
    });
}

void HorizonPrepare::executeTask()
{
#if !defined(Q_OS_WIN) || !defined(Q_PROCESSOR_X86_64)
    emitFailed(tr("This experimental Horizon instance requires Windows x64."));
    return;
#endif
    m_java = FS::ResolveExecutable(m_parent->instance()->settings()->get("JavaPath").toString());
    if (m_root.exists("client/horizon-client.exe") && m_root.exists("client/horizon-prepare.exe") &&
        m_root.exists("minecraft-26.3.json") && m_root.exists("server-26.3.jar")) {
        checkResources();
        return;
    }
    if (!m_online) {
        emitFailed(tr("Start Horizon online once to download its files."));
        return;
    }
    QFile source(":/documents/HorizonBootstrap.java");
    if (!source.open(QIODevice::ReadOnly) || !m_source.open()) {
        emitFailed(tr("Could not open the Horizon download helper."));
        return;
    }
    const auto contents = source.readAll();
    if (m_source.write(contents) != contents.size() || !m_source.flush()) {
        emitFailed(tr("Could not save the Horizon download helper."));
        return;
    }
    m_source.close();
    setStatus(tr("Downloading Horizon 26.3…"));
    m_process.start(m_java, { "--source", "25", m_source.fileName(), m_root.absolutePath(), "online" });
    emit progressReportingRequest();
}

void HorizonPrepare::checkResources()
{
    m_phase = Phase::Check;
    m_process.setWorkingDirectory(m_root.filePath("client"));
    m_process.start(m_root.filePath("client/horizon-prepare.exe"),
                    { "--check", "--root", m_root.filePath("client"), "--output", m_root.filePath("resources") });
}

void HorizonPrepare::prepareResources()
{
    m_phase = Phase::Prepare;
    auto* instance = m_parent->instance();
    const auto libraries = instance->librariesPath();
    const auto javaHome = QFileInfo(m_java).dir().absoluteFilePath("..");
    setStatus(tr("Preparing Horizon resources…"));
    m_process.start(m_root.filePath("client/horizon-prepare.exe"),
                    { "--prepare", "--root", m_root.filePath("client"), "--output", m_root.filePath("resources"),
                      "--version-json", m_root.filePath("minecraft-26.3.json"), "--server-jar", m_root.filePath("server-26.3.jar"),
                      "--client-jar", libraries.filePath("com/mojang/minecraft/26.3/minecraft-26.3-client.jar"),
                      "--libraries-dir", libraries.absolutePath(), "--assets-dir", QDir("assets").absolutePath(),
                      "--java-home", javaHome, "--complete", "--progress", "json" });
    emit progressReportingRequest();
}

void HorizonPrepare::readProgress()
{
    while (m_process.canReadLine()) {
        const auto line = m_process.readLine().trimmed();
        if (m_phase == Phase::Check || line.isEmpty())
            continue;
        const auto progress = QJsonDocument::fromJson(line).object();
        if (progress.isEmpty()) {
            emit logLine(QString::fromUtf8(line), MessageLevel::Launcher);
            continue;
        }
        if (const auto message = progress.value("message").toString(); !message.isEmpty())
            setStatus(message);
        if (const auto fraction = progress.value("fraction"); fraction.isDouble())
            setProgress(qBound(0, qRound(fraction.toDouble() * 100), 100), 100);
        if (const auto error = progress.value("error").toString(); !error.isEmpty())
            m_error = error;
    }
}

void HorizonPrepare::finished(int code, QProcess::ExitStatus status)
{
    readProgress();
    if (m_aborted) {
        emitAborted();
    } else if (status == QProcess::NormalExit && m_phase == Phase::Check && code == 1) {
        prepareResources();
    } else if (status != QProcess::NormalExit || code != 0) {
        emitFailed(m_error.isEmpty() ? tr("Horizon preparation failed (exit code %1).").arg(code) : m_error);
    } else if (m_phase == Phase::Bootstrap) {
        checkResources();
    } else {
        emitSucceeded();
    }
}

bool HorizonPrepare::abort()
{
    m_aborted = true;
    m_process.kill();
    return true;
}
