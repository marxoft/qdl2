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
    
    title: qsTr("Retrieve URLs")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: urlRetrievalModel.clear()
        }
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: urlRetrievalModel.status == UrlRetrievalModel.Completed
            onClicked: {
                var urls = urlRetrievalModel.urls;
                
                if (urls.length > 0) {
                    appWindow.pageStack.replace(Qt.resolvedUrl("AddUrlsDialog.qml"),
                    {text: urls.join("\n") + "\n"});
                }
                else {
                    appWindow.pageStack.pop();
                }
                
                urlRetrievalModel.clear();
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
            spacing: platformStyle.paddingLarge
            
            TextArea {
                id: urlsEdit
                
                width: parent.width
            }
            
            ValueSelector {
                id: serviceSelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                title: qsTr("Service")
                focusItem: flickable
                model: ServiceSelectionModel {}
                value: settings.defaultServicePlugin
                visible: !inputContext.visible
                onAccepted: settings.defaultServicePlugin = value
            }
            
            MyButton {
                id: addButton
                
                x: Math.floor((parent.width - width) / 2)
                text: qsTr("Add")
                enabled: urlsEdit.text != ""
                visible: !inputContext.visible
                onClicked: {
                    urlRetrievalModel.append(urlsEdit.text.split(/\s+/));
                    urlsEdit.text = "";
                }
            }
            
            ProgressBar {
                id: progressBar
                
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: urlRetrievalModel.progress
                visible: !inputContext.visible
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
