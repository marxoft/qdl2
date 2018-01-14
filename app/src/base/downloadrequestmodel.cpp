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

#include "downloadrequestmodel.h"
#include "captchatype.h"
#include "definitions.h"
#include "logger.h"
#include "utils.h"
#include <QIcon>

static QString downloadResultToString(const DownloadResult &result) {
    QString s("Filename: ");
    s.append(result.fileName);
    s.append("\nURL: ");
    s.append(result.request.url().toString());
    s.append("\nMethod: ");
    s.append(QString::fromUtf8(result.method));
    s.append("\n");

    if (!result.data.isEmpty()) {
        s.append("Post data: ");
        s.append(QString::fromUtf8(result.data));
        s.append("\n");
    }

    const QList<QByteArray> headers = result.request.rawHeaderList();

    if (!headers.isEmpty()) {
        s.append("Headers:\n");

        foreach (const QByteArray &header, headers) {
            s.append(QString::fromUtf8(header));
            s.append(": ");
            s.append(QString::fromUtf8(result.request.rawHeader(header)));
            s.append("\n");
        }
    }

    s.append("\n");
    return s;
}

static QString downloadResultListToString(const DownloadResultList &results) {
    QString s;

    foreach (const DownloadResult &result, results) {
        s.append(downloadResultToString(result));
    }

    return s;
}

DownloadRequestModel* DownloadRequestModel::self = 0;

DownloadRequestModel::DownloadRequestModel() :
    QAbstractListModel(),
    m_checker(0),
    m_requester(0),
    m_captchaType(CaptchaType::Unknown),
    m_timeRemaining(CAPTCHA_TIMEOUT),
    m_status(Idle),
    m_index(-1)
{
    m_roles[UrlRole] = "url";
    m_roles[IsCheckedRole] = "checked";
    m_roles[IsOkRole] = "ok";
    m_roles[ResultsRole] = "results";
    m_roles[ResultsStringRole] = "resultsString";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

DownloadRequestModel::~DownloadRequestModel() {
    self = 0;
}

DownloadRequestModel* DownloadRequestModel::instance() {
    return self ? self : self = new DownloadRequestModel;
}

int DownloadRequestModel::captchaType() const {
    return m_captchaType;
}

QString DownloadRequestModel::captchaTypeString() const {
    switch (captchaType()) {
    case CaptchaType::Image:
        return tr("Image");
    case CaptchaType::NoCaptcha:
        return tr("NoCaptcha");
    default:
        return tr("Unknown");
    }
}

QByteArray DownloadRequestModel::captchaData() const {
    return m_captchaData;
}

void DownloadRequestModel::setCaptchaData(int captchaType, const QByteArray &captchaData) {
    m_captchaType = captchaType;
    m_captchaData = captchaData;
}

void DownloadRequestModel::clearCaptchaData() {
    m_captchaType = CaptchaType::Unknown;
    m_captchaData.clear();
}

int DownloadRequestModel::captchaTimeout() const {
    return status() == AwaitingCaptchaResponse ? m_timeRemaining : 0;
}

QString DownloadRequestModel::captchaTimeoutString() const {
    return Utils::formatMSecs(captchaTimeout());
}

int DownloadRequestModel::progress() const {
    return (!m_items.isEmpty()) && (m_index > 0) ? m_index * 100 / m_items.size() : 0;
}

QVariantList DownloadRequestModel::requestedSettings() const {
    return m_requestedSettings;
}

int DownloadRequestModel::requestedSettingsTimeout() const {
    return status() == AwaitingSettingsResponse ? m_timeRemaining : 0;
}

QString DownloadRequestModel::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString DownloadRequestModel::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void DownloadRequestModel::setRequestedSettings(const QString &title, const QVariantList &settings) {
    m_requestedSettingsTitle = title;
    m_requestedSettings = settings;
}

void DownloadRequestModel::clearRequestedSettings() {
    m_requestedSettingsTitle.clear();
    m_requestedSettings.clear();
}

DownloadResultList DownloadRequestModel::results() const {
    DownloadResultList list;

    for (int i = 0; i < m_items.size(); i++) {
        list << m_items.at(i).results;
    }

    return list;
}

QString DownloadRequestModel::resultsString() const {
    return downloadResultListToString(results());
}

DownloadRequestModel::Status DownloadRequestModel::status() const {
    return m_status;
}

void DownloadRequestModel::setStatus(DownloadRequestModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString DownloadRequestModel::statusString() const {
    switch (status()) {
    case Active:
        return tr("Fetching download requests");
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

int DownloadRequestModel::waitTime() const {
    switch (status()) {
    case WaitingActive:
    case WaitingInactive:
        return m_timeRemaining;
    default:
        return 0;
    }
}

QString DownloadRequestModel::waitTimeString() const {
    return Utils::formatMSecs(waitTime());
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> DownloadRequestModel::roleNames() const {
    return m_roles;
}
#endif

int DownloadRequestModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int DownloadRequestModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant DownloadRequestModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QVariant DownloadRequestModel::data(const QModelIndex &index, int role) const {
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
    case IsCheckedRole:
        return m_items.at(index.row()).checked;
    case IsOkRole:
        return m_items.at(index.row()).ok;
    case ResultsRole:
        return QVariant::fromValue(m_items.at(index.row()).results);
    case ResultsStringRole:
        return downloadResultListToString(m_items.at(index.row()).results);
    default:
        return QVariant();
    }
}

QMap<int, QVariant> DownloadRequestModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant DownloadRequestModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap DownloadRequestModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList DownloadRequestModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
        Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int DownloadRequestModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void DownloadRequestModel::append(const QString &url) {
    Logger::log("DownloadRequestModel::append(): " + url, Logger::LowVerbosity);
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << DownloadRequest(url);
    endInsertRows();
    emit countChanged(rowCount());
    emit progressChanged(progress());

    if (status() != Active) {
        nextUrlCheck();
    }
}

void DownloadRequestModel::append(const QStringList &urls) {
    foreach (const QString &url, urls) {
        append(url);
    }
}

bool DownloadRequestModel::remove(int row) {
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

void DownloadRequestModel::cancel() {
    if (m_checker) {
        m_checker->cancel();
    }

    if (m_requester) {
        m_requester->cancel();
    }

    setStatus(Canceled);
}

void DownloadRequestModel::clear() {
    m_urls.clear();

    if (!m_items.isEmpty()) {
        cancel();
        beginResetModel();
        m_items.clear();
        m_index = -1;
        endResetModel();
        emit countChanged(0);
    }
}

bool DownloadRequestModel::submitCaptchaResponse(const QString &response) {
    if ((m_checker) && (m_checker->status() == UrlChecker::AwaitingCaptchaResponse)) {
        return m_checker->submitCaptchaResponse(response);
    }

    if ((m_requester) && (m_requester->status() == DownloadRequester::AwaitingCaptchaResponse)) {
        return m_requester->submitCaptchaResponse(response);
    }

    return false;
}

bool DownloadRequestModel::submitSettingsResponse(const QVariantMap &settings) {
    if (m_checker) {
        switch (m_checker->status()) {
        case UrlChecker::AwaitingDecaptchaSettingsResponse:
        case UrlChecker::AwaitingRecaptchaSettingsResponse:
        case UrlChecker::AwaitingServiceSettingsResponse:
            return m_checker->submitSettingsResponse(settings);
        default:
            break;
        }
    }

    if (m_requester) {
        switch (m_requester->status()) {
        case DownloadRequester::AwaitingDecaptchaSettingsResponse:
        case DownloadRequester::AwaitingRecaptchaSettingsResponse:
        case DownloadRequester::AwaitingServiceSettingsResponse:
            return m_requester->submitSettingsResponse(settings);
        default:
            break;
        }
    }

    return false;
}

void DownloadRequestModel::nextUrlCheck() {
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

void DownloadRequestModel::nextDownloadRequest() {
    if (!m_urls.isEmpty()) {
        setStatus(Active);
        requester()->getDownloadRequest(m_urls.first().url);
    }
    else {
        nextUrlCheck();
    }
}

UrlChecker* DownloadRequestModel::checker() {
    if (!m_checker) {
        m_checker = new UrlChecker(this);
        connect(m_checker, SIGNAL(captchaRequest(int, QByteArray)), this, SLOT(onCaptchaRequest(int, QByteArray)));
        connect(m_checker, SIGNAL(captchaTimeoutChanged(int)), this, SLOT(onCaptchaTimeoutChanged(int)));
        connect(m_checker, SIGNAL(error(QString)), this, SLOT(onCheckerError(QString)));
        connect(m_checker, SIGNAL(requestedSettingsTimeoutChanged(int)),
                this, SLOT(onRequestedSettingsTimeoutChanged(int)));
        connect(m_checker, SIGNAL(settingsRequest(QString, QVariantList)),
                this, SLOT(onSettingsRequest(QString, QVariantList)));
        connect(m_checker, SIGNAL(statusChanged(UrlChecker::Status)),
                this, SLOT(onCheckerStatusChanged(UrlChecker::Status)));
        connect(m_checker, SIGNAL(urlChecked(UrlResult)), this, SLOT(onUrlChecked(UrlResult)));
        connect(m_checker, SIGNAL(urlChecked(UrlResultList, QString)),
                this, SLOT(onUrlChecked(UrlResultList, QString)));
        connect(m_checker, SIGNAL(waitTimeChanged(int)), this, SLOT(onWaitTimeChanged(int)));
    }

    return m_checker;
}

DownloadRequester* DownloadRequestModel::requester() {
    if (!m_requester) {
        m_requester = new DownloadRequester(this);
        connect(m_requester, SIGNAL(captchaRequest(int, QByteArray)), this, SLOT(onCaptchaRequest(int, QByteArray)));
        connect(m_requester, SIGNAL(captchaTimeoutChanged(int)), this, SLOT(onCaptchaTimeoutChanged(int)));
        connect(m_requester, SIGNAL(downloadRequest(QNetworkRequest, QByteArray, QByteArray)),
                this, SLOT(onDownloadRequest(QNetworkRequest, QByteArray, QByteArray)));
        connect(m_requester, SIGNAL(error(QString)), this, SLOT(onRequesterError(QString)));
        connect(m_requester, SIGNAL(requestedSettingsTimeoutChanged(int)),
                this, SLOT(onRequestedSettingsTimeoutChanged(int)));
        connect(m_requester, SIGNAL(settingsRequest(QString, QVariantList)),
                this, SLOT(onSettingsRequest(QString, QVariantList)));
        connect(m_requester, SIGNAL(statusChanged(DownloadRequester::Status)),
                this, SLOT(onRequesterStatusChanged(DownloadRequester::Status)));
        connect(m_requester, SIGNAL(waitTimeChanged(int)), this, SLOT(onWaitTimeChanged(int)));
    }

    return m_requester;
}

void DownloadRequestModel::onCheckerStatusChanged(UrlChecker::Status s) {
    switch (s) {
    case UrlChecker::RetrievingCaptchaChallenge:
        setStatus(RetrievingCaptchaChallenge);
        break;
    case UrlChecker::RetrievingCaptchaResponse:
        setStatus(RetrievingCaptchaResponse);
        break;
    case UrlChecker::SubmittingCaptchaResponse:
        clearCaptchaData();
        setStatus(SubmittingCaptchaResponse);
        break;
    case UrlChecker::ReportingCaptchaResponse:
        setStatus(ReportingCaptchaResponse);
        break;
    case UrlChecker::SubmittingDecaptchaSettingsResponse:
    case UrlChecker::SubmittingRecaptchaSettingsResponse:
    case UrlChecker::SubmittingServiceSettingsResponse:
        clearRequestedSettings();
        setStatus(SubmittingSettingsResponse);
        break;
    case UrlChecker::WaitingActive:
        setStatus(WaitingActive);
        break;
    case UrlChecker::WaitingInactive:
        checker()->cancel();
        onCheckerError(tr("Must wait %1 for this URL check").arg(waitTimeString()));
        break;
    case UrlChecker::Canceled:
        clearCaptchaData();
        clearRequestedSettings();
        break;
    default:
        break;
    }
}

void DownloadRequestModel::onRequesterStatusChanged(DownloadRequester::Status s) {
    switch (s) {
    case DownloadRequester::RetrievingCaptchaChallenge:
        setStatus(RetrievingCaptchaChallenge);
        break;
    case DownloadRequester::RetrievingCaptchaResponse:
        setStatus(RetrievingCaptchaResponse);
        break;
    case DownloadRequester::SubmittingCaptchaResponse:
        clearCaptchaData();
        setStatus(SubmittingCaptchaResponse);
        break;
    case DownloadRequester::ReportingCaptchaResponse:
        setStatus(ReportingCaptchaResponse);
        break;
    case DownloadRequester::SubmittingDecaptchaSettingsResponse:
    case DownloadRequester::SubmittingRecaptchaSettingsResponse:
    case DownloadRequester::SubmittingServiceSettingsResponse:
        clearRequestedSettings();
        setStatus(SubmittingSettingsResponse);
        break;
    case DownloadRequester::WaitingActive:
        setStatus(WaitingActive);
        break;
    case DownloadRequester::WaitingInactive:
        checker()->cancel();
        onRequesterError(tr("Must wait %1 for this download request").arg(waitTimeString()));
        break;
    case DownloadRequester::Canceled:
        clearCaptchaData();
        clearRequestedSettings();
        break;
    default:
        break;
    }
}

void DownloadRequestModel::onCaptchaRequest(int captchaType, const QByteArray &captchaData) {
    setCaptchaData(captchaType, captchaData);
    setStatus(AwaitingCaptchaResponse);
    emit captchaRequest(captchaType, captchaData);
}

void DownloadRequestModel::onCaptchaTimeoutChanged(int timeout) {
    m_timeRemaining = timeout;
    emit captchaTimeoutChanged(timeout);
}

void DownloadRequestModel::onRequestedSettingsTimeoutChanged(int timeout) {
    m_timeRemaining = timeout;
    emit requestedSettingsTimeoutChanged(timeout);
}

void DownloadRequestModel::onSettingsRequest(const QString &title, const QVariantList &settings) {
    setRequestedSettings(title, settings);
    setStatus(AwaitingSettingsResponse);
    emit settingsRequest(title, settings);
}

void DownloadRequestModel::onWaitTimeChanged(int wait) {
    m_timeRemaining = wait;
    emit waitTimeChanged(wait);
}

void DownloadRequestModel::onUrlChecked(const UrlResult &result) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("DownloadRequestModel::onUrlChecked(): %1 1 URL found").arg(m_items.at(m_index).url),
            Logger::MediumVerbosity);
    m_urls << result;
    nextDownloadRequest();
}

void DownloadRequestModel::onUrlChecked(const UrlResultList &results, const QString &) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("DownloadRequestModel::onUrlChecked(): %1. %2 URLs found")
            .arg(m_items[m_index].url).arg(results.size()), Logger::MediumVerbosity);
    m_urls << results;
    nextDownloadRequest();
}

void DownloadRequestModel::onDownloadRequest(const QNetworkRequest &request, const QByteArray &method,
        const QByteArray &data) {
    if ((m_index < 0) || (m_index >= m_items.size()) || (m_urls.isEmpty())) {
        return;
    }

    Logger::log("DownloadRequestModel::onDownloadRequest(): " + request.url().toString(), Logger::MediumVerbosity);
    m_items[m_index].results << DownloadResult(m_urls.takeFirst().fileName, request, method, data);

    if (m_urls.isEmpty()) {
        m_items[m_index].checked = true;
        m_items[m_index].ok = true;
        const QModelIndex idx = index(m_index, 1);
        emit dataChanged(idx, idx);
    }

    clearCaptchaData();
    clearRequestedSettings();
    nextDownloadRequest();
}

void DownloadRequestModel::onCheckerError(const QString &errorString) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("DownloadRequestModel::onCheckerError(): %1. Error: %2").arg(m_items.at(m_index).url)
            .arg(errorString));
    m_items[m_index].checked = true;
    m_items[m_index].ok = false;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    clearCaptchaData();
    clearRequestedSettings();
    nextUrlCheck();
}

void DownloadRequestModel::onRequesterError(const QString &errorString) {
    if ((m_index < 0) || (m_index >= m_items.size()) || (m_urls.isEmpty())) {
        return;
    }

    Logger::log(QString("DownloadRequestModel::onRequesterError(): %1. Error: %2").arg(m_urls.takeFirst().url)
            .arg(errorString));

    if (m_urls.isEmpty()) {
        m_items[m_index].checked = true;
        m_items[m_index].ok = false;
        const QModelIndex idx = index(m_index, 1);
        emit dataChanged(idx, idx);
    }

    clearCaptchaData();
    clearRequestedSettings();
    nextDownloadRequest();
}
