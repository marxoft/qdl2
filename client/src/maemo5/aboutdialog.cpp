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

#include "aboutdialog.h"
#include "aboutpluginsdialog.h"
#include "definitions.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_iconLabel(new QLabel(this)),
    m_textLabel(new QLabel(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this)),
    m_layout(new QGridLayout(this))
{
    setWindowTitle(tr("About"));

    m_iconLabel->setPixmap(QPixmap(DEFAULT_ICON));
    
    m_textLabel->setWordWrap(true);
    m_textLabel->setTextFormat(Qt::RichText);
    m_textLabel->setOpenExternalLinks(true);
    m_textLabel->setText(QString("<div style='text-align: center'><p style='font-size: 24pt; font-weight: bold;'>QDL-Client %1</p><p>A client application for QDL, a plugin-based download manager.</p><p>&copy; Stuart Howarth 2018</p><p><a href='http://marxoft.co.uk/projects/qdl'>marxoft.co.uk</a></p></div>").arg(VERSION_NUMBER));
    
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Plugins"));

    m_layout->addWidget(m_iconLabel, 0, 0);
    m_layout->addWidget(m_textLabel, 1, 0);
    m_layout->addWidget(m_buttonBox, 1, 1);
    m_layout->setRowStretch(1, 1);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(showPluginsDialog()));
}

void AboutDialog::showPluginsDialog() {
    AboutPluginsDialog(this).exec();
}
