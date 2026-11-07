// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDir>
#include <QString>

#include "Result.h"

namespace SvinPortableUpdate {
int releaseNumber(const QString& tag);
Result<> unpack(const QString& archivePath, const QDir& staging);
Result<> install(const QDir& staging, const QDir& target);
}
