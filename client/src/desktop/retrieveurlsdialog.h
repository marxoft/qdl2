/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef RETRIEVEURLSDIALOG_H
#define RETRIEVEURLSDIALOG_H

#include "urlretrievalmodel.h"
#include <QDialog>

class ServiceSelectionModel;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLabel;
class QProgressBar;
class QPushButton;
class QTextEdit;

class RetrieveUrlsDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QStringList results READ results)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QStringList urls READ urls WRITE setUrls)

public:
    explicit RetrieveUrlsDialog(QWidget *parent = 0);

    QStringList results() const;

    QString text() const;
    
    QStringList urls() const;

public Q_SLOTS:
    virtual void accept();
    virtual void reject();
    
    virtual void clear();
    
    void setText(const QString &t);

    void setUrls(const QStringList &u);

    void addUrl(const QString &url);
    void addUrls(const QStringList &urls);

    void importUrls(const QString &fileName);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private Q_SLOTS:
    void addUrls();
    
    void onStatusChanged(UrlRetrievalModel::Status status);
    void onTextChanged();

private:
    ServiceSelectionModel *m_serviceModel;
    
    QTextEdit *m_edit;
    
    QComboBox *m_serviceSelector;

    QPushButton *m_button;

    QProgressBar *m_progressBar;

    QLabel *m_statusLabel;

    QDialogButtonBox *m_buttonBox;

    QFormLayout *m_layout;
};

#endif // RETRIEVEURLSDIALOG_H
