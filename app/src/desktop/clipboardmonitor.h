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

#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include <QObject>

class ClipboardMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QStringList urls READ urls NOTIFY urlsAdded)

public:
    ~ClipboardMonitor();

    static ClipboardMonitor* instance();

    bool isEnabled() const;

    QStringList urls() const;

public Q_SLOTS:
    void setEnabled(bool enabled);
    
    void clear();

private Q_SLOTS:
    void onClipboardDataChanged();

Q_SIGNALS:
    void enabledChanged(bool enabled);
    void urlsAdded(const QStringList &urls);

private:
    ClipboardMonitor();

    static ClipboardMonitor *self;

    bool m_enabled;
    
    QStringList m_urls;
};

#endif // CLIPBOARDMONITOR_H
