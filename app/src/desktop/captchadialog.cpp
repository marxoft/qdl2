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
#include "utils.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

CaptchaDialog::CaptchaDialog(QWidget *parent) :
    QDialog(parent),
    m_timer(new QTimer(this)),
    m_imageLabel(new QLabel(this)),
    m_timeLabel(new QLabel(Utils::formatMSecs(CAPTCHA_TIMEOUT), this)),
    m_responseEdit(new QLineEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this)),
    m_timeout(CAPTCHA_TIMEOUT),
    m_remaining(CAPTCHA_TIMEOUT)
{
    setWindowTitle(tr("Please complete captcha"));

    m_timer->setInterval(1000);

    m_imageLabel->setScaledContents(true);

    m_layout->addWidget(m_imageLabel);
    m_layout->addWidget(m_timeLabel);
    m_layout->addWidget(m_responseEdit);
    m_layout->addWidget(m_buttonBox);

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimeRemaining()));
    connect(m_responseEdit, SIGNAL(textChanged(QString)), this, SLOT(onResponseChanged(QString)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QImage CaptchaDialog::image() const {
    return m_image;
}

void CaptchaDialog::setImage(const QImage &i) {
    m_image = i;
    m_imageLabel->setPixmap(QPixmap::fromImage(i));
}

QString CaptchaDialog::response() const {
    return m_responseEdit->text();
}

void CaptchaDialog::setResponse(const QString &r) {
    m_responseEdit->setText(r);
}

int CaptchaDialog::timeout() const {
    return m_timeout;
}

void CaptchaDialog::setTimeout(int t) {
    m_timeout = t;
    m_remaining = t;
    m_timer->start();
}

int CaptchaDialog::timeRemaining() const {
    return m_remaining;
}

void CaptchaDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    if (!m_timer->isActive()) {
        m_remaining = timeout();
        m_timer->start();
    }
}

void CaptchaDialog::closeEvent(QCloseEvent *event) {
    m_timer->stop();
    QDialog::closeEvent(event);
}

void CaptchaDialog::onResponseChanged(const QString &response) {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!response.isEmpty());
}

void CaptchaDialog::updateTimeRemaining() {
    m_remaining -= m_timer->interval();
    m_timeLabel->setText(Utils::formatMSecs(m_remaining));

    if (m_remaining <= 0) {
        reject();
    }
}
