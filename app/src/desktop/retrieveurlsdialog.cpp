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
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFile>
#include <QFormLayout>
#include <QLabel>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>

RetrieveUrlsDialog::RetrieveUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(new ServiceSelectionModel(this)),
    m_edit(new QTextEdit(this)),
    m_serviceSelector(new QComboBox(this)),
    m_button(new QPushButton(QIcon::fromTheme("list-add"), tr("&Add"), this)),
    m_progressBar(new QProgressBar(this)),
    m_statusLabel(new QLabel(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Retrieve URLs"));
    setAcceptDrops(true);
    
    m_serviceSelector->setModel(m_serviceModel);
    m_serviceSelector->setCurrentIndex(m_serviceSelector->findData(Settings::defaultServicePlugin()));
    
    m_button->setEnabled(false);

    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(UrlRetrievalModel::instance()->progress());
    
    m_layout->addRow(m_edit);
    m_layout->addRow(tr("&Service:"), m_serviceSelector);
    m_layout->addWidget(m_button);
    m_layout->addRow(m_progressBar);
    m_layout->addRow(m_statusLabel);
    m_layout->addRow(m_buttonBox);

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
    Settings::setDefaultServicePlugin(m_serviceSelector->itemData(m_serviceSelector->currentIndex()).toString());
    QDialog::accept();
}

void RetrieveUrlsDialog::reject() {
    clear();
    QDialog::reject();
}

void RetrieveUrlsDialog::clear() {
    UrlRetrievalModel::instance()->clear();
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
    return text().split("\n", QString::SkipEmptyParts);
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

void RetrieveUrlsDialog::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasUrls()) && (event->mimeData()->urls().first().path().toLower().endsWith(".txt"))) {
        event->acceptProposedAction();
    }
}

void RetrieveUrlsDialog::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        const QString fileName = event->mimeData()->urls().first().path();

        if ((QFile::exists(fileName)) && (fileName.toLower().endsWith(".txt"))) {
            importUrls(fileName);
        }
    }
}

void RetrieveUrlsDialog::addUrls() {
    UrlRetrievalModel::instance()->append(m_edit->toPlainText().split("\n", QString::SkipEmptyParts),
                                          m_serviceSelector->itemData(m_serviceSelector->currentIndex()).toString());
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
