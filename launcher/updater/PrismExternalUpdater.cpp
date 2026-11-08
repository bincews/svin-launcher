// SPDX-FileCopyrightText: 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2023 Rachel Powers <508861+Ryex@users.noreply.github.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "PrismExternalUpdater.h"

#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>
#include <algorithm>
#include <memory>

#include "StringUtils.h"

#include "Application.h"
#include "BuildConfig.h"

#include "ui/dialogs/UpdateAvailableDialog.h"

class PrismExternalUpdater::Private {
   public:
    QDir appDir;
    QDir dataDir;
    QTimer updateTimer;
    bool allowBeta{};
    bool autoCheck{};
    double updateInterval{};
    QDateTime lastCheck;
    std::unique_ptr<QSettings> settings;

    QWidget* parent{};
    QProcess* checkProcess{};
};

PrismExternalUpdater::PrismExternalUpdater(QWidget* parent, const QString& appDir, const QString& dataDir)
    : priv(new PrismExternalUpdater::Private())
{
    priv->appDir = QDir(appDir);
    priv->dataDir = QDir(dataDir);
    auto settingsFile = priv->dataDir.absoluteFilePath(BuildConfig.LAUNCHER_APP_BINARY_NAME + "_update.cfg");
    priv->settings = std::make_unique<QSettings>(settingsFile, QSettings::Format::IniFormat);
    priv->allowBeta = priv->settings->value("allow_beta", false).toBool();
    priv->autoCheck = priv->settings->value("auto_check", true).toBool();
    bool intervalOk = false;
    // default once per day
    priv->updateInterval = priv->settings->value("update_interval", 86400).toInt(&intervalOk);
    if (!intervalOk) {
        priv->updateInterval = 86400;
    }
    if (const auto lastCheck = priv->settings->value("last_check"); !lastCheck.isNull() && lastCheck.isValid()) {
        priv->lastCheck = QDateTime::fromString(lastCheck.toString(), Qt::ISODate);
    }
    priv->parent = parent;
    connectTimer();
    resetAutoCheckTimer();
    if (priv->autoCheck && priv->updateInterval == 0)
        QTimer::singleShot(0, this, &PrismExternalUpdater::autoCheckTimerFired);
}

PrismExternalUpdater::~PrismExternalUpdater()
{
    if (priv->updateTimer.isActive()) {
        priv->updateTimer.stop();
    }
    disconnectTimer();
    if (auto* process = priv->checkProcess) {
        process->disconnect(this);
        delete process;
    }
    priv->settings->sync();
    delete priv;
}

void PrismExternalUpdater::checkForUpdates()
{
    checkForUpdates(true);
}

void PrismExternalUpdater::checkForUpdates(bool triggeredByUser)
{
    if (priv->checkProcess)
        return;

    auto* process = new QProcess(this);
    priv->checkProcess = process;
    priv->updateTimer.stop();
    QPointer<QProgressDialog> progress;
    if (triggeredByUser) {
        progress = new QProgressDialog(tr("Checking for updates..."), "", 0, 0, priv->parent);
        progress->setMinimumDuration(0);
        progress->setCancelButton(nullptr);
        progress->show();
    }

    auto exeName = QStringLiteral("%1_updater").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
#ifdef Q_OS_WIN32
    exeName.append(".exe");
#else
    exeName.prepend("bin/");
#endif
    QStringList args = { "--check-only", "--dir", priv->dataDir.absolutePath(), "--debug" };
    if (priv->allowBeta)
        args.append("--pre-release");

    const auto complete = [this, process, progress, triggeredByUser](bool failedToStart) {
        if (priv->checkProcess != process)
            return;
        if (progress) {
            progress->close();
            progress->deleteLater();
        }

        const auto stdoutText = process->readAllStandardOutput();
        const auto stderrText = process->readAllStandardError();
        const int code = process->exitCode();
        if (failedToStart || process->exitStatus() != QProcess::NormalExit || (code != 0 && code != 100)) {
            qWarning() << "Update check failed:" << process->errorString() << stderrText;
            if (triggeredByUser) {
                QMessageBox box(QMessageBox::Warning, tr("Update Check Failed"),
                                tr("Could not check for Svin Launcher updates."), QMessageBox::Ok, priv->parent);
                box.setDetailedText(failedToStart ? process->errorString() : QString::fromLocal8Bit(stderrText));
                box.exec();
            }
        } else if (code == 100) {
            const auto [firstLine, rest1] = StringUtils::splitFirst(stdoutText, '\n');
            const auto [secondLine, rest2] = StringUtils::splitFirst(rest1, '\n');
            const auto releaseNotes = StringUtils::splitFirst(rest2, '\n').second;
            auto versionName = StringUtils::splitFirst(firstLine, ": ").second.trimmed();
            const auto versionTag = StringUtils::splitFirst(secondLine, ": ").second.trimmed();
            if (BuildConfig.SVIN_RELEASE > 0 && versionTag.contains('@'))
                versionName = tr("%1 (updated build)").arg(versionName);
            offerUpdate(versionName, versionTag, releaseNotes, triggeredByUser);
        } else if (triggeredByUser) {
            QMessageBox::information(priv->parent, tr("No Update Available"),
                                     tr("You are running the latest version of Svin Launcher (%1).").arg(BuildConfig.printableVersionString()));
        }

        priv->checkProcess = nullptr;
        process->deleteLater();
        priv->lastCheck = QDateTime::currentDateTime();
        priv->settings->setValue("last_check", priv->lastCheck.toString(Qt::ISODate));
        priv->settings->sync();
        resetAutoCheckTimer();
    };

    connect(process, &QProcess::finished, this, [complete](int, QProcess::ExitStatus) { complete(false); });
    connect(process, &QProcess::errorOccurred, this, [complete](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            complete(true);
    });
    QTimer::singleShot(60000, process, [process] {
        if (process->state() != QProcess::NotRunning)
            process->kill();
    });
    process->start(priv->appDir.absoluteFilePath(exeName), args);
}

bool PrismExternalUpdater::getAutomaticallyChecksForUpdates()
{
    return priv->autoCheck;
}

double PrismExternalUpdater::getUpdateCheckInterval()
{
    return priv->updateInterval;
}

bool PrismExternalUpdater::getBetaAllowed()
{
    return priv->allowBeta;
}

void PrismExternalUpdater::setAutomaticallyChecksForUpdates(bool check)
{
    priv->autoCheck = check;
    priv->settings->setValue("auto_check", check);
    priv->settings->sync();
    resetAutoCheckTimer();
}

void PrismExternalUpdater::setUpdateCheckInterval(double seconds)
{
    priv->updateInterval = seconds;
    priv->settings->setValue("update_interval", seconds);
    priv->settings->sync();
    resetAutoCheckTimer();
}

void PrismExternalUpdater::setBetaAllowed(bool allowed)
{
    priv->allowBeta = allowed;
    priv->settings->setValue("auto_beta", allowed);
    priv->settings->sync();
}

void PrismExternalUpdater::resetAutoCheckTimer() const
{
    if (priv->autoCheck && priv->updateInterval > 0) {
        auto now = QDateTime::currentDateTime();

        qint64 timeoutMs = 0;

        if (priv->lastCheck.isValid()) {
            const qint64 diff = priv->lastCheck.secsTo(now);
            const qint64 secsLeft = std::max<qint64>(priv->updateInterval - diff, 0);
            timeoutMs = secsLeft * 1000;
        }

        timeoutMs = std::min(timeoutMs, static_cast<qint64>(INT_MAX));

        qDebug() << "Auto update timer starting," << timeoutMs / 1000 << "seconds left";
        priv->updateTimer.start(static_cast<int>(timeoutMs));
    } else {
        if (priv->updateTimer.isActive()) {
            priv->updateTimer.stop();
        }
    }
}

void PrismExternalUpdater::connectTimer()
{
    connect(&priv->updateTimer, &QTimer::timeout, this, &PrismExternalUpdater::autoCheckTimerFired);
}

void PrismExternalUpdater::disconnectTimer()
{
    disconnect(&priv->updateTimer, &QTimer::timeout, this, &PrismExternalUpdater::autoCheckTimerFired);
}

void PrismExternalUpdater::autoCheckTimerFired()
{
    qDebug() << "Auto update Timer fired";
    checkForUpdates(false);
}

void PrismExternalUpdater::offerUpdate(const QString& versionName,
                                       const QString& versionTag,
                                       const QString& releaseNotes,
                                       const bool triggeredByUser) const
{
    if (!triggeredByUser && !APPLICATION->updatesAreAllowed())
        return;

    priv->settings->beginGroup("skip");
    auto shouldSkip = !triggeredByUser && priv->settings->value(versionTag, false).toBool();
    priv->settings->endGroup();

    if (shouldSkip) {
        if (triggeredByUser) {
            auto msgBox = QMessageBox(QMessageBox::Information, tr("No Update Available"), tr("There are no new updates available."),
                                      QMessageBox::Ok, priv->parent);
            msgBox.setMinimumWidth(460);
            msgBox.adjustSize();
            msgBox.exec();
        }
        return;
    }

    UpdateAvailableDialog dlg(BuildConfig.printableVersionString(), versionName, releaseNotes, priv->parent);

    auto result = dlg.exec();
    qDebug() << "offer dlg result" << result;

    priv->settings->beginGroup("skip");
    if (result == UpdateAvailableDialog::Skip) {
        priv->settings->setValue(versionTag, true);
    } else {
        if (result == UpdateAvailableDialog::Install) {
            performUpdate(versionTag);
        }
        priv->settings->remove(versionTag);
    }
    priv->settings->endGroup();
    priv->settings->sync();
}

void PrismExternalUpdater::performUpdate(const QString& versionTag) const
{
    if (!APPLICATION->updatesAreAllowed()) {
        QMessageBox::information(priv->parent, tr("Update"), tr("Close the running game before updating Svin Launcher."));
        return;
    }
    QProcess proc;
    auto exeName = QStringLiteral("%1_updater").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
#ifdef Q_OS_WIN32
    exeName.append(".exe");

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("__COMPAT_LAYER", "RUNASINVOKER");
    proc.setProcessEnvironment(env);
#else
    exeName = QString("bin/%1").arg(exeName);
#endif

    QStringList args = { "--dir", priv->dataDir.absolutePath(), "--install-version", versionTag };
    if (BuildConfig.SVIN_RELEASE > 0)
        args << "--svin-parent" << QString::number(QCoreApplication::applicationPid());
    if (priv->allowBeta) {
        args.append("--pre-release");
    }

    proc.setProgram(priv->appDir.absoluteFilePath(exeName));
    proc.setArguments(args);
    auto result = proc.startDetached();
    if (!result) {
        qDebug() << "Failed to start updater:" << proc.error() << proc.errorString();
        QMessageBox::warning(priv->parent, tr("Update Failed"), proc.errorString());
        return;
    }
    QCoreApplication::exit();
}
