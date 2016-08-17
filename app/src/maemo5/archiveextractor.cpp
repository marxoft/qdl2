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

#include "archiveextractor.h"
#include "logger.h"
#include <QFile>

ArchiveExtractor::ArchiveExtractor(QObject *parent) :
    QObject(parent),
    m_process(0)
{
}

QString ArchiveExtractor::errorString() const {
    return m_errorString;
}

void ArchiveExtractor::setErrorString(const QString &errorString) {
    m_errorString = errorString;
}

void ArchiveExtractor::start(const Archive &archive) {
    if ((archive.fileNames.isEmpty()) || (archive.outputDirectory.isEmpty())) {
        setErrorString(tr("No input filenames and/or output directory specified"));
        emit error(QProcess::FailedToStart);
        return;
    }

    m_archive = archive;
    extract();
}

void ArchiveExtractor::start(const QStringList &fileNames, const QString &outputDirectory, bool createSubdirectory,
                             bool deleteWhenExtracted, const QStringList &passwords) {
    if ((fileNames.isEmpty()) || (outputDirectory.isEmpty())) {
        setErrorString(tr("No input filenames and/or output directory specified"));
        emit error(QProcess::FailedToStart);
        return;
    }

    m_archive.fileNames = fileNames;
    m_archive.outputDirectory = outputDirectory;
    m_archive.createSubdirectory = createSubdirectory;
    m_archive.deleteWhenExtracted = deleteWhenExtracted;
    m_archive.passwords = passwords;
    extract();
}

void ArchiveExtractor::extract(const QString &password) {
    QString command;
    const QString &fileName = m_archive.fileNames.first();
    const QString fileSuffix = fileName.mid(fileName.lastIndexOf('.') + 1);
    QString subFolder = fileName.mid(fileName.lastIndexOf('/') + 1);
    subFolder = subFolder.left(subFolder.lastIndexOf(QRegExp("part\\d+\\.", Qt::CaseInsensitive)));
    subFolder = subFolder.left(subFolder.lastIndexOf('.'));

    if (fileSuffix == "rar") {
        command = QString("unrar x -or -p-");

        if (m_archive.createSubdirectory) {
            command.append(" -ad");
        }

        if (!password.isEmpty()) {
            command.append(" -p" + password);
        }

        command.append(QString(" \"%1\" \"%2\"").arg(fileName).arg(m_archive.outputDirectory));
    }
    else if (fileSuffix == "zip") {
        command = QString("unzip -n \"%1\"").arg(fileName);

        if (!password.isEmpty()) {
            command.append(" -P " + password);
        }

        command.append(QString(" -d \"%1%2\"").arg(m_archive.outputDirectory)
                                              .arg(m_archive.createSubdirectory ? subFolder : ""));
    }
    else if (fileSuffix == "gz") {
        command = QString("untar xvf \"%1\" -C \"%2%3\"")
                  .arg(fileName)
                  .arg(m_archive.outputDirectory)
                  .arg(m_archive.createSubdirectory ? subFolder : "");
    }
    else if (fileSuffix == "bz") {
        command = QString("untar xjf \"%1\" -C \"%2%3\"")
                  .arg(fileName)
                  .arg(m_archive.outputDirectory)
                  .arg(m_archive.createSubdirectory ? subFolder : "");
    }
    else if (fileSuffix == "7z") {
        command = QString("7za x -p");
        
        if (!password.isEmpty()) {
            command.append(password);
        }
        else {
            // Avoid password prompt
            command.append(" ");
        }

        command.append(QString(" -o\"%1%2\" \"%3\"").arg(m_archive.outputDirectory)
                                                    .arg(m_archive.createSubdirectory ? subFolder : "")
                                                    .arg(fileName));
    }

    if (command.isEmpty()) {
        setErrorString(tr("No handler found for archive"));
        emit error(QProcess::FailedToStart);
    }
    else {
        if (!m_process) {
            m_process = new QProcess(this);
            connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                    this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
            connect(m_process, SIGNAL(error(QProcess::ProcessError)),
                    this, SLOT(onProcessError(QProcess::ProcessError)));
        }

        if (m_process->state() != QProcess::Running) {
            Logger::log("ArchiveExtractor::extract(). Extracting archive using command: " + command,
                        Logger::MediumVerbosity);
            m_process->start(command);
        }
    }
}

void ArchiveExtractor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode == 0) {
        if (m_archive.deleteWhenExtracted) {
            foreach (const QString &fileName, m_archive.fileNames) {
                QFile::remove(fileName);
            }
        }
        
        emit finished(exitCode, exitStatus);
    }
    else if (!m_archive.passwords.isEmpty()) {
        extract(m_archive.passwords.takeFirst());
    }
    else {
        setErrorString(m_process->readAllStandardError());
        emit error(m_process->error());
    }
}

void ArchiveExtractor::onProcessError(QProcess::ProcessError e) {
    setErrorString(m_process->errorString());
    emit error(e);
}
