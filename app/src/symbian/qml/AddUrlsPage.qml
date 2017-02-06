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

import QtQuick 1.1
import com.nokia.symbian 1.1
import Qdl 2.0

MyPage {
    id: root
    
    property alias text: urlsEdit.text
    property string requestMethod: "GET"
    property variant requestHeaders: ({})
    property string postData
    
    title: qsTr("Add URLs")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: urlsEdit.text != ""
            onClicked: {
                var urls = urlsEdit.text.split("\n");
                
                if (settings.usePlugins) {
                    urlCheckModel.append(urls);
                    appWindow.pageStack.replace(Qt.resolvedUrl("UrlCheckPage.qml"));
                }
                else {
                    transferModel.append(urls, requestMethod, requestHeaders, postData);
                    appWindow.pageStack.pop();
                }
            }
        }
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: inputContext.visible ? height : column.height + platformStyle.paddingLarge
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: platformStyle.paddingLarge
            }
            
            TextArea {
                id: urlsEdit
                
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            ValueSelector {
                id: categorySelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                focusItem: flickable
                title: qsTr("Category")
                model: CategorySelectionModel {}
                value: settings.defaultCategory
                visible: !inputContext.visible
                onAccepted: settings.defaultCategory = value
            }
            
            MyListItem {
                id: methodButton

                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                flickableMode: true
                subItemIndicator: true
                enabled: !settings.usePlugins
                visible: !inputContext.visible
                
                MyListItemText {
                    anchors {
                        left: methodButton.paddingItem.left
                        right: methodButton.paddingItem.right
                        verticalCenter: methodButton.paddingItem.verticalCenter
                    }
                    role: "Title"
                    elide: Text.ElideRight
                    text: qsTr("Method")
                }
                
                onClicked: {
                    var page = appWindow.pageStack.push(Qt.resolvedUrl("AddUrlsMethodPage.qml"),
                    {requestMethod: root.requestMethod, postData: root.postData});
                    page.accepted.connect(function () {
                        root.requestMethod = page.requestMethod;
                        root.postData = page.postData;
                    });
                }
            }
            
            MyListItem {
                id: headersButton

                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                flickableMode: true
                subItemIndicator: true
                enabled: !settings.usePlugins
                visible: !inputContext.visible
                
                MyListItemText {
                    anchors {
                        left: headersButton.paddingItem.left
                        right: headersButton.paddingItem.right
                        verticalCenter: headersButton.paddingItem.verticalCenter
                    }
                    role: "Title"
                    elide: Text.ElideRight
                    text: qsTr("Headers")
                }
                
                onClicked: {
                    var page = appWindow.pageStack.push(Qt.resolvedUrl("AddUrlsHeadersPage.qml"),
                    {requestHeaders: root.requestHeaders});
                    page.accepted.connect(function () {
                        root.requestHeaders = page.requestHeaders;
                    });
                }
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            MySwitch {
                id: pluginCheckBox
                
                width: parent.width
                text: qsTr("Use plugins")
                checked: settings.usePlugins
                visible: !inputContext.visible
                onCheckedChanged: settings.usePlugins = checked
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
