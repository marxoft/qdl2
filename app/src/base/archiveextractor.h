/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <QObject>
#include <QList>
#include <QProcess>

struct Archive {
    QString outputDirectory;
    QStringList fileNames;
    QStringList passwords;

    bool createSubdirectory;
    bool deleteWhenExtracted;
};

typedef QList<Archive> ArchiveList;

class ArchiveExtractor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString)

public:
    explicit ArchiveExtractor(QObject *parent = 0);

    QString errorString() const;

public Q_SLOTS:
    void start(const Archive &archive);
    void start(const QStringList &parts, const QString &outputDirectory, bool createSubdirectory = false,
               bool deleteWhenExtracted = false, const QStringList &passwords = QStringList());

private:
    void setErrorString(const QString &errorString);

    void extract(const QString &password = QString());

private Q_SLOTS:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError e);

Q_SIGNALS:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void error(QProcess::ProcessError error);
    
private:
    QProcess *m_process;

    Archive m_archive;
    
    QString m_errorString;
};

#endif // ARCHIVEEXTRACTOR_H
