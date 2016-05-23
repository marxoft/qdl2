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
    
    title: qsTr("Add URLs")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: urlsEdit.text != ""
            onClicked: {
                var urls = urlsEdit.text.split(/\s+/);
                urlCheckModel.append(urls);
                appWindow.pageStack.replace(Qt.resolvedUrl("UrlCheckPage.qml"));
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
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
