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

#include "package.h"
#include "categories.h"
#include "logger.h"
#include "settings.h"
#include "utils.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>

Package::Package(QObject *parent) :
    TransferItem(parent),
    m_extractor(0),
    m_process(0),
    m_createSubfolder(false),
    m_deleteArchives(false),
    m_priority(NormalPriority),
    m_status(Null)
{
}

QVariant Package::data(int role) const {
    switch (role) {
    case CategoryRole:
        return category();
    case CreateSubfolderRole:
        return createSubfolder();
    case CustomCommandRole:
        return customCommand();
    case ErrorStringRole:
        return errorString();
    case IdRole:
        return id();
    case NameRole:
        return name();
    case PriorityRole:
        return priority();
    case PriorityStringRole:
        return priorityString();
    case ProgressRole:
        return progress();
    case ProgressStringRole:
        return progressString();
    case StatusRole:
        return status();
    case StatusStringRole:
        return statusString();
    case SuffixRole:
        return suffix();
    default:
        return TransferItem::data(role);
    }
}

bool Package::setData(int role, const QVariant &value) {
    switch (role) {
    case CategoryRole:
        setCategory(value.toString());
        return true;
    case CreateSubfolderRole:
        setCreateSubfolder(value.toBool());
        return true;
    case CustomCommandRole:
        setCustomCommand(value.toString());
        return true;
    case IdRole:
        setId(value.toString());
        return true;
    case NameRole:
        setName(value.toString());
        return true;
    case PriorityRole:
        setPriority(TransferItem::Priority(value.toInt()));
        return true;
    case StatusRole:
        switch (value.toInt()) {
        case Queued:
            queue();
            return true;
        case Downloading:
            start();
            return true;
        case Paused:
            pause();
            return true;
        case Canceled:
            cancel();
            return true;
        case CanceledAndDeleted:
            cancel(true);
            return true;
        default:
            return TransferItem::setData(role, value);
        }
    case SuffixRole:
        setSuffix(value.toString());
        return true;
    default:
        return TransferItem::setData(role, value);
    }
}

QMap<int, QVariant> Package::itemData() const {
    QMap<int, QVariant> map = TransferItem::itemData();
    map[CategoryRole] = category();
    map[CreateSubfolderRole] = createSubfolder();
    map[CustomCommandRole] = customCommand();
    map[ErrorStringRole] = errorString();
    map[IdRole] = id();
    map[NameRole] = name();
    map[PriorityRole] = priority();
    map[PriorityStringRole] = priorityString();
    map[ProgressRole] = progress();
    map[ProgressStringRole] = progressString();
    map[StatusRole] = status();
    map[StatusStringRole] = statusString();
    map[SuffixRole] = suffix();
    return map;
}

QVariantMap Package::itemDataWithRoleNames() const {
    QVariantMap map = TransferItem::itemDataWithRoleNames();
    map[roleNames().value(CategoryRole)] = category();
    map[roleNames().value(CreateSubfolderRole)] = createSubfolder();
    map[roleNames().value(CustomCommandRole)] = customCommand();
    map[roleNames().value(ErrorStringRole)] = errorString();
    map[roleNames().value(IdRole)] = id();
    map[roleNames().value(NameRole)] = name();
    map[roleNames().value(PriorityRole)] = priority();
    map[roleNames().value(PriorityStringRole)] = priorityString();
    map[roleNames().value(ProgressRole)] = progress();
    map[roleNames().value(ProgressStringRole)] = progressString();
    map[roleNames().value(StatusRole)] = status();
    map[roleNames().value(StatusStringRole)] = statusString();
    map[roleNames().value(SuffixRole)] = suffix();
    return map;
}

TransferItem::ItemType Package::itemType() const {
    return TransferItem::PackageType;
}

bool Package::canStart() const {
    return (rowCount() > 0) && (status() == Null);
}

bool Package::canPause() const {
    return (rowCount() > 0) && (status() == Null);
}

bool Package::canCancel() const {
    return (rowCount() == 0) || (status() == Null);
}

QString Package::category() const {
    return m_category;
}

void Package::setCategory(const QString &c) {
    if (c != category()) {
        m_category = c;
        emit dataChanged(this, CategoryRole);
    }
}

bool Package::createSubfolder() const {
    return m_createSubfolder;
}

void Package::setCreateSubfolder(bool enabled) {
    if (enabled != createSubfolder()) {
        m_createSubfolder = enabled;
        emit dataChanged(this, CreateSubfolderRole);
    }
}

QString Package::customCommand() const {
    return m_command;
}

void Package::setCustomCommand(const QString &c) {
    if (c != customCommand()) {
        m_command = c;
        emit dataChanged(this, CustomCommandRole);
    }
}

QString Package::id() const {
    return m_id;
}

void Package::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit dataChanged(this, IdRole);
    }
}

QString Package::name() const {
    return m_name;
}

void Package::setName(const QString &n) {
    if (n != name()) {
        m_name = n;
        emit dataChanged(this, NameRole);
    }
}

QString Package::suffix() const {
    return m_suffix;
}

void Package::setSuffix(const QString &s) {
    if (s != suffix()) {
        m_suffix = s;
        emit dataChanged(this, SuffixRole);
    }
}

TransferItem::Priority Package::priority() const {
    return m_priority;
}

void Package::setPriority(TransferItem::Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit dataChanged(this, PriorityRole);
    }

    foreach (TransferItem *child, m_childItems) {
        child->setData(PriorityRole, p);
    }
}

QString Package::priorityString() const {
    return TransferItem::priorityString(priority());
}

int Package::progress() const {
    if (rowCount() == 0) {
        return 0;
    }
    
    int completed = 0;

    foreach (TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            completed++;
        }
    }

    if (completed > 0) {
        return completed * 100 / rowCount();
    }

    return 0;
}

QString Package::progressString() const {
    if (rowCount() == 0) {
        return QString();
    }

    int completed = 0;

    foreach (TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            completed++;
        }
    }

    return tr("%1 of %2 (%3%)").arg(completed).arg(rowCount()).arg(completed > 0 ? completed * 100 / rowCount() : 0);
}

TransferItem::Status Package::status() const {
    return m_status;
}

void Package::setStatus(TransferItem::Status s) {
    if (s != status()) {
        m_status = s;
        Logger::log(QString("Package::setStatus(): ID: %1, Status: %2").arg(id()).arg(statusString()));

        switch (s) {
        case Canceled:
        case CanceledAndDeleted:
        case Failed:
        case Completed:
            emit finished(this);
            break;
        default:
            break;
        }
        
        emit dataChanged(this, StatusRole);
    }
}

QString Package::statusString() const {
    switch (status()) {
    case Failed:
        return QString("%1: %2").arg(TransferItem::statusString(Failed)).arg(errorString());
    default:
        return TransferItem::statusString(status());
    }
}

QString Package::errorString() const {
    return m_errorString;
}

void Package::setErrorString(const QString &e) {
    m_errorString = e;
}

bool Package::cancel(bool deleteFiles) {
    if (rowCount() == 0) {
        setStatus(deleteFiles ? CanceledAndDeleted : Canceled);
        return true;
    }

    return TransferItem::cancel(deleteFiles);
}

void Package::restore(const QSettings &settings) {
    setCategory(settings.value("category").toString());
    setCustomCommand(settings.value("customCommand").toString());
    setErrorString(settings.value("errorString").toString());
    setId(settings.value("id").toString());
    setName(settings.value("name").toString());
    setPriority(TransferItem::Priority(settings.value("priority").toInt()));
    setSuffix(settings.value("suffix").toString());

    const TransferItem::Status status = TransferItem::Status(settings.value("status").toInt());

    switch (status) {
    case Null:
    case Failed:
        setStatus(status);
        break;
    default:
        setStatus(Null);
        break;
    }
}

void Package::save(QSettings &settings) {
    settings.setValue("category", category());
    settings.setValue("customCommand", customCommand());
    settings.setValue("errorString", errorString());
    settings.setValue("id", id());
    settings.setValue("name", name());
    settings.setValue("priority", TransferItem::Priority(priority()));
    settings.setValue("suffix", suffix());

    switch (status()) {
    case Null:
    case Failed:
        settings.setValue("status", TransferItem::Status(status()));
        break;
    default:
        settings.setValue("status", TransferItem::Status(Null));
        break;
    }
}

void Package::childItemFinished(TransferItem *item) {
    const TransferItem::Status status = TransferItem::Status(item->data(StatusRole).toInt());
    
    if (status == Completed) {
        Logger::log("Package::childItemFinished(): Child item completed");
        emit dataChanged(this, ProgressRole);
        
        foreach (const TransferItem *child, m_childItems) {
            if (child->data(StatusRole) != Completed) {
                Logger::log("Package::childItemFinished(): Incomplete items remaining. Doing nothing");
                return;
            }
        }

        if (Settings::extractArchives()) {
            getArchives();
            
            if (!m_archives.isEmpty()) {
                extractArchive(m_archives.takeFirst());
                return;
            }
        }

        if (moveFiles()) {
            cleanup();
            setStatus(Completed);
        }
        else {
            setStatus(Failed);
        }
    }
    else if ((status == Canceled) || (status == CanceledAndDeleted)) {
        Logger::log("Package::childItemFinished(): Child item canceled.");
        
        foreach (TransferItem *child, m_childItems) {
            switch (child->data(StatusRole).toInt()) {
            case Canceled:
            case CanceledAndDeleted:
                break;
            default:
                Logger::log("Package::childItemFinished(): Non-canceled items remaining. Doing nothing");
                return;
            }
        }
        
        cleanup();
        setStatus(status);
    }
}

void Package::executeCustomCommand(const QString &fileName) {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCustomCommandFinished(int)));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onCustomCommandError()));
    }

    m_process->start(customCommand().replace("%FILENAME%", fileName));
}

void Package::extractArchive(const Archive &archive) {
    if (!m_extractor) {
        m_extractor = new ArchiveExtractor(this);
        connect(m_extractor, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onArchiveExtractionFinished(int)));
        connect(m_extractor, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onArchiveExtractionError()));
    }

    setStatus(ExtractingArchive);
    m_extractor->start(archive);
}

void Package::getArchives() {
    m_archives.clear();
    QString outputPath;
    
    if (!category().isEmpty()) {
        outputPath = Categories::get(category()).path;
    }
    
    if (outputPath.isEmpty()) {
        outputPath = Settings::downloadPath();
    }
    
    QList<TransferItem*> children = m_childItems;

    while (!children.isEmpty()) {
        const TransferItem *child = children.takeFirst();
        const QString fileName = child->data(FileNameRole).toString();

        if (Utils::isArchive(fileName)) {
            Logger::log("Package::getArchives(): Found archive " + fileName);
            const QString path = child->data(DownloadPathRole).toString();
            Archive archive;
            archive.fileNames << path + fileName;
            archive.passwords = Settings::archivePasswords();
            archive.outputDirectory = outputPath;
            archive.createSubdirectory = createSubfolder();
            archive.deleteWhenExtracted = Settings::deleteExtractedArchives();

            if (Utils::isSplitArchive(fileName)) {
                for (int i = children.size() - 1; i >= 0; i--) {
                    TransferItem *other = children.at(i);
                    const QString otherFileName = other->data(FileNameRole).toString();

                    if (Utils::isArchive(otherFileName)) {
                        const QString otherPath = other->data(DownloadPathRole).toString();
                        
                        if (Utils::belongsToArchive(otherFileName, fileName)) {
                            Logger::log("Package::getArchives(): Found archive part " + otherFileName);
                            
                            if (QFile::rename(otherPath + otherFileName, path + otherFileName)) {
                                Logger::log(QString("Package::getArchives(): Moving archive part %1 to path %2")
                                                   .arg(otherFileName).arg(path));
                                archive.fileNames << path + otherFileName;
                            }

                            children.removeAt(i);
                        }
                    }
                    else {
                        children.removeAt(i);
                    }
                }
            }

            m_archives << archive;
        }
    }
}

bool Package::moveFiles() {
    setStatus(MovingFiles);
    QDir dir;
    QString outputPath;

    if (!category().isEmpty()) {
        outputPath = Categories::get(category()).path;
    }

    if (outputPath.isEmpty()) {
        outputPath = Settings::downloadPath();
    }

    if (createSubfolder()) {
        outputPath.append(name() + "/");
    }

    if (!dir.mkpath(outputPath)) {
        Logger::log("Package::moveFiles(): Cannot create output directory");
        setErrorString(tr("Cannot create output directory for downloaded files"));
        return false;
    };

    Logger::log("Package::moveFiles(): Moving files to directory " + outputPath);
    QFile file;

    foreach (TransferItem *child, m_childItems) {
        const QString oldFilePath = child->data(FilePathRole).toString();

        if (QFile::exists(oldFilePath)) {
            const QString oldFileName = child->data(FileNameRole).toString();
            const QString newFilePath = Utils::getSaveFileName(oldFileName, outputPath);
            
            if ((newFilePath.isEmpty()) || (!file.rename(oldFilePath, newFilePath))) {
                Logger::log(QString("Package::moveFiles(): Cannot rename %1 to %2").arg(oldFilePath)
                                                                                   .arg(newFilePath));
                setErrorString(file.errorString());
                return false;
            }
            
            Logger::log(QString("Package::moveFiles(): Renamed %1 to %2").arg(oldFilePath).arg(newFilePath));
        }
    }

    return true;
}

void Package::cleanup() {
    QDir dir;
    
    foreach (TransferItem *child, m_childItems) {
        dir.setPath(child->data(DownloadPathRole).toString());

        if (dir.rmdir(dir.path())) {
            Logger::log("Package::cleanup(): Removed directory " + dir.path());
        }
        else {
            Logger::log("Package::cleanup(): Cannot remove directory " + dir.path());
        }
    }
}

void Package::onCustomCommandFinished(int exitCode) {
    if (exitCode == 0) {
        Logger::log("Package::onCustomCommandFinished(): OK");
        cleanup();
        setStatus(Completed);
    }
    else {
        Logger::log("Package::onCustomCommandFinished(): Error: " + m_process->errorString());
        setErrorString(m_process->errorString());
        setStatus(Failed);
    }
}

void Package::onCustomCommandError() {
    Logger::log("Package::onCustomCommandError(): " + m_process->errorString());
    setErrorString(m_process->errorString());
    setStatus(Failed);
}

void Package::onArchiveExtractionFinished(int exitCode) {
    if (!m_archives.isEmpty()) {
        extractArchive(m_archives.takeFirst());
        return;
    }
    
    if ((exitCode == 0) && (moveFiles())) {
        cleanup();
        setStatus(Completed);
    }
    else {
        Logger::log("Package::onArchiveExtractionFinished(): Error: " + m_extractor->errorString());
        setErrorString(m_extractor->errorString());
        setStatus(Failed);
    }
}

void Package::onArchiveExtractionError() {
    if (!m_archives.isEmpty()) {
        extractArchive(m_archives.takeFirst());
        return;
    }
    
    Logger::log("Package::onArchiveExtractionError(): " + m_extractor->errorString());
    setErrorString(m_extractor->errorString());
    setStatus(Failed);
}
