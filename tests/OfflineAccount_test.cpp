// SPDX-License-Identifier: GPL-3.0-only
#include <QTemporaryDir>
#include <QTest>

#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/AuthSession.h"
#include "minecraft/auth/MinecraftAccount.h"

class OfflineAccountTest : public QObject {
    Q_OBJECT
   private slots:
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
};

QTEST_GUILESS_MAIN(OfflineAccountTest)
#include "OfflineAccount_test.moc"
