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

#include "urlcheckmodel.h"
#include "captchatype.h"
#include "definitions.h"
#include "logger.h"
#include "transfermodel.h"
#include <QIcon>

UrlCheckModel* UrlCheckModel::self = 0;

UrlCheckModel::UrlCheckModel() :
    QAbstractListModel(),
    m_checker(0),
    m_status(Idle),
    m_index(-1)
{
    m_roles[UrlRole] = "url";
    m_roles[CategoryRole] = "category";
    m_roles[CreateSubfolderRole] = "createSubfolder";
    m_roles[PriorityRole] = "priority";
    m_roles[CustomCommandRole] = "customCommand";
    m_roles[CustomCommandOverrideEnabledRole] = "customCommandOverrideEnabled";
    m_roles[StartAutomaticallyRole] = "startAutomatically";
    m_roles[IsCheckedRole] = "checked";
    m_roles[IsOkRole] = "ok";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

UrlCheckModel::~UrlCheckModel() {
    self = 0;
}

UrlCheckModel* UrlCheckModel::instance() {
    return self ? self : self = new UrlCheckModel;
}

int UrlCheckModel::captchaType() const {
    return m_checker ? m_checker->captchaType() : CaptchaType::Unknown;
}

QString UrlCheckModel::captchaTypeString() const {
    return m_checker ? m_checker->captchaTypeString() : tr("Unknown");
}

QByteArray UrlCheckModel::captchaData() const {
    return m_checker ? m_checker->captchaData() : QByteArray();
}

int UrlCheckModel::captchaTimeout() const {
    return m_checker ? m_checker->captchaTimeout() : 0;
}

QString UrlCheckModel::captchaTimeoutString() const {
    return m_checker ? m_checker->captchaTimeoutString() : QString();
}

int UrlCheckModel::progress() const {
    return (!m_items.isEmpty()) && (m_index > 0) ? m_index * 100 / m_items.size() : 0;
}

QVariantList UrlCheckModel::requestedSettings() const {
    return m_checker ? m_checker->requestedSettings() : QVariantList();
}

int UrlCheckModel::requestedSettingsTimeout() const {
    return m_checker ? m_checker->requestedSettingsTimeout() : 0;
}

QString UrlCheckModel::requestedSettingsTimeoutString() const {
    return m_checker ? m_checker->requestedSettingsTimeoutString() : QString();
}

QString UrlCheckModel::requestedSettingsTitle() const {
    return m_checker ? m_checker->requestedSettingsTitle() : QString();
}

UrlCheckModel::Status UrlCheckModel::status() const {
    return m_status;
}

void UrlCheckModel::setStatus(UrlCheckModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString UrlCheckModel::statusString() const {
    switch (status()) {
    case Active:
        return tr("Checking URLs");
    case RetrievingCaptchaChallenge:
        return tr("Retrieving captcha challenge");
    case AwaitingCaptchaResponse:
        return tr("Awaiting captcha response: %1").arg(captchaTimeoutString());
    case RetrievingCaptchaResponse:
        return tr("Retrieving captcha response");
    case SubmittingCaptchaResponse:
        return tr("Submitting captcha response");
    case AwaitingSettingsResponse:
        return tr("Awaiting settings response: %1").arg(requestedSettingsTimeoutString());
    case SubmittingSettingsResponse:
        return tr("Submitting settings response");
    case WaitingActive:
        return tr("Waiting (active): %1").arg(waitTimeString());
    case WaitingInactive:
        return tr("Waiting (inactive): %2").arg(waitTimeString());
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    default:
        return QString();
    }
}

int UrlCheckModel::waitTime() const {
    return m_checker ? m_checker->waitTime() : 0;
}

QString UrlCheckModel::waitTimeString() const {
    return m_checker ? m_checker->waitTimeString() : QString();
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> UrlCheckModel::roleNames() const {
    return m_roles;
}
#endif

int UrlCheckModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int UrlCheckModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant UrlCheckModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("URL");
    case 1:
        return tr("Ok?");
    default:
        return QVariant();
    }
}

QVariant UrlCheckModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_items.at(index.row()).url;
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case 1:
            if (m_items.at(index.row()).checked) {
                if (m_items.at(index.row()).ok) {
                    return QIcon::fromTheme(OK_ICON);
                }
                
                return QIcon::fromTheme(ERROR_ICON);
            }

            return QVariant();
        default:
            return QVariant();
        }
    case UrlRole:
        return m_items.at(index.row()).url;
    case CategoryRole:
        return m_items.at(index.row()).category;
    case CreateSubfolderRole:
        return m_items.at(index.row()).createSubfolder;
    case PriorityRole:
        return m_items.at(index.row()).priority;
    case CustomCommandRole:
        return m_items.at(index.row()).customCommand;
    case CustomCommandOverrideEnabledRole:
        return m_items.at(index.row()).customCommandOverrideEnabled;
    case StartAutomaticallyRole:
        return m_items.at(index.row()).startAutomatically;
    case IsCheckedRole:
        return m_items.at(index.row()).checked;
    case IsOkRole:
        return m_items.at(index.row()).ok;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> UrlCheckModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant UrlCheckModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap UrlCheckModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList UrlCheckModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
        Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int UrlCheckModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void UrlCheckModel::append(const QString &url, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    Logger::log("UrlCheckModel::append(): " + url, Logger::LowVerbosity);
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << UrlCheck(url, category, createSubfolder, priority, customCommand, overrideGlobalCommand,
            startAutomatically);
    endInsertRows();
    emit countChanged(rowCount());
    emit progressChanged(progress());

    if (status() != Active) {
        next();
    }
}

void UrlCheckModel::append(const QStringList &urls, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    foreach (const QString &url, urls) {
        append(url, category, createSubfolder, priority, customCommand, overrideGlobalCommand, startAutomatically);
    }
}

bool UrlCheckModel::remove(int row) {
    if ((row > m_index) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
        emit countChanged(rowCount());
        emit progressChanged(progress());
        return true;
    }
    
    return false;
}

void UrlCheckModel::cancel() {
    if (m_checker) {
        m_checker->cancel();
    }

    setStatus(Canceled);
}

void UrlCheckModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        beginResetModel();
        m_items.clear();
        m_index = -1;
        endResetModel();
        emit countChanged(0);
    }
}

bool UrlCheckModel::submitCaptchaResponse(const QString &response) {
    return m_checker ? m_checker->submitCaptchaResponse(response) : false;
}

bool UrlCheckModel::submitSettingsResponse(const QVariantMap &settings) {
    return m_checker ? m_checker->submitSettingsResponse(settings) : false;
}

void UrlCheckModel::next() {
    ++m_index;
    emit progressChanged(progress());

    if (m_index >= m_items.size()) {
        setStatus(Completed);
        return;
    }

    setStatus(Active);
    const QModelIndex idx = index(m_index, 0);
    const QString url = data(idx, UrlRole).toString();
    checker()->checkUrl(url);
}

UrlChecker* UrlCheckModel::checker() {
    if (!m_checker) {
        m_checker = new UrlChecker(this);
        connect(m_checker, SIGNAL(captchaRequest(int, QByteArray)), this, SIGNAL(captchaRequest(int, QByteArray)));
        connect(m_checker, SIGNAL(captchaTimeoutChanged(int)), this, SIGNAL(captchaTimeoutChanged(int)));
        connect(m_checker, SIGNAL(error(QString)), this, SLOT(onError(QString)));
        connect(m_checker, SIGNAL(requestedSettingsTimeoutChanged(int)),
                this, SIGNAL(requestedSettingsTimeoutChanged(int)));
        connect(m_checker, SIGNAL(settingsRequest(QString, QVariantList)),
                this, SIGNAL(settingsRequest(QString, QVariantList)));
        connect(m_checker, SIGNAL(statusChanged(UrlChecker::Status)), this, SLOT(onStatusChanged(UrlChecker::Status)));
        connect(m_checker, SIGNAL(urlChecked(UrlResult)), this, SLOT(onUrlChecked(UrlResult)));
        connect(m_checker, SIGNAL(urlChecked(UrlResultList, QString)),
                this, SLOT(onUrlChecked(UrlResultList, QString)));
        connect(m_checker, SIGNAL(waitTimeChanged(int)), this, SIGNAL(waitTimeChanged(int)));
    }

    return m_checker;
}

void UrlCheckModel::onStatusChanged(UrlChecker::Status s) {
    switch (s) {
    case UrlChecker::RetrievingCaptchaChallenge:
        setStatus(RetrievingCaptchaChallenge);
        break;
    case UrlChecker::AwaitingCaptchaResponse:
        setStatus(AwaitingCaptchaResponse);
        break;
    case UrlChecker::RetrievingCaptchaResponse:
        setStatus(RetrievingCaptchaResponse);
        break;
    case UrlChecker::SubmittingCaptchaResponse:
        setStatus(SubmittingCaptchaResponse);
        break;
    case UrlChecker::ReportingCaptchaResponse:
        setStatus(ReportingCaptchaResponse);
        break;
    case UrlChecker::AwaitingDecaptchaSettingsResponse:
    case UrlChecker::AwaitingRecaptchaSettingsResponse:
    case UrlChecker::AwaitingServiceSettingsResponse:
        setStatus(AwaitingSettingsResponse);
        break;
    case UrlChecker::SubmittingDecaptchaSettingsResponse:
    case UrlChecker::SubmittingRecaptchaSettingsResponse:
    case UrlChecker::SubmittingServiceSettingsResponse:
        setStatus(SubmittingSettingsResponse);
        break;
    case UrlChecker::WaitingActive:
        setStatus(WaitingActive);
        break;
    case UrlChecker::WaitingInactive:
        checker()->cancel();
        onError(tr("Must wait %1 for this URL check").arg(waitTimeString()));
        break;
    default:
        break;
    }
}

void UrlCheckModel::onUrlChecked(const UrlResult &result) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1 1 URL found").arg(m_items.at(m_index).url),
            Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = true;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    const UrlCheck &check = m_items.at(m_index);
    TransferModel::instance()->append(result, check.category, check.createSubfolder, check.priority,
            check.customCommand, check.customCommandOverrideEnabled, check.startAutomatically);
    next();
}

void UrlCheckModel::onUrlChecked(const UrlResultList &results, const QString &packageName) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1. %2 URLs found")
            .arg(m_items.at(m_index).url).arg(results.size()), Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = !results.isEmpty();
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    const UrlCheck &check = m_items.at(m_index);

    if (!results.isEmpty()) {
        TransferModel::instance()->append(results, packageName, check.category, check.createSubfolder,
                check.priority, check.customCommand, check.customCommandOverrideEnabled, check.startAutomatically);
    }

    next();
}

void UrlCheckModel::onError(const QString &errorString) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onError(): %1. Error: %2").arg(m_items.at(m_index).url).arg(errorString));
    m_items[m_index].checked = true;
    m_items[m_index].ok = false;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    next();
}
