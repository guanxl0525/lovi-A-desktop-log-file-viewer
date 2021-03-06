/*
 * Copyright 2019 Aurélien Gâteau <mail@agateau.com>
 *
 * This file is part of Lovi.
 *
 * Lovi is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LOGFORMATSTORE_H
#define LOGFORMATSTORE_H

#include "Vector.h"

#include <QObject>

#include <memory>

class LogFormat;

class LogFormatStore : public QObject {
    Q_OBJECT
public:
    explicit LogFormatStore(const QString& dirPath, QObject* parent = nullptr);
    ~LogFormatStore();

    LogFormat* byName(const QString& name) const;

    QString nameAt(int idx) const;
    LogFormat* at(int idx) const;

    int count() const;

    /**
     * Try to add a new log format
     * Returns an error message in case of failure
     */
    std::optional<QString> addLogFormat(const QString& name);

signals:
    void logFormatAdded();

private:
    void load();
    QString pathForName(const QString& name) const;

    QString mDirPath;
    stdq::Vector<QString> mLogFormatNames;
    mutable stdq::Vector<std::unique_ptr<LogFormat>> mLogFormats;
};

#endif // LOGFORMATSTORE_H
