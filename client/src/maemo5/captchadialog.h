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

#ifndef CAPTCHADIALOG_H
#define CAPTCHADIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QTimer;

class CaptchaDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setImage)
    Q_PROPERTY(QString response READ response WRITE setResponse)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(int timeRemaining READ timeRemaining)

public:
    explicit CaptchaDialog(QWidget *parent = 0);

    QImage image() const;
    void setImage(const QImage &i);

    QString response() const;
    void setResponse(const QString &r);

    int timeout() const;
    void setTimeout(int t);

    int timeRemaining() const;

private Q_SLOTS:
    void onResponseChanged(const QString &response);
    void updateTimeRemaining();

private:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    QTimer *m_timer;
    
    QLabel *m_imageLabel;
    QLabel *m_timeLabel;
    
    QLineEdit *m_responseEdit;
    
    QDialogButtonBox *m_buttonBox;
    
    QGridLayout *m_layout;

    QImage m_image;
    
    int m_timeout;
    int m_remaining;
};

#endif // CAPTCHADIALOG_H
