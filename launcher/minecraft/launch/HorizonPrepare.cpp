// SPDX-License-Identifier: GPL-3.0-only

#include "HorizonPrepare.h"

#include <algorithm>
#include <sstream>

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include "FileSystem.h"
#include "io/stream_reader.h"
#include "io/stream_writer.h"
#include "launch/LaunchTask.h"
#include "tag_compound.h"
#include "tag_list.h"
#include "tag_string.h"

namespace {
constexpr auto officialAddress = "play.playhorizon.dev";
constexpr auto partnerAddress = "afri-network.co.za";

bool isPresetServer(const nbt::value& entry)
{
    const auto& server = entry.as<nbt::tag_compound>();
    if (!server.has_key("name", nbt::tag_type::String) || !server.has_key("ip", nbt::tag_type::String))
        return false;

    const auto& name = server.at("name").as<nbt::tag_string>().get();
    auto address = QByteArray::fromStdString(server.at("ip").as<nbt::tag_string>().get()).trimmed().toLower();
    if (address.endsWith(":25565"))
        address.chop(6);

    return (name == "Horizon Official" && (address == officialAddress || address == "horizon.bunchy7s.dev")) ||
           (name == "Afri-Network (South Africa)" && address == partnerAddress);
}

Result<> removePresetServers(const QString& path)
{
    if (!QFile::exists(path))
        return {};

    const auto contents = FS::read(path);
    if (!contents)
        return std::unexpected(contents.error());

    try {
        std::istringstream input(contents->toStdString());
        auto [name, data] = nbt::io::read_compound(input);
        if (!data || !data->has_key("servers", nbt::tag_type::List))
            return {};

        auto& servers = data->at("servers").as<nbt::tag_list>();
        if (servers.el_type() != nbt::tag_type::Compound)
            return {};

        const auto kept = std::distance(servers.begin(), std::remove_if(servers.begin(), servers.end(), isPresetServer));
        if (static_cast<size_t>(kept) == servers.size())
            return {};

        while (servers.size() > static_cast<size_t>(kept))
            servers.pop_back();

        std::ostringstream output;
        nbt::io::write_tag(name, *data, output);
        return FS::write(path, QByteArray::fromStdString(output.str()));
    } catch (const std::exception&) {
        return std::unexpected(QStringLiteral("Could not read the Horizon server list."));
    }
}

Result<> markServerOffered(const QDir& game, const QString& marker, const QByteArray& address)
{
    const auto path = game.filePath(marker);
    QByteArray data;
    if (QFile::exists(path)) {
        const auto saved = FS::read(path);
        if (!saved)
            return std::unexpected(saved.error());
        data = *saved;
    }
    for (const auto& line : data.split('\n')) {
        if (line.trimmed().compare(address, Qt::CaseInsensitive) == 0)
            return {};
    }
    if (!data.isEmpty() && !data.endsWith('\n'))
        data += '\n';
    return FS::write(path, data + address + '\n');
}
}  // namespace

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
        // Horizon uses these markers to remember which bundled entries were already offered.
        const QDir game(m_parent->instance()->gameRoot());
        const auto official = FS::write(game.filePath("horizon_official_server.txt"), QByteArray(officialAddress) + '\n');
        const auto partner = markServerOffered(game, "horizon_partner_servers.txt", partnerAddress);
        if (!official || !partner) {
            emitFailed(!official ? official.error() : partner.error());
            return;
        }
        if (const auto cleanup = removePresetServers(game.filePath("servers.dat")); !cleanup)
            emit logLine(cleanup.error(), MessageLevel::Warning);
        emitSucceeded();
    }
}

bool HorizonPrepare::abort()
{
    m_aborted = true;
    m_process.kill();
    return true;
}
