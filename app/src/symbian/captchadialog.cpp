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

#include "captchadialog.h"
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

CaptchaDialog::CaptchaDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new GraphicsView(this)),
    m_engine(new QDeclarativeEngine(this)),
    m_item(0),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Captcha"));
    setWindowFlags(Qt::Window);
    
    m_engine->rootContext()->setContextProperty("utils", new Utils(m_engine));
    QDeclarativeComponent component(m_engine, QUrl::fromLocalFile(QCoreApplication::applicationDirPath()
                                    + "/qml/CaptchaWindow.qml"));
    QObject *obj = component.create();
    
    if (obj) {
        m_item = qobject_cast<QDeclarativeItem*>(obj);
        
        if (m_item) {
            m_view->addItem(m_item);
            m_item->setProperty("timeout", CAPTCHA_TIMEOUT);
            connect(m_item, SIGNAL(accepted()), this, SLOT(accept()));
            connect(m_item, SIGNAL(rejected()), this, SLOT(reject()));
        }
        else {
            Logger::log("CaptchaDialog::CaptchaDialog(): Cannot cast created object to QDeclarativeItem*");
            delete obj;
        }
    }
    else {
        Logger::log("CaptchaDialog::CaptchaDialog(): Created object is NULL");
    }
    
    m_layout->addWidget(m_view);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

QImage CaptchaDialog::image() const {
    return m_image;
}

void CaptchaDialog::setImage(const QImage &i) {
    m_image = i;
    
    if (m_item) {
        m_item->setProperty("image", QPixmap::fromImage(i));
    }
}

QString CaptchaDialog::response() const {
    return m_item ? m_item->property("response").toString() : QString();
}

void CaptchaDialog::setResponse(const QString &r) {
    if (m_item) {
        m_item->setProperty("response", r);
    }
}

int CaptchaDialog::timeout() const {
    return m_item ? m_item->property("timeout").toInt() : 0;
}

void CaptchaDialog::setTimeout(int t) {
    if (m_item) {
        m_item->setProperty("timeout", t);
    }
}

int CaptchaDialog::timeRemaining() const {
    return m_item ? m_item->property("timeRemaining").toInt() : 0;
}

void CaptchaDialog::showEvent(QShowEvent *event) {
    if (m_item) {
        m_item->setProperty("title", windowTitle());
        QMetaObject::invokeMethod(m_item, "startTimer");
    }
    
    QDialog::showEvent(event);
}

void CaptchaDialog::closeEvent(QCloseEvent *event) {
    if (m_item) {
        QMetaObject::invokeMethod(m_item, "stopTimer");
    }
    
    QDialog::closeEvent(event);
}
