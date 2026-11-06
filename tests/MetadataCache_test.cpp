// SPDX-License-Identifier: GPL-3.0-only
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "meta/Version.h"
#include "minecraft/VersionFile.h"
#include "minecraft/LaunchProfile.h"
#include "minecraft/Library.h"
#include "minecraft/MinecraftLoadAndCheck.h"

class MetadataCacheTest : public QObject {
    Q_OBJECT
    QString m_originalDirectory;
    QTemporaryDir m_directory;

   private slots:
    void init()
    {
        QVERIFY(m_directory.isValid());
        m_originalDirectory = QDir::currentPath();
        QVERIFY(QDir::setCurrent(m_directory.path()));
        QVERIFY(QDir().mkpath("meta/test.game"));
    }

    void cleanup() { QVERIFY(QDir::setCurrent(m_originalDirectory)); }

    void cachedVersionLoadsOnlyAfterTaskStarts()
    {
        QFile file("meta/test.game/cached.json");
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"({"formatVersion":1,"uid":"test.game","version":"cached","releaseTime":"2025-10-07T00:00:00Z","name":"Test game","mainClass":"net.minecraft.client.main.Main","mainJar":{"name":"com.mojang:minecraft:1.21.10:client"}})");
        file.close();
        Meta::Version version("test.game", "cached");
        const auto task = version.loadTask(Net::Mode::Offline);
        QVERIFY(!version.data());
        QCOMPARE(task->getState(), Task::State::Inactive);
        task->start();
        QVERIFY2(task->wasSuccessful(), qPrintable(task->failReason()));
        QVERIFY(version.data());
        QCOMPARE(version.data()->mainClass, QString("net.minecraft.client.main.Main"));
        QVERIFY(version.data()->mainJar);
    }

    void failedComponentBlocksAnOtherwiseValidProfile()
    {
        LaunchProfile profile;
        profile.applyMainClass("net.minecraft.client.main.Main");
        profile.applyMainJar(std::make_shared<Library>("com.mojang:minecraft:1.21.10:client"));
        QVERIFY(MinecraftLoadAndCheck::validateLaunchProfile(profile));
        // A missing Fabric component leaves the vanilla main class and jar intact.
        profile.applyProblemSeverity(ProblemSeverity::Error);
        const auto result = MinecraftLoadAndCheck::validateLaunchProfile(profile);
        QVERIFY(!result);
        QVERIFY(!result.error().isEmpty());
    }

    void missingOfflineMetadataFailsWithoutNetwork()
    {
        Meta::Version version("test.game", "missing");
        const auto task = version.loadTask(Net::Mode::Offline);
        task->start();
        QCOMPARE(task->getState(), Task::State::Failed);
        QVERIFY(!task->failReason().isEmpty());
        QVERIFY(!version.data());
    }

    void invalidOfflineMetadataFailsWithoutNetwork()
    {
        QFile file("meta/test.game/invalid.json");
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("not JSON");
        file.close();
        Meta::Version version("test.game", "invalid");
        const auto task = version.loadTask(Net::Mode::Offline);
        task->start();
        QCOMPARE(task->getState(), Task::State::Failed);
        QVERIFY(!version.data());
    }
};

QTEST_GUILESS_MAIN(MetadataCacheTest)
#include "MetadataCache_test.moc"
