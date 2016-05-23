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

#include "pluginsettingsdialog.h"
#include "definitions.h"
#include "graphicsview.h"
#include "logger.h"
#include "utils.h"
#include <QCoreApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeItem>
#include <QVBoxLayout>

PluginSettingsDialog::PluginSettingsDialog(const QVariantList &settings, QWidget *parent) :
    QDialog(parent),
    m_view(new GraphicsView(this)),
    m_engine(new QDeclarativeEngine(this)),
    m_item(0),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Settings"));
    setWindowFlags(Qt::Window);
    
    m_engine->rootContext()->setContextProperty("utils", new Utils(m_engine));
    QDeclarativeComponent component(m_engine, QUrl::fromLocalFile(QCoreApplication::applicationDirPath()
                                    + "/qml/PluginSettingsWindow.qml"));
    QObject *obj = component.create();
    
    if (obj) {
        m_item = qobject_cast<QDeclarativeItem*>(obj);
        
        if (m_item) {
            m_view->addItem(m_item);
            m_item->setProperty("settings", settings);
            m_item->setProperty("timeout", CAPTCHA_TIMEOUT);
            connect(m_item, SIGNAL(accepted()), this, SLOT(accept()));
            connect(m_item, SIGNAL(rejected()), this, SLOT(reject()));
        }
        else {
            Logger::log("PluginSettingsDialog::PluginSettingsDialog(): Cannot cast created object to QDeclarativeItem*");
            delete obj;
        }
    }
    else {
        Logger::log("PluginSettingsDialog::PluginSettingsDialog(): Created object is NULL");
    }
    
    m_layout->addWidget(m_view);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

QVariantMap PluginSettingsDialog::settings() const {
    return m_item ? m_item->property("response").toMap() : QVariantMap();
}

int PluginSettingsDialog::timeout() const {
    return m_item ? m_item->property("timeout").toInt() : 0;
}

void PluginSettingsDialog::setTimeout(int t) {
    if (m_item) {
        m_item->setProperty("timeout", t);
    }
}

int PluginSettingsDialog::timeRemaining() const {
    return m_item ? m_item->property("timeRemaining").toInt() : 0;
}

void PluginSettingsDialog::showEvent(QShowEvent *event) {
    if (m_item) {
        m_item->setProperty("title", windowTitle());
        QMetaObject::invokeMethod(m_item, "startTimer");
    }
    
    QDialog::showEvent(event);
}

void PluginSettingsDialog::closeEvent(QCloseEvent *event) {
    if (m_item) {
        QMetaObject::invokeMethod(m_item, "stopTimer");
    }
    
    QDialog::closeEvent(event);
}
