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
        QCOMPARE(SvinPortableUpdate::releaseNumber("svin-r3"), 3);
        QVERIFY(SvinPortableUpdate::releaseNumber("svin-r10") > SvinPortableUpdate::releaseNumber("svin-r9"));
        QCOMPARE(SvinPortableUpdate::releaseNumber("svin-r4-beta"), 0);
        QCOMPARE(SvinPortableUpdate::releaseNumber("12.0.0"), 0);
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
};

QTEST_GUILESS_MAIN(SvinPortableUpdateTest)
#include "SvinPortableUpdate_test.moc"
