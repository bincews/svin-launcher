#pragma once

#include <QString>
#include <memory>

#include "LaunchMode.h"

class MinecraftAccount;

struct AuthSession {
    bool MakeOffline(QString offline_playername);
    void MakeDemo(QString name, QString uuid);

    QString serializeUserProperties();

    bool canDownloadGameFiles() const { return launchMode != LaunchMode::Offline || allowOnlineDownloads; }

    // combined session ID
    QString session;
    // volatile auth token
    QString access_token;
    // profile name
    QString player_name;
    // profile ID
    QString uuid;
    // 'msa' or 'offline', depending on account type
    QString user_type;
    // the actual launch mode for this session
    LaunchMode launchMode;
    // A local profile can download game files without Microsoft authentication.
    bool allowOnlineDownloads = false;
};

using AuthSessionPtr = std::shared_ptr<AuthSession>;
