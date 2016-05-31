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

MyPage {
    id: root
    
    property alias requestMethod: methodEdit.text
    property alias postData: postEdit.text
    
    signal accepted()
    
    title: qsTr("Method")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: methodEdit.text != ""
            onClicked: {
                root.accepted();
                appWindow.pageStack.pop();
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
            
            Label {
                width: parent.width
                text: qsTr("Method")
                visible: methodEdit.visible
            }
            
            MyTextField {
                id: methodEdit
                
                width: parent.width
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhUpperCaseOnly
                visible: (!inputContext.visible) || (activeFocus)
            }
            
            Label {
                width: parent.width
                text: qsTr("Post data")
                visible: postEdit.visible
            }
            
            MyTextField {
                id: postEdit
                
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                visible: (!inputContext.visible) || (activeFocus)
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
