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

#include "aboutdialog.h"
#include "decaptchapluginconfigmodel.h"
#include "definitions.h"
#include "recaptchapluginconfigmodel.h"
#include "servicepluginconfigmodel.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QListView>
#include <QTabBar>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_serviceModel(0),
    m_recaptchaModel(0),
    m_decaptchaModel(0),
    m_iconLabel(new QLabel(this)),
    m_textLabel(new QLabel(this)),
    m_view(new QListView(this)),
    m_tabBar(new QTabBar(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("About"));

    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setPixmap(QPixmap(DEFAULT_ICON));
    
    m_textLabel->setWordWrap(true);
    m_textLabel->setTextFormat(Qt::RichText);
    m_textLabel->setOpenExternalLinks(true);
    m_textLabel->setText(QString("<div style='text-align: center'><p style='font-size: 16pt; font-weight: bold;'>QDL</p><p>Version: %1</p><p>A plugin-based download manager.</p><p>&copy; Stuart Howarth 2016</p><p><a href='http://marxoft.co.uk/projects/qdl'>marxoft.co.uk</a></p></div><p>Installed plugins:</p>").arg(VERSION_NUMBER));

    m_view->setUniformItemSizes(true);
    
    m_tabBar->addTab(tr("Services"));
    m_tabBar->addTab(tr("Recaptcha"));
    m_tabBar->addTab(tr("Decaptcha"));

    m_layout->addWidget(m_iconLabel);
    m_layout->addWidget(m_textLabel);
    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_view);
    m_layout->addWidget(m_buttonBox);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    showServicePlugins();
}

void AboutDialog::showServicePlugins() {
    if (!m_serviceModel) {
        m_serviceModel = new ServicePluginConfigModel(this);
    }

    m_view->setModel(m_serviceModel);
}

void AboutDialog::showRecaptchaPlugins() {
    if (!m_recaptchaModel) {
        m_recaptchaModel = new RecaptchaPluginConfigModel(this);
    }

    m_view->setModel(m_recaptchaModel);
}

void AboutDialog::showDecaptchaPlugins() {
    if (!m_decaptchaModel) {
        m_decaptchaModel = new DecaptchaPluginConfigModel(this);
    }

    m_view->setModel(m_decaptchaModel);
}

void AboutDialog::onTabChanged(int index) {
    switch (index) {
    case 0:
        showServicePlugins();
        break;
    case 1:
        showRecaptchaPlugins();
        break;
    case 2:
        showDecaptchaPlugins();
        break;
    default:
        break;
    }
}
