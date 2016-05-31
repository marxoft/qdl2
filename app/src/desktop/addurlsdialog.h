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

#ifndef ADDURLSDIALOG_H
#define ADDURLSDIALOG_H

#include <QDialog>
#include <QVariantMap>

class CategorySelectionModel;
class SelectionModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLineEdit;
class QStackedWidget;
class QTabBar;
class QTextEdit;
class QTreeView;
class QVBoxLayout;

class AddUrlsDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString postData READ postData WRITE setPostData)
    Q_PROPERTY(QVariantMap requestHeaders READ requestHeaders WRITE setRequestHeaders)
    Q_PROPERTY(QString requestMethod READ requestMethod WRITE setRequestMethod)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QStringList urls READ urls WRITE setUrls)
    Q_PROPERTY(bool usePlugins READ usePlugins WRITE setUsePlugin)

public:
    explicit AddUrlsDialog(QWidget *parent = 0);
    
    QString postData() const;
    
    QVariantMap requestHeaders() const;
    
    QString requestMethod() const;

    QString text() const;
    
    QStringList urls() const;
    
    bool usePlugins() const;

public Q_SLOTS:
    virtual void accept();
    
    void setPostData(const QString &data);
    
    void setRequestHeaders(const QVariantMap &headers);
    void addRequestHeader(const QString &name, const QVariant &value);
    
    void setRequestMethod(const QString &method);
    
    void setText(const QString &t);

    void setUrls(const QStringList &u);

    void addUrl(const QString &url);
    void addUrls(const QStringList &urls);

    void importUrls(const QString &fileName);
    
    void setUsePlugin(bool enabled);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private Q_SLOTS:
    void setCurrentTab(int index);
    void showUrlsTab();
    void showMethodTab();
    void showHeadersTab();
    
    void addRequestHeader();
    void showRequestHeaderContextMenu(const QPoint &pos);

    void onPostDataChanged(const QString &data);
    void onRequestHeaderNameChanged(const QString &name);
    void onRequestHeaderValueChanged(const QString &value);
    void onRequestMethodChanged(const QString &method);
    void onUrlsChanged();
    void onUsePluginsChanged(bool enabled);

private:
    CategorySelectionModel *m_categoryModel;
    SelectionModel *m_headerModel;
    
    QTabBar *m_tabBar;
    
    QStackedWidget *m_stack;
    
    QDialogButtonBox *m_buttonBox;
    
    QVBoxLayout *m_layout;
    
    QWidget *m_urlsTab;
    
    QTextEdit *m_urlsEdit;
    
    QComboBox *m_categorySelector;
    
    QCheckBox *m_pluginCheckBox;
        
    QFormLayout *m_urlsLayout;
    
    QWidget *m_methodTab;

    QLineEdit *m_methodEdit;
    QLineEdit *m_postEdit;

    QFormLayout *m_methodLayout;

    QWidget *m_headersTab;
    
    QTreeView *m_headerView;
    
    QLineEdit *m_headerNameEdit;
    QLineEdit *m_headerValueEdit;
    
    QPushButton *m_headerButton;
    
    QFormLayout *m_headerLayout;
    
    QString m_method;
    QString m_postData;
};

#endif // ADDURLSDIALOG_H
