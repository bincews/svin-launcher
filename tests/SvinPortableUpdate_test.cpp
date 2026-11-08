// SPDX-License-Identifier: GPL-3.0-only

#include <QTemporaryDir>
#include <QTest>

#include "FileSystem.h"
#include "updater/prismupdater/SvinPortableUpdate.h"

class SvinPortableUpdateTest : public QObject {
    Q_OBJECT
   private slots:
    void releaseNumbers()
    {
        using SvinPortableUpdate::parseVersion;
        QCOMPARE(parseVersion("svin-r6").release, 6);
        QCOMPARE(parseVersion("svin-r6").revision, 0);
        QCOMPARE(parseVersion("svin-r6.1").identifier(), QString("svin-r6.1"));
        QVERIFY(parseVersion("svin-r6.1") > parseVersion("svin-r6"));
        QVERIFY(parseVersion("svin-r6.10") > parseVersion("svin-r6.9"));
        QVERIFY(parseVersion("svin-r7") > parseVersion("svin-r6.999"));
        QVERIFY(!parseVersion("svin-r6.0").isValid());
        QVERIFY(!parseVersion("svin-r6.1-beta").isValid());
        QVERIFY(!parseVersion("12.0.0").isValid());
    }

    void assetsInOneRelease()
    {
        using SvinPortableUpdate::assetVersion;
        const auto base = assetVersion("svin-r6", "Svin-Launcher-Windows-x64-r6.zip");
        const auto revision = assetVersion("svin-r6", "Svin-Launcher-Windows-x64-r6.2.zip");
        QVERIFY(base.isValid());
        QVERIFY(revision > base);
        QCOMPARE(revision.identifier(), QString("svin-r6.2"));
        QVERIFY(!assetVersion("svin-r5", "Svin-Launcher-Windows-x64-r6.2.zip").isValid());
        QVERIFY(!assetVersion("svin-r6.2", "Svin-Launcher-Windows-x64-r6.2.zip").isValid());
        QVERIFY(!assetVersion("svin-r6", "Svin-Launcher-Windows-arm64-r6.2.zip").isValid());
        QVERIFY(!assetVersion("svin-r6", "Svin-Launcher-Windows-x64-r6.2.exe").isValid());
    }

    void preservesPortableData()
    {
        QTemporaryDir staging;
        QTemporaryDir target;
        QVERIFY(staging.isValid());
        QVERIFY(target.isValid());
        const QDir source(staging.path());
        const QDir destination(target.path());
        QVERIFY(source.mkpath("platforms"));

        for (const auto* file : { "svinlauncher.exe", "svinlauncher_updater.exe", "svinlauncher_filelink.exe", "portable.txt",
                                  "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Network.dll", "platforms/qwindows.dll" })
            QVERIFY(FS::write(source.filePath(file), "new build"));

        QVERIFY(FS::write(destination.filePath("svinlauncher.exe"), "old build"));
        QVERIFY(FS::write(destination.filePath("portable.txt"), ""));
        QVERIFY(FS::write(destination.filePath("accounts.json"), "player data"));

        QVERIFY(FS::write(source.filePath("accounts.json"), "invalid package"));
        QVERIFY(!SvinPortableUpdate::install(source, destination));
        QCOMPARE(FS::read(destination.filePath("svinlauncher.exe")).value_or(""), "old build");
        QVERIFY(QFile::remove(source.filePath("accounts.json")));

        const auto result = SvinPortableUpdate::install(source, destination);
        QVERIFY2(result, result ? "" : qPrintable(result.error()));
        QCOMPARE(FS::read(destination.filePath("svinlauncher.exe")).value_or(""), "new build");
        QCOMPARE(FS::read(destination.filePath("accounts.json")).value_or(""), "player data");
    }

    void detectsReplacedArchive()
    {
        using namespace SvinPortableUpdate;
        const auto originalPayload = QStringLiteral("svin-payload-sha256:") + QString(64, 'a');
        const auto updatedPayload = QStringLiteral("svin-payload-sha256:") + QString(64, 'b');
        const auto originalDigest = QStringLiteral("sha256:") + QString(64, 'c');
        const auto updatedDigest = QStringLiteral("sha256:") + QString(64, 'd');
        const auto freshInstall = originalPayload.toLatin1() + '\n';
        QVERIFY(!isReplacement(freshInstall, originalDigest, originalPayload));
        QVERIFY(isReplacement(freshInstall, updatedDigest, updatedPayload));
        QVERIFY(!isReplacement({}, updatedDigest, updatedPayload));
        QVERIFY(!isReplacement(freshInstall, "invalid", updatedPayload));

        QTemporaryDir install;
        QVERIFY(install.isValid());
        const QDir directory(install.path());
        const auto marker = directory.filePath("portable.txt");
        QVERIFY(FS::write(marker, freshInstall));
        QVERIFY(recordArchive(directory, originalDigest));
        auto recorded = FS::read(marker).value_or(QByteArray{});
        QCOMPARE(recorded, freshInstall + originalDigest.toLatin1() + '\n');
        QVERIFY(!isReplacement(recorded, originalDigest, {}));
        QVERIFY(isReplacement(recorded, updatedDigest, {}));
        QVERIFY(recordArchive(directory, updatedDigest));
        recorded = FS::read(marker).value_or(QByteArray{});
        QCOMPARE(recorded, freshInstall + updatedDigest.toLatin1() + '\n');
        QVERIFY(!isReplacement(recorded, updatedDigest, {}));
    }
};

QTEST_GUILESS_MAIN(SvinPortableUpdateTest)
#include "SvinPortableUpdate_test.moc"
