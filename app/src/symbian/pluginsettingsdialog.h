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

#ifndef PLUGINSETTINGSDIALOG_H
#define PLUGINSETTINGSDIALOG_H

#include <QDialog>
#include <QVariantMap>

class GraphicsView;
class QDeclarativeEngine;
class QDeclarativeItem;
class QVBoxLayout;

class PluginSettingsDialog : public QDialog
{
    Q_OBJECT
    
    Q_PROPERTY(QVariantMap settings READ settings)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(int timeRemaining READ timeRemaining)

public:
    explicit PluginSettingsDialog(const QVariantList &settings, QWidget *parent = 0);

    QVariantMap settings() const;

    int timeout() const;
    void setTimeout(int t);

    int timeRemaining() const;
    
protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    GraphicsView *m_view;
    
    QDeclarativeEngine *m_engine;
    
    QDeclarativeItem *m_item;
    
    QVBoxLayout *m_layout;
    
    QImage m_image;
    
    int m_timeout;
    int m_remaining;
};

#endif // PLUGINSETTINGSDIALOG_H
