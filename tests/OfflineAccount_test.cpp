// SPDX-License-Identifier: GPL-3.0-only
#include <QTemporaryDir>
#include <QBuffer>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

#include "LaunchController.h"
#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/AuthSession.h"
#include "minecraft/auth/MinecraftAccount.h"

class SkinReply : public QNetworkReply {
   public:
    SkinReply(const QNetworkRequest& request, const QByteArray& png, QObject* parent) : QNetworkReply(parent)
    {
        setUrl(request.url());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
        m_png.setData(png);
        m_png.open(QIODevice::ReadOnly);
        open(QIODevice::ReadOnly);
        QTimer::singleShot(0, this, [this] {
            setFinished(true);
            emit readyRead();
            emit finished();
        });
    }

    void abort() override {}
    qint64 bytesAvailable() const override { return m_png.bytesAvailable() + QNetworkReply::bytesAvailable(); }

   protected:
    qint64 readData(char* data, qint64 size) override { return m_png.read(data, size); }

   private:
    QBuffer m_png;
};

class SkinNetwork : public QNetworkAccessManager {
   public:
    QByteArray png;
    QList<QNetworkRequest> requests;

   protected:
    QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
    {
        requests.append(request);
        return new SkinReply(request, png, this);
    }
};

class OfflineAccountTest : public QObject {
    Q_OBJECT
   private slots:
    void launchMode_data()
    {
        QTest::addColumn<bool>("offlineProfile");
        QTest::addColumn<bool>("microsoftProfile");
        QTest::addColumn<LaunchMode>("requested");
        QTest::addColumn<LaunchMode>("expected");
        QTest::newRow("offline-normal") << true << false << LaunchMode::Normal << LaunchMode::Offline;
        QTest::newRow("offline-explicit") << true << false << LaunchMode::Offline << LaunchMode::Offline;
        QTest::newRow("offline-demo") << true << false << LaunchMode::Demo << LaunchMode::Demo;
        QTest::newRow("no-account") << false << false << LaunchMode::Normal << LaunchMode::Demo;
        QTest::newRow("microsoft-no-entitlement") << false << true << LaunchMode::Normal << LaunchMode::Demo;
    }

    void launchMode()
    {
        QFETCH(bool, offlineProfile);
        QFETCH(bool, microsoftProfile);
        QFETCH(LaunchMode, requested);
        QFETCH(LaunchMode, expected);
        LaunchController controller;
        if (offlineProfile)
            controller.setAccountToUse(MinecraftAccount::createOffline("LocalPlayer"));
        else if (microsoftProfile)
            controller.setAccountToUse(MinecraftAccount::createBlankMSA());
        controller.setLaunchMode(requested);
        QCOMPARE(controller.decideLaunchMode(), LaunchDecision::Continue);
        QCOMPARE(controller.m_actualLaunchMode, expected);
    }

    void localProfileDownloads_data()
    {
        QTest::addColumn<LaunchMode>("requested");
        QTest::addColumn<bool>("downloadsAllowed");
        QTest::newRow("ordinary-launch-downloads-libraries") << LaunchMode::Normal << true;
        QTest::newRow("explicit-offline-uses-cache-only") << LaunchMode::Offline << false;
    }

    void localProfileDownloads()
    {
        QFETCH(LaunchMode, requested);
        QFETCH(bool, downloadsAllowed);
        LaunchController controller;
        controller.setAccountToUse(MinecraftAccount::createOffline("LocalPlayer"));
        controller.setLaunchMode(requested);
        QCOMPARE(controller.decideLaunchMode(), LaunchDecision::Continue);
        const auto session = controller.createSession();
        QCOMPARE(session->launchMode, LaunchMode::Offline);
        QCOMPARE(session->canDownloadGameFiles(), downloadsAllowed);
        QCOMPARE(session->access_token, QString("0"));
        QCOMPARE(session->player_name, QString("LocalPlayer"));
    }

    void existingDownloadPolicyIsUnchanged()
    {
        AuthSession session;
        session.launchMode = LaunchMode::Normal;
        QVERIFY(session.canDownloadGameFiles());
        session.launchMode = LaunchMode::Demo;
        QVERIFY(session.canDownloadGameFiles());
        session.launchMode = LaunchMode::Offline;
        QVERIFY(!session.canDownloadGameFiles());
    }

    void localProfileNeedsNoMicrosoftAccount()
    {
        AccountList accounts;
        QVERIFY(!accounts.anyAccountIsValid());
        auto offline = MinecraftAccount::createOffline("LocalPlayer");
        accounts.addAccount(offline);
        QVERIFY(accounts.anyAccountIsValid());
        QVERIFY(!offline->ownsMinecraft());
        accounts.removeAccount(accounts.index(0));
        QVERIFY(!accounts.anyAccountIsValid());
    }

    void microsoftEntitlementsAreUnchanged()
    {
        AccountList accounts;
        auto account = MinecraftAccount::createBlankMSA();
        accounts.addAccount(account);
        QVERIFY(!accounts.anyAccountIsValid());
        account->accountData()->minecraftEntitlement.ownsMinecraft = true;
        QVERIFY(accounts.anyAccountIsValid());
    }

    void localProfileSurvivesRestart()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("accounts.json");
        const auto offline = MinecraftAccount::createOffline("LocalPlayer");
        {
            AccountList accounts;
            accounts.setListFilePath(path);
            accounts.addAccount(offline);
            accounts.setDefaultAccount(offline);
            QVERIFY(accounts.saveList());
        }
        AccountList restored;
        restored.setListFilePath(path);
        QVERIFY(restored.loadList());
        QCOMPARE(restored.count(), 1);
        QVERIFY(restored.anyAccountIsValid());
        QVERIFY(restored.defaultAccount());
        QCOMPARE(restored.defaultAccount()->accountType(), AccountType::Offline);
        auto session = std::make_shared<AuthSession>();
        session->launchMode = LaunchMode::Offline;
        restored.defaultAccount()->fillSession(session);
        QCOMPARE(session->player_name, QString("LocalPlayer"));
        QCOMPARE(session->uuid, offline->profileId());
        QCOMPARE(session->access_token, QString("0"));
        QCOMPARE(session->launchMode, LaunchMode::Offline);
    }

    void offlineSkinSurvivesRestart()
    {
        const auto offline = MinecraftAccount::createOffline("LocalPlayer");
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        QByteArray png;
        QBuffer buffer(&png);
        QVERIFY(buffer.open(QIODevice::WriteOnly));
        QVERIFY(image.save(&buffer, "PNG"));
        QSignalSpy changed(offline.get(), &MinecraftAccount::changed);
        QVERIFY(!offline->hasOfflineSkin());
        QVERIFY(offline->setOfflineSkin(png));
        QVERIFY(offline->hasOfflineSkin());
        QVERIFY(offline->setOfflineSkin(png));
        QCOMPARE(changed.size(), 1);
        QVERIFY(!offline->setOfflineSkin("broken skin"));
        QVERIFY(!MinecraftAccount::createBlankMSA()->setOfflineSkin(png));

        const auto restored = MinecraftAccount::loadFromJsonV3(offline->saveToJson());
        QVERIFY(restored);
        QVERIFY(restored->hasOfflineSkin());
        QCOMPARE(restored->accountData()->minecraftProfile.skin.data, png);
        QCOMPARE(restored->profileId(), offline->profileId());
    }

    void refreshesCachedElyAvatarAtStartup()
    {
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        QByteArray previousPng;
        QBuffer previous(&previousPng);
        QVERIFY(previous.open(QIODevice::WriteOnly));
        QVERIFY(image.save(&previous, "PNG"));
        auto offline = MinecraftAccount::createOffline("LocalPlayer");
        QVERIFY(offline->setOfflineSkin(previousPng));

        SkinNetwork network;
        image.fill(Qt::green);
        QBuffer updated(&network.png);
        QVERIFY(updated.open(QIODevice::WriteOnly));
        QVERIFY(image.save(&updated, "PNG"));

        AccountList accounts;
        accounts.addAccount(offline);
        accounts.addAccount(MinecraftAccount::createOffline("NoElyAvatar"));
        QSignalSpy changed(&accounts, &AccountList::dataChanged);
        accounts.startElySkinRefresh(&network, "Svin test");
        QTRY_COMPARE(offline->accountData()->minecraftProfile.skin.data, network.png);
        QCOMPARE(changed.size(), 1);
        QCOMPARE(network.requests.size(), 1);
        QCOMPARE(network.requests.first().url(), QUrl("https://skinsystem.ely.by/skins/LocalPlayer.png"));
        QCOMPARE(network.requests.first().rawHeader("Cache-Control"), QByteArray("no-cache"));
    }
};

QTEST_GUILESS_MAIN(OfflineAccountTest)
#include "OfflineAccount_test.moc"
