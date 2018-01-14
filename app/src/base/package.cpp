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
#include <QSettings>

Package::Package(QObject *parent) :
    TransferItem(parent),
    m_extractor(0),
    m_process(0),
    m_createSubfolder(false),
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
    return (rowCount() == 0) || (status() == Null) || (status() == Failed);
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

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            ++completed;
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

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            ++completed;
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
        
        emit statusChanged(this, s);
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

bool Package::queue() {
    if (!canStart()) {
        return false;
    }

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) != Completed) {
            return TransferItem::queue();
        }
    }

    processCompletedItems();
    return true;
}

bool Package::start() {
    if (!canStart()) {
        return false;
    }

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) != Completed) {
            return TransferItem::start();
        }
    }

    processCompletedItems();
    return true;
}

bool Package::cancel(bool deleteFiles) {
    if (!canCancel()) {
        return false;
    }

    if (rowCount() == 0) {
        setStatus(deleteFiles ? CanceledAndDeleted : Canceled);
        return true;
    }

    setStatus(Canceling);
    return TransferItem::cancel(deleteFiles);
}

void Package::restore(const QSettings &settings) {
    setCategory(settings.value("category").toString());
    setCreateSubfolder(settings.value("createSubfolder", false).toBool());
    setErrorString(settings.value("errorString").toString());
    setId(settings.value("id").toString());
    setName(settings.value("name").toString());
    setPriority(TransferItem::Priority(settings.value("priority", NormalPriority).toInt()));
    setSuffix(settings.value("suffix").toString());

    const TransferItem::Status status = TransferItem::Status(settings.value("status", Null).toInt());

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
    settings.setValue("createSubfolder", createSubfolder());
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
        Logger::log("Package::childItemFinished(): Child item completed", Logger::LowVerbosity);
        emit dataChanged(this, ProgressRole);
        
        foreach (const TransferItem *child, m_childItems) {
            if (child->data(StatusRole) != Completed) {
                Logger::log("Package::childItemFinished(): Incomplete items remaining. Doing nothing",
                            Logger::MediumVerbosity);
                return;
            }
        }

        processCompletedItems();        
    }
    else if ((status == Canceled) || (status == CanceledAndDeleted)) {
        Logger::log("Package::childItemFinished(): Child item canceled.", Logger::LowVerbosity);
        
        foreach (const TransferItem *child, m_childItems) {
            switch (child->data(StatusRole).toInt()) {
            case Canceled:
            case CanceledAndDeleted:
                break;
            default:
                Logger::log("Package::childItemFinished(): Non-canceled items remaining. Doing nothing",
                            Logger::MediumVerbosity);
                return;
            }
        }
        
        cleanup();
        setStatus(status);
    }
}

void Package::processCompletedItems() {
    if (Settings::extractArchives()) {
        getArchives();
        
        if (!m_archives.isEmpty()) {
            extractArchive(m_archives.takeFirst());
            return;
        }
    }
    
    if (moveFiles()) {
        getCustomCommands();

        if (!m_commands.isEmpty()) {
            executeCustomCommand(m_commands.takeFirst());
        }
        else {
            setStatus(Completed);
        }
    }
    else {
        setStatus(Failed);
    }
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
            Logger::log("Package::getArchives(): Found archive " + fileName, Logger::MediumVerbosity);
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
                            Logger::log("Package::getArchives(): Found archive part " + otherFileName,
                                        Logger::MediumVerbosity);
                            
                            if (QFile::rename(otherPath + otherFileName, path + otherFileName)) {
                                Logger::log(QString("Package::getArchives(): Moving archive part %1 to path %2")
                                                   .arg(otherFileName).arg(path), Logger::MediumVerbosity);
                                archive.fileNames << path + otherFileName;
                                other->setData(FilePathRole, path + otherFileName);

                                if (QDir().rmdir(otherPath)) {
                                    Logger::log("Package::getArchives(): Removed directory " + otherPath,
                                                Logger::MediumVerbosity);
                                }
                                else {
                                    Logger::log("Package::getArchives(): Cannot remove directory " + otherPath,
                                                Logger::MediumVerbosity);
                                }
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
    
    Logger::log(QString("Package::getArchives(): %1 archives found").arg(m_archives.size()), Logger::LowVerbosity);
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
        outputPath = Utils::getSaveDirectory(outputPath + name());
    }

    if (!dir.mkpath(outputPath)) {
        setErrorString(tr("Cannot create output directory for downloaded files"));
        return false;
    };

    Logger::log("Package::moveFiles(): Moving files to directory " + outputPath, Logger::LowVerbosity);
    QFile file;

    foreach (TransferItem *child, m_childItems) {
        const QString oldFilePath = child->data(FilePathRole).toString();

        if (QFile::exists(oldFilePath)) {
            const QString oldFileName = child->data(FileNameRole).toString();
            const QString newFilePath = Utils::getSaveFileName(oldFileName, outputPath);
            
            if ((newFilePath.isEmpty()) || (!file.rename(oldFilePath, newFilePath))) {
                Logger::log(QString("Package::moveFiles(): Cannot rename %1 to %2").arg(oldFilePath).arg(newFilePath),
                            Logger::LowVerbosity);
                setErrorString(tr("Cannot move files: %1").arg(file.errorString()));
                return false;
            }
            
            Logger::log(QString("Package::moveFiles(): Renamed %1 to %2").arg(oldFilePath).arg(newFilePath),
                        Logger::MediumVerbosity);
            dir.setPath(child->data(DownloadPathRole).toString());
            
            if (dir.rmdir(dir.path())) {
                Logger::log("Package::moveFiles(): Removed directory " + dir.path(), Logger::MediumVerbosity);
            }
            else {
                Logger::log("Package::moveFiles(): Cannot remove directory " + dir.path(), Logger::MediumVerbosity);
            }
            
            child->setData(FilePathRole, newFilePath);
        }
    }

    return true;
}

void Package::cleanup() {
    QDir dir;
    
    foreach (const TransferItem *child, m_childItems) {
        dir.setPath(child->data(DownloadPathRole).toString());
        
        if (dir.rmdir(dir.path())) {
            Logger::log("Package::cleanup(): Removed directory " + dir.path(), Logger::MediumVerbosity);
        }
        else {
            Logger::log("Package::cleanup(): Cannot remove directory " + dir.path(), Logger::MediumVerbosity);
        }
    }
}

void Package::getCustomCommands() {
    m_commands.clear();
    const QString defaultCommand = Settings::customCommand();
    const bool defaultEnabled = (!defaultCommand.isEmpty()) && (Settings::customCommandEnabled());

    foreach (const TransferItem *child, m_childItems) {
        QString command = child->data(CustomCommandRole).toString();        

        if (!command.isEmpty()) {
            const QString workingDirectory = child->data(DownloadPathRole).toString();
            command.replace("%f", child->data(FileNameRole).toString());
            Logger::log(QString("Package::getCustomCommands(): Adding custom command: Working directory: %1, Command: %2")
                               .arg(workingDirectory).arg(command), Logger::MediumVerbosity);
            m_commands << Command(workingDirectory, command);
        }
        
        if ((defaultEnabled) && ((command.isEmpty()) || (!child->data(CustomCommandOverrideEnabledRole).toBool()))) {
            const QString workingDirectory = child->data(DownloadPathRole).toString();
            command = defaultCommand;
            command.replace("%f", child->data(FileNameRole).toString());
            Logger::log(QString("Package::getCustomCommands(): Adding custom command: Working directory: %1, Command: %2")
                               .arg(workingDirectory).arg(command), Logger::MediumVerbosity);
            m_commands << Command(workingDirectory, command);
        }
    }
}

void Package::executeCustomCommand(const Command &command) {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCustomCommandFinished(int)));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onCustomCommandError()));
    }

    Logger::log(QString("Package::executeCustomCommand(): Working directory: %1, Command: %2")
                       .arg(command.workingDirectory).arg(command.command), Logger::LowVerbosity);
    
    if (QDir(command.workingDirectory).exists()) {
        m_process->setWorkingDirectory(command.workingDirectory);
    }
    
    m_process->start(command.command);
}

void Package::onCustomCommandFinished(int exitCode) {
    if (exitCode != 0) {
        Logger::log("Package::onCustomCommandFinished(): Error: " + m_process->readAllStandardError());
    }
    
    if (!m_commands.isEmpty()) {
        executeCustomCommand(m_commands.takeFirst());
    }
    else {
        setStatus(Completed);
    }
}

void Package::onCustomCommandError() {
    Logger::log("Package::onCustomCommandError(): " + m_process->errorString());

    if (!m_commands.isEmpty()) {
        executeCustomCommand(m_commands.takeFirst());
    }
    else {
        setStatus(Completed);
    }
}

void Package::onArchiveExtractionFinished(int exitCode) {
    if (exitCode != 0) {
        Logger::log("Package::onArchiveExtractionFinished(): Error: " + m_extractor->errorString());
    }
    
    if (!m_archives.isEmpty()) {
        extractArchive(m_archives.takeFirst());
        return;
    }

    if (moveFiles()) {
        cleanup();
        getCustomCommands();

        if (!m_commands.isEmpty()) {
            executeCustomCommand(m_commands.takeFirst());
        }
        else {
            setStatus(Completed);
        }
    }
    else {
        setStatus(Failed);
    }
}

void Package::onArchiveExtractionError() {
    Logger::log("Package::onArchiveExtractionError(): Error: " + m_extractor->errorString());
    
    if (!m_archives.isEmpty()) {
        extractArchive(m_archives.takeFirst());
        return;
    }
    
    if (moveFiles()) {
        cleanup();
        getCustomCommands();

        if (!m_commands.isEmpty()) {
            executeCustomCommand(m_commands.takeFirst());
        }
        else {
            setStatus(Completed);
        }
    }
    else {
        setStatus(Failed);
    }
}
