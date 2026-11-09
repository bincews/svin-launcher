// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include "launch/LaunchStep.h"

class HorizonPrepare : public LaunchStep {
    Q_OBJECT
   public:
    HorizonPrepare(LaunchTask* parent, bool online);
    void executeTask() override;
    bool abort() override;
    bool canAbort() const override { return true; }

    static QDir directory();

   private:
    enum class Phase { Bootstrap, Check, Prepare };
    void checkResources();
    void prepareResources();
    void readProgress();
    void finished(int code, QProcess::ExitStatus status);

    QProcess m_process;
    QTemporaryFile m_source;
    QDir m_root;
    QString m_java;
    QString m_error;
    Phase m_phase = Phase::Bootstrap;
    bool m_online;
    bool m_aborted = false;
};
