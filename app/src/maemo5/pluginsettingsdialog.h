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

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QScrollArea;
class QTimer;
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

private Q_SLOTS:
    void setBooleanValue(bool value);
    void setIntegerValue(int value);
    void setListValue(const QVariant &value);
    void setMultiListValue(const QVariantList &values);
    void setTextValue(const QString &value);

    void updateTimeRemaining();

private:
    void addCheckBox(const QString &label, const QString &key, bool value);
    void addGroup(const QString &label, const QString &key, const QVariantList &settings);
    void addLineEdit(const QString &label, const QString &key, const QString &value, bool isPassword = false);
    void addMultiValueSelector(const QString &label, const QString &key, const QVariantList &options,
                               const QVariantList &values);
    void addSpinBox(const QString &label, const QString &key, int minimum, int maximum, int step, int value);
    void addValueSelector(const QString &label, const QString &key, const QVariantList &options, const QVariant &value);
    void addWidget(const QVariantMap &setting, const QString &group = QString());

    QTimer *m_timer;
    
    QScrollArea *m_scrollArea;
    
    QWidget *m_container;
    
    QLabel *m_timeLabel;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_vbox;
    QGridLayout *m_layout;

    QVariantMap m_settings;

    int m_timeout;
    int m_remaining;
};

#endif // PLUGINSETTINGSDIALOG_H
