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

#include "pluginsettingspage.h"
#include "pluginsettings.h"
#include "selectionmodel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>

PluginSettingsPage::PluginSettingsPage(const QString &pluginId, const QVariantList &settings, QWidget *parent) :
    SettingsPage(parent),
    m_plugin(new PluginSettings(pluginId, this)),
    m_layout(new QFormLayout(this))
{
    foreach (const QVariant &setting, settings) {
        addWidget(m_layout, setting.toMap());
    }
}

void PluginSettingsPage::save() {
    if (m_settings.isEmpty()) {
        return;
    }

    QMapIterator<QString, QVariant> iterator(m_settings);

    while (iterator.hasNext()) {
        iterator.next();
        m_plugin->setValue(iterator.key(), iterator.value());
    }
}

bool PluginSettingsPage::eventFilter(QObject *obj, QEvent *event) {
    if (const QMenu *menu = qobject_cast<QMenu*>(obj)) {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (QAction *action = menu->activeAction()) {
                action->trigger();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress) {
            if (const QKeyEvent *key = static_cast<QKeyEvent*>(event)) {
                switch (key->key()) {
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                        if (QAction *action = menu->activeAction()) {
                            action->trigger();
                            return true;
                        }

                        break;
                    default:
                        break;
                }
            }
        }
    }

    return SettingsPage::eventFilter(obj, event);
}

void PluginSettingsPage::addCheckBox(QFormLayout *layout, const QString &label, const QString &key, bool value) {
    QCheckBox *checkbox = new QCheckBox("&" + label, this);
    checkbox->setProperty("key", key);
    checkbox->setChecked(value);
    layout->addRow(checkbox);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(setBooleanValue(bool)));
}

void PluginSettingsPage::addComboBox(QFormLayout *layout, const QString &label, const QString &key,
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

void PluginSettingsPage::addGroupBox(QFormLayout *layout, const QString &label, const QString &key,
                                     const QVariantList &settings) {
    QGroupBox *groupbox = new QGroupBox("&" + label, this);
    QFormLayout *form = new QFormLayout(groupbox);

    foreach (const QVariant &setting, settings) {
        addWidget(form, setting.toMap(), key);
    }

    layout->addRow(groupbox);
}

void PluginSettingsPage::addLineEdit(QFormLayout *layout, const QString &label, const QString &key,
                                     const QString &value, bool isPassword) {
    QLineEdit *edit = new QLineEdit(value, this);
    edit->setProperty("key", key);

    if (isPassword) {
        edit->setEchoMode(QLineEdit::Password);
    }

    layout->addRow("&" + label + ":", edit);
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(setTextValue(QString)));
}

void PluginSettingsPage::addMenuButton(QFormLayout *layout, const QString &label, const QString &key,
                                         const QVariantList &options, const QVariantList &values) {
    QPushButton *button = new QPushButton(tr("Select options"), this);
    QMenu *menu = new QMenu(button);
    menu->installEventFilter(this);
    button->setMenu(menu);

    foreach (const QVariant &var, options) {
        const QVariantMap option = var.toMap();
        QAction *action = menu->addAction(option.value("label").toString());
        action->setProperty("key", key);
        action->setData(option.value("value"));
        action->setCheckable(true);
        action->setChecked(values.contains(option.value("value")));
    }

    layout->addRow("&" + label + ":", button);
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(setMultiListValue(QAction*)));
}

void PluginSettingsPage::addSpinBox(QFormLayout *layout, const QString &label, const QString &key, int minimum,
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

void PluginSettingsPage::addWidget(QFormLayout *layout, const QVariantMap &setting, const QString &group) {
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
        if (setting.value("multiselect").toBool()) {
            addMenuButton(layout, setting.value("label").toString(), key, setting.value("options").toList(),
                          value.toList());
        }
        else {
            addComboBox(layout, setting.value("label").toString(), key, setting.value("options").toList(), value);
        }
    }
    else if (type == "password") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString(), true);
    }
    else if (type == "text") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString());
    }
}

void PluginSettingsPage::setBooleanValue(bool value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginSettingsPage::setIntegerValue(int value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginSettingsPage::setListValue(int value) {
    if (const QComboBox *combobox = qobject_cast<QComboBox*>(sender())) {
        m_settings[combobox->property("key").toString()] = combobox->itemData(value);
    }
}

void PluginSettingsPage::setMultiListValue(QAction *action) {
    const QString key = action->property("key").toString();
    QVariantList list = m_settings.value(key).toList();

    if (action->isChecked()) {
        list << action->data();
    }
    else {
        list.removeOne(action->data());
    }

    m_settings[key] = list;
}

void PluginSettingsPage::setTextValue(const QString &value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}
