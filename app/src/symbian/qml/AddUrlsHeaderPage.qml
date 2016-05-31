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
    
    property alias name: nameEdit.text
    property alias value: valueEdit.text
    
    signal accepted()
    
    title: qsTr("Add header")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: nameEdit.text != ""
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
                text: qsTr("Name")
                visible: nameEdit.visible
            }
            
            MyTextField {
                id: nameEdit
                
                width: parent.width
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoPredictiveText
                visible: (!inputContext.visible) || (activeFocus)
            }
            
            Label {
                width: parent.width
                text: qsTr("Value")
                visible: valueEdit.visible
            }
            
            MyTextField {
                id: valueEdit
                
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
