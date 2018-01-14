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

#ifndef NETWORKPROXYDIALOG_H
#define NETWORKPROXYDIALOG_H

#include <QDialog>

class NetworkProxyTypeModel;
class ValueSelector;
class QCheckBox;
class QDialogButtonBox;
class QHBoxLayout;
class QLineEdit;
class QScrollArea;
class QSpinBox;
class QVBoxLayout;

class NetworkProxyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkProxyDialog(QWidget *parent = 0);

public Q_SLOTS:
    virtual void accept();

private:
    NetworkProxyTypeModel *m_proxyTypeModel;

    QScrollArea *m_scrollArea;

    QWidget *m_container;
    
    QCheckBox *m_proxyCheckBox;
    QCheckBox *m_authCheckBox;

    ValueSelector *m_proxyTypeSelector;

    QLineEdit *m_hostEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;

    QSpinBox *m_portSpinBox;

    QDialogButtonBox *m_buttonBox;

    QVBoxLayout *m_vbox;
    QHBoxLayout *m_layout;
};

#endif // NETWORKPROXYDIALOG_H
