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

#include "plugindialog.h"
#include "multivalueselector.h"
#include "pluginsettings.h"
#include "selectionmodel.h"
#include "valueselector.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

PluginDialog::PluginDialog(const QString &pluginId, const QVariantList &settings, QWidget *parent) :
    QDialog(parent),
    m_plugin(new PluginSettings(pluginId, this)),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget(m_scrollArea)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_vbox(new QVBoxLayout(m_container)),
    m_layout(new QHBoxLayout(this))
{
    setWindowTitle(tr("Plugin settings"));
    
    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);

    m_vbox->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_scrollArea);
    m_layout->addWidget(m_buttonBox);
    
    foreach (const QVariant &setting, settings) {
        addWidget(setting.toMap());
    }

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void PluginDialog::accept() {
    QMapIterator<QString, QVariant> iterator(m_settings);

    while (iterator.hasNext()) {
        iterator.next();
        m_plugin->setValue(iterator.key(), iterator.value());
    }

    QDialog::accept();
}

void PluginDialog::addCheckBox(const QString &label, const QString &key, bool value) {
    QCheckBox *checkbox = new QCheckBox(label, m_container);
    checkbox->setProperty("key", key);
    checkbox->setChecked(value);
    m_vbox->addWidget(checkbox);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(setBooleanValue(bool)));
}

void PluginDialog::addGroup(const QString &label, const QString &key, const QVariantList &settings) {
    m_vbox->addWidget(new QLabel(QString("<p align='center'; style='color: %1'>%2</p>")
                                        .arg(palette().color(QPalette::Mid).name()).arg(label), m_container));

    foreach (const QVariant &setting, settings) {
        addWidget(setting.toMap(), key);
    }
}

void PluginDialog::addLineEdit(const QString &label, const QString &key, const QString &value, bool isPassword) {
    QLineEdit *edit = new QLineEdit(value, m_container);
    edit->setProperty("key", key);

    if (isPassword) {
        edit->setEchoMode(QLineEdit::Password);
    }
    
    if (!focusWidget()) {
        edit->setFocus();
    }

    m_vbox->addWidget(new QLabel(label, m_container));
    m_vbox->addWidget(edit);
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(setTextValue(QString)));
}

void PluginDialog::addMultiValueSelector(const QString &label, const QString &key, const QVariantList &options,
                                         const QVariantList &values) {
    MultiValueSelector *selector = new MultiValueSelector(label, m_container);
    SelectionModel *model = new SelectionModel(selector);
    selector->setProperty("key", key);
    selector->setModel(model);

    foreach (const QVariant &var, options) {
        const QVariantMap option = var.toMap();
        model->append(option.value("label").toString(), option.value("value"));
    }

    selector->setValues(values);
    m_vbox->addWidget(selector);
    connect(selector, SIGNAL(valuesChanged(QVariantList)), this, SLOT(setMultiListValue(QVariantList)));
}

void PluginDialog::addSpinBox(const QString &label, const QString &key, int minimum, int maximum, int step,
                              int value) {
    QSpinBox *spinbox = new QSpinBox(m_container);
    spinbox->setProperty("key", key);
    spinbox->setMinimum(minimum);
    spinbox->setMaximum(maximum);
    spinbox->setSingleStep(step);
    spinbox->setValue(value);
    m_vbox->addWidget(new QLabel(label, m_container));
    m_vbox->addWidget(spinbox);
    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(setIntegerValue(int)));
}

void PluginDialog::addValueSelector(const QString &label, const QString &key, const QVariantList &options,
                                    const QVariant &value) {
    ValueSelector *selector = new ValueSelector(label, m_container);
    SelectionModel *model = new SelectionModel(selector);
    selector->setProperty("key", key);
    selector->setModel(model);

    foreach (const QVariant &var, options) {
        const QVariantMap option = var.toMap();
        model->append(option.value("label").toString(), option.value("value"));
    }

    selector->setValue(value);
    m_vbox->addWidget(selector);
    connect(selector, SIGNAL(valueChanged(QVariant)), this, SLOT(setListValue(QVariant)));
}

void PluginDialog::addWidget(const QVariantMap &setting, const QString &group) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }

    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();
    const QVariant value = m_plugin->value(key, setting.value("value"));

    if (type == "boolean") {
        addCheckBox(setting.value("label").toString(), key, value.toBool());
    }
    else if (type == "group") {
        addGroup(setting.value("label").toString(), key, setting.value("settings").toList());
    }
    else if (type == "integer") {
        addSpinBox(setting.value("label").toString(), key, setting.value("minimum", 0).toInt(),
                   setting.value("maximum", 100).toInt(), setting.value("step", 1).toInt(), value.toInt());
    }
    else if (type == "list") {
        if (setting.value("multiselect").toBool()) {
            addMultiValueSelector(setting.value("label").toString(), key, setting.value("options").toList(),
                                  value.toList());
        }
        else {
            addValueSelector(setting.value("label").toString(), key, setting.value("options").toList(), value);
        }
    }
    else if (type == "password") {
        addLineEdit(setting.value("label").toString(), key, value.toString(), true);
    }
    else if (type == "text") {
        addLineEdit(setting.value("label").toString(), key, value.toString());
    }
}

void PluginDialog::setBooleanValue(bool value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setIntegerValue(int value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setListValue(const QVariant &value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setMultiListValue(const QVariantList &values) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = values;
    }
}

void PluginDialog::setTextValue(const QString &value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}
