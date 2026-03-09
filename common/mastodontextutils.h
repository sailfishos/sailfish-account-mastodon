/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MASTODONTEXTUTILS_H
#define MASTODONTEXTUTILS_H

#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>

namespace MastodonTextUtils {

inline QString decodeHtmlEntities(QString text)
{
    text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));

    static const QRegularExpression decimalEntity(QStringLiteral("&#(\\d+);"));
    QRegularExpressionMatch match;
    int index = 0;
    while ((index = text.indexOf(decimalEntity, index, &match)) != -1) {
        bool ok = false;
        const uint value = match.captured(1).toUInt(&ok, 10);
        QString replacement;
        if (ok && value > 0 && value <= 0x10FFFF) {
            replacement = QString::fromUcs4(&value, 1);
        }
        text.replace(index, match.capturedLength(0), replacement);
        index += replacement.size();
    }

    static const QRegularExpression hexEntity(QStringLiteral("&#x([0-9a-fA-F]+);"));
    index = 0;
    while ((index = text.indexOf(hexEntity, index, &match)) != -1) {
        bool ok = false;
        const uint value = match.captured(1).toUInt(&ok, 16);
        QString replacement;
        if (ok && value > 0 && value <= 0x10FFFF) {
            replacement = QString::fromUcs4(&value, 1);
        }
        text.replace(index, match.capturedLength(0), replacement);
        index += replacement.size();
    }

    return text;
}

inline QString sanitizeContent(const QString &content)
{
    QString plain = content;
    plain.replace(QRegularExpression(QStringLiteral("<\\s*br\\s*/?\\s*>"), QRegularExpression::CaseInsensitiveOption),
                  QStringLiteral("\n"));
    plain.replace(QRegularExpression(QStringLiteral("<\\s*/\\s*p\\s*>"), QRegularExpression::CaseInsensitiveOption),
                  QStringLiteral("\n"));
    plain.remove(QRegularExpression(QStringLiteral("<[^>]+>"), QRegularExpression::CaseInsensitiveOption));

    return decodeHtmlEntities(plain).trimmed();
}

inline QDateTime parseTimestamp(const QString &timestampString)
{
    QDateTime timestamp;

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    timestamp = QDateTime::fromString(timestampString, Qt::ISODateWithMs);
    if (timestamp.isValid()) {
        return timestamp;
    }
#endif

    timestamp = QDateTime::fromString(timestampString, Qt::ISODate);
    if (timestamp.isValid()) {
        return timestamp;
    }

    // Qt 5.6 cannot parse ISO-8601 timestamps with fractional seconds.
    const int timeSeparator = timestampString.indexOf(QLatin1Char('T'));
    const int fractionSeparator = timestampString.indexOf(QLatin1Char('.'), timeSeparator + 1);
    if (timeSeparator > -1 && fractionSeparator > -1) {
        int timezoneSeparator = timestampString.indexOf(QLatin1Char('Z'), fractionSeparator + 1);
        if (timezoneSeparator == -1) {
            timezoneSeparator = timestampString.indexOf(QLatin1Char('+'), fractionSeparator + 1);
        }
        if (timezoneSeparator == -1) {
            timezoneSeparator = timestampString.indexOf(QLatin1Char('-'), fractionSeparator + 1);
        }

        QString stripped = timestampString;
        if (timezoneSeparator > -1) {
            stripped.remove(fractionSeparator, timezoneSeparator - fractionSeparator);
        } else {
            stripped.truncate(fractionSeparator);
        }

        timestamp = QDateTime::fromString(stripped, Qt::ISODate);
    }

    return timestamp;
}

} // namespace MastodonTextUtils

#endif // MASTODONTEXTUTILS_H
