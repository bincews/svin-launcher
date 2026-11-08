// SPDX-License-Identifier: GPL-3.0-only

#include "SvinPortableUpdate.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTemporaryDir>

#include "FileSystem.h"
#include "archive/ArchiveReader.h"

namespace {
const QString packageRoot = QStringLiteral("Svin-Launcher-Windows-x64/");
const QString launcherName = QStringLiteral("svinlauncher.exe");

bool packageFile(const QString& path)
{
    static const QSet<QString> files = {
        launcherName, "svinlauncher_updater.exe", "svinlauncher_filelink.exe", "portable.txt",
        "qt.conf", "qtlogging.ini", "LICENSE", "COPYING.md", "LOCAL-FORK.md", "SVIN-BRANDING.md",
        "HELP.md", QStringLiteral("ПРОЧТИ-МЕНЯ.txt"),
    };
    static const QRegularExpression runtime(
        R"(\A(?:(?:Qt6[A-Za-z0-9]+|(?:msvcp|vcruntime|vccorlib|concrt)[0-9A-Za-z_]+)\.dll|(?:iconengines|imageformats|platforms|styles|tls)/q[A-Za-z0-9_-]+\.dll|jars/(?:JavaCheck|NewLaunch|NewLaunchLegacy)\.jar)\z)");
    return files.contains(path) || runtime.match(path).hasMatch();
}

bool hasRuntime(const QSet<QString>& files)
{
    for (const QString& name : { launcherName, QStringLiteral("svinlauncher_updater.exe"), QStringLiteral("svinlauncher_filelink.exe"),
                                 QStringLiteral("portable.txt"), QStringLiteral("Qt6Core.dll"), QStringLiteral("Qt6Gui.dll"),
                                 QStringLiteral("Qt6Widgets.dll"), QStringLiteral("Qt6Network.dll"), QStringLiteral("platforms/qwindows.dll") }) {
        if (!files.contains(name))
            return false;
    }
    return true;
}

Result<> replaceFile(const QString& path, const QByteArray& contents)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(contents) != contents.size() || !file.commit())
        return std::unexpected(QString("Could not replace %1: %2").arg(path, file.errorString()));
    return {};
}

bool linked(const QFileInfo& path)
{
    return path.isSymLink() || path.isJunction();
}

bool validPayloadId(const QString& label)
{
    static const QRegularExpression pattern(R"(\Asvin-payload-sha256:[a-f0-9]{64}\z)");
    return pattern.match(label).hasMatch();
}
}

namespace SvinPortableUpdate {
QString ReleaseVersion::identifier() const
{
    const auto tag = QStringLiteral("svin-r%1").arg(release);
    return revision > 0 ? tag + QStringLiteral(".%1").arg(revision) : tag;
}

ReleaseVersion parseVersion(const QString& identifier)
{
    static const QRegularExpression pattern(R"(\Asvin-r([1-9][0-9]{0,8})(?:\.([1-9][0-9]{0,8}))?\z)");
    const auto match = pattern.match(identifier);
    return { match.captured(1).toInt(), match.captured(2).toInt() };
}

ReleaseVersion assetVersion(const QString& releaseTag, const QString& assetName)
{
    const auto tag = parseVersion(releaseTag);
    const auto prefix = QStringLiteral("Svin-Launcher-Windows-x64-");
    if (!tag.isValid() || tag.revision != 0 || !assetName.startsWith(prefix) || !assetName.endsWith(".zip"))
        return {};
    const auto version = parseVersion(QStringLiteral("svin-") + assetName.sliced(prefix.size()).chopped(4));
    return version.release == tag.release ? version : ReleaseVersion{};
}

bool validArchiveDigest(const QString& digest)
{
    static const QRegularExpression pattern(R"(\Asha256:[a-f0-9]{64}\z)");
    return pattern.match(digest).hasMatch();
}

bool matchesPayload(const QByteArray& installed, const QString& label)
{
    return validPayloadId(label) && installed.split('\n').contains(label.toLatin1());
}

bool isReplacement(const QByteArray& installed, const QString& digest, const QString& label)
{
    if (!validArchiveDigest(digest) || matchesPayload(installed, label))
        return false;
    bool hasPayloadId = false;
    for (const auto& line : installed.split('\n')) {
        const auto id = QString::fromLatin1(line);
        if (validArchiveDigest(id))
            return id != digest;
        hasPayloadId |= validPayloadId(id);
    }
    return hasPayloadId && validPayloadId(label);
}

Result<> recordArchive(const QDir& installation, const QString& digest)
{
    const auto marker = installation.filePath("portable.txt");
    const QFileInfo info(marker);
    if (!validArchiveDigest(digest) || !info.isFile() || linked(info))
        return std::unexpected(QStringLiteral("Could not record the Svin archive checksum"));

    TRY_INTO(const auto current, FS::read(marker))
    QByteArray contents;
    for (const auto& line : current.split('\n')) {
        if (!line.isEmpty() && !validArchiveDigest(QString::fromLatin1(line)))
            contents += line + '\n';
    }
    contents += digest.toLatin1() + '\n';
    if (contents == current)
        return {};
    return replaceFile(marker, contents);
}

Result<> unpack(const QString& archivePath, const QDir& staging)
{
    MMCZip::ArchiveReader archive(archivePath);
    QSet<QString> files;
    QSet<QString> windowsNames;
    const auto extract = [&](MMCZip::ArchiveReader::File* entry) -> Result<> {
        const auto name = entry->filename();
        if (!name.startsWith(packageRoot))
            return std::unexpected(QString("Unexpected archive entry: %1").arg(name));
        const auto path = name.sliced(packageRoot.size());
        if (!entry->isFile() && (path.isEmpty() || path == "iconengines/" || path == "imageformats/" || path == "platforms/" ||
                                 path == "styles/" || path == "tls/" || path == "jars/"))
            return entry->skip();
        const auto windowsName = path.toLower();
        if (!entry->isFile() || !packageFile(path) || windowsNames.contains(windowsName))
            return std::unexpected(QString("Unexpected archive entry: %1").arg(name));
        TRY_INTO(const auto contents, entry->readAll())
        if (contents.isEmpty() && path != "portable.txt")
            return std::unexpected(QString("Empty archive entry: %1").arg(name));
        const auto out = staging.filePath(path);
        if (!FS::ensureFilePathExists(out))
            return std::unexpected(QString("Could not create the staging folder for %1").arg(path));
        TRY(replaceFile(out, contents))
        files.insert(path);
        windowsNames.insert(windowsName);
        return {};
    };
    TRY(archive.parse(extract))
    if (!hasRuntime(files))
        return std::unexpected(QStringLiteral("The release archive is missing launcher files"));
    return {};
}

Result<> install(const QDir& staging, const QDir& target)
{
    if (!target.exists() || !target.exists(launcherName) || !target.exists("portable.txt") ||
        staging.canonicalPath() == target.canonicalPath())
        return std::unexpected(QStringLiteral("The update target is not a portable Svin installation"));

    QSet<QString> files;
    QDirIterator iterator(staging.absolutePath(), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const auto path = staging.relativeFilePath(iterator.next());
        const QFileInfo destination(target.filePath(path));
        if (!packageFile(path) || linked(iterator.fileInfo()) || linked(destination) ||
            linked(QFileInfo(destination.absolutePath())) || (destination.exists() && !destination.isFile()))
            return std::unexpected(QString("Invalid update destination: %1").arg(destination.filePath()));
        files.insert(path);
    }
    if (!hasRuntime(files))
        return std::unexpected(QStringLiteral("The staged update is missing launcher files"));

    QTemporaryDir backup(target.filePath(".svin-update-backup-XXXXXX"));
    if (!backup.isValid())
        return std::unexpected(QStringLiteral("Could not create a backup in the launcher folder"));

    auto paths = files.values();
    paths.removeOne(launcherName);
    paths.append(launcherName);
    struct Change {
        QString path;
        bool existed;
    };
    QList<Change> changes;
    const auto applyFile = [&](const QString& path) -> Result<> {
        const auto destination = target.filePath(path);
        TRY_INTO(const auto contents, FS::read(staging.filePath(path)))
        const bool existed = QFileInfo::exists(destination);
        if (existed) {
            TRY_INTO(const auto old, FS::read(destination))
            if (contents == old)
                return {};
            const auto saved = QDir(backup.path()).filePath(path);
            if (!FS::ensureFilePathExists(saved) || !QFile::copy(destination, saved))
                return std::unexpected(QString("Could not back up %1").arg(destination));
        }
        if (!FS::ensureFilePathExists(destination))
            return std::unexpected(QString("Could not create the destination folder for %1").arg(path));
        TRY(replaceFile(destination, contents))
        changes.append({ path, existed });
        return {};
    };

    for (const auto& path : paths) {
        const auto result = applyFile(path);
        if (result)
            continue;
        bool restored = true;
        for (auto change = changes.crbegin(); change != changes.crend(); ++change) {
            const auto destination = target.filePath(change->path);
            if (change->existed) {
                const auto saved = FS::read(QDir(backup.path()).filePath(change->path));
                restored &= saved && replaceFile(destination, *saved).has_value();
            } else {
                restored &= QFile::remove(destination);
            }
        }
        if (!restored) {
            backup.setAutoRemove(false);
            return std::unexpected(QString("%1\nBackup: %2").arg(result.error(), backup.path()));
        }
        return result;
    }
    return {};
}
}
