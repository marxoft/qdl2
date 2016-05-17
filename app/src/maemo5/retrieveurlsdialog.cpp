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

#include "retrieveurlsdialog.h"
#include "serviceselectionmodel.h"
#include "settings.h"
#include "valueselector.h"
#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>

RetrieveUrlsDialog::RetrieveUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(new ServiceSelectionModel(this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_edit(new QTextEdit(m_container)),
    m_serviceSelector(new ValueSelector(tr("Service"), m_container)),
    m_button(new QPushButton(QIcon::fromTheme("general_add"), tr("Add"), m_container)),
    m_progressBar(new QProgressBar(m_container)),
    m_statusLabel(new QLabel(m_container)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Retrieve URLs"));
    setMinimumHeight(360);

    if (UrlRetrievalModel::instance()->status() != UrlRetrievalModel::Active) {
        UrlRetrievalModel::instance()->clear();
    }

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);
    
    m_edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_edit->setFocus(Qt::OtherFocusReason);
    
    m_serviceSelector->setModel(m_serviceModel);
    m_serviceSelector->setValue(Settings::defaultServicePlugin());
    
    m_button->setEnabled(false);

    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(UrlRetrievalModel::instance()->progress());
    
    m_vbox->addWidget(m_edit);
    m_vbox->addWidget(m_serviceSelector);
    m_vbox->addWidget(m_button);
    m_vbox->addWidget(m_progressBar);
    m_vbox->addWidget(m_statusLabel);
    m_vbox->addWidget(m_buttonBox);
    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);

    connect(UrlRetrievalModel::instance(), SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));
    connect(UrlRetrievalModel::instance(), SIGNAL(statusChanged(UrlRetrievalModel::Status)),
            this, SLOT(onStatusChanged(UrlRetrievalModel::Status)));
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(m_button, SIGNAL(clicked()), this, SLOT(addUrls()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));    

    onStatusChanged(UrlRetrievalModel::instance()->status());
}

void RetrieveUrlsDialog::accept() {
    Settings::setDefaultServicePlugin(m_serviceSelector->currentValue().toString());
    QDialog::accept();
}

void RetrieveUrlsDialog::reject() {
    UrlRetrievalModel::instance()->clear();
    QDialog::reject();
}

QStringList RetrieveUrlsDialog::results() const {
    return UrlRetrievalModel::instance()->results();
}

QString RetrieveUrlsDialog::text() const {
    return m_edit->toPlainText();
}

void RetrieveUrlsDialog::setText(const QString &text) {
    m_edit->setText(text);
}

QStringList RetrieveUrlsDialog::urls() const {
    return text().split(QRegExp("\\s+"));
}

void RetrieveUrlsDialog::setUrls(const QStringList &u) {
    setText(u.join("\n") + "\n");
}

void RetrieveUrlsDialog::addUrl(const QString &url) {
    m_edit->insertPlainText(url + "\n");
}

void RetrieveUrlsDialog::addUrls(const QStringList &urls) {
    foreach (const QString &url, urls) {
        addUrl(url);
    }
}

void RetrieveUrlsDialog::importUrls(const QString &fileName) {
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            const QUrl url = QUrl::fromUserInput(file.readLine());

            if (url.isValid()) {
                m_edit->insertPlainText(url.toString() + "\n");
            }
        }

        file.close();
    }
}

void RetrieveUrlsDialog::addUrls() {
    UrlRetrievalModel::instance()->append(m_edit->toPlainText().split(QRegExp("\\s+"), QString::SkipEmptyParts),
                                          m_serviceSelector->currentValue().toString());
    m_edit->clear();
}

void RetrieveUrlsDialog::onStatusChanged(UrlRetrievalModel::Status status) {
    m_statusLabel->setText(UrlRetrievalModel::instance()->statusString());

    switch (status) {
    case UrlRetrievalModel::Completed:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        break;
    default:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
}

void RetrieveUrlsDialog::onTextChanged() {
    m_button->setEnabled(!text().isEmpty());
}
