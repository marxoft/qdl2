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
#include "selectionmodel.h"
#include "utils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QTimer>

PluginSettingsDialog::PluginSettingsDialog(const QVariantList &settings, QWidget *parent) :
    QDialog(parent),
    m_timer(new QTimer(this)),
    m_scrollArea(new QScrollArea(this)),
    m_widget(new QWidget(m_scrollArea)),
    m_form(new QFormLayout(m_widget)),
    m_timeLabel(new QLabel(Utils::formatMSecs(CAPTCHA_TIMEOUT), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this)),
    m_timeout(CAPTCHA_TIMEOUT),
    m_remaining(CAPTCHA_TIMEOUT)
{
    setWindowTitle(tr("Please enter settings"));
    
    foreach (const QVariant &setting, settings) {
        addWidget(m_form, setting.toMap());
    }

    m_timer->setInterval(1000);

    m_scrollArea->setWidget(m_widget);
    m_scrollArea->setWidgetResizable(true);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_timeLabel);
    m_layout->addWidget(m_buttonBox);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimeRemaining()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QVariantMap PluginSettingsDialog::settings() const {
    return m_settings;
}

int PluginSettingsDialog::timeout() const {
    return m_timeout;
}

void PluginSettingsDialog::setTimeout(int t) {
    m_timeout = t;
    m_remaining = t;
    
    if (t > 0) {
        m_timeLabel->show();
        m_timer->start();
    }
    else {
        m_timeLabel->hide();
        m_timer->stop();
    }
}

int PluginSettingsDialog::timeRemaining() const {
    return m_remaining;
}

void PluginSettingsDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    if (!m_timer->isActive()) {
        m_remaining = timeout();
        
        if (m_remaining > 0) {
            m_timer->start();
        }
    }
}

void PluginSettingsDialog::closeEvent(QCloseEvent *event) {
    m_timer->stop();
    QDialog::closeEvent(event);
}

void PluginSettingsDialog::addCheckBox(QFormLayout *layout, const QString &label, const QString &key, bool value) {
    QCheckBox *checkbox = new QCheckBox("&" + label, this);
    checkbox->setProperty("key", key);
    checkbox->setChecked(value);
    layout->addRow(checkbox);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(setBooleanValue(bool)));
}

void PluginSettingsDialog::addComboBox(QFormLayout *layout, const QString &label, const QString &key,
                                    const QVariantList &options, const QVariant &value) {
    QComboBox *combobox = new QComboBox(this);
    SelectionModel *model = new SelectionModel(combobox);
    combobox->setProperty("key", key);
    combobox->setModel(model);

    foreach (const QVariant &var, options) {
        const QVariantMap option = var.toMap();
        model->append(option.value("label").toString(), option.value("value"));
    }

    combobox->setCurrentIndex(qMax(0, combobox->findData(value)));
    layout->addRow("&" + label + ":", combobox);
    connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(setListValue(int)));
}

void PluginSettingsDialog::addGroupBox(QFormLayout *layout, const QString &label, const QString &key,
                                    const QVariantList &settings) {
    QGroupBox *groupbox = new QGroupBox("&" + label, this);
    QFormLayout *form = new QFormLayout(groupbox);

    foreach (const QVariant &setting, settings) {
        addWidget(form, setting.toMap(), key);
    }

    layout->addRow(groupbox);
}

void PluginSettingsDialog::addLineEdit(QFormLayout *layout, const QString &label, const QString &key,
                                    const QString &value, bool isPassword) {
    QLineEdit *edit = new QLineEdit(value, this);
    edit->setProperty("key", key);

    if (isPassword) {
        edit->setEchoMode(QLineEdit::Password);
    }

    layout->addRow("&" + label + ":", edit);
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(setTextValue(QString)));
}

void PluginSettingsDialog::addSpinBox(QFormLayout *layout, const QString &label, const QString &key, int minimum,
                                   int maximum, int step, int value) {
    QSpinBox *spinbox = new QSpinBox(this);
    spinbox->setProperty("key", key);
    spinbox->setMinimum(minimum);
    spinbox->setMaximum(maximum);
    spinbox->setSingleStep(step);
    spinbox->setValue(value);
    layout->addRow("&" + label + ":", spinbox);
    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(setIntegerValue(int)));
}

void PluginSettingsDialog::addWidget(QFormLayout *layout, const QVariantMap &setting, const QString &group) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }

    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();
    const QVariant value = setting.value("value");
    m_settings[key] = value;

    if (type == "boolean") {
        addCheckBox(layout, setting.value("label").toString(), key, value.toBool());
    }
    else if (type == "group") {
        addGroupBox(layout, setting.value("label").toString(), key, setting.value("settings").toList());
    }
    else if (type == "integer") {
        addSpinBox(layout, setting.value("label").toString(), key, setting.value("minimum", 0).toInt(),
                   setting.value("maximum", 100).toInt(), setting.value("step", 1).toInt(), value.toInt());
    }
    else if (type == "list") {
        addComboBox(layout, setting.value("label").toString(), key, setting.value("options").toList(), value);
    }
    else if (type == "password") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString(), true);
    }
    else if (type == "text") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString());
    }
}

void PluginSettingsDialog::setBooleanValue(bool value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginSettingsDialog::setIntegerValue(int value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginSettingsDialog::setListValue(int value) {
    if (const QComboBox *combobox = qobject_cast<QComboBox*>(sender())) {
        m_settings[combobox->property("key").toString()] = combobox->itemData(value);
    }
}

void PluginSettingsDialog::setTextValue(const QString &value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginSettingsDialog::updateTimeRemaining() {
    m_remaining -= m_timer->interval();
    m_timeLabel->setText(Utils::formatMSecs(m_remaining));

    if (m_remaining <= 0) {
        reject();
    }
}
