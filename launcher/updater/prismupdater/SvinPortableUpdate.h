// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDir>
#include <QString>
#include <compare>

#include "Result.h"

namespace SvinPortableUpdate {
struct ReleaseVersion {
    int release = 0;
    int revision = 0;
    bool isValid() const { return release > 0; }
    QString identifier() const;
    auto operator<=>(const ReleaseVersion&) const = default;
};

ReleaseVersion parseVersion(const QString& identifier);
ReleaseVersion assetVersion(const QString& releaseTag, const QString& assetName);
Result<> unpack(const QString& archivePath, const QDir& staging);
Result<> install(const QDir& staging, const QDir& target);
}
