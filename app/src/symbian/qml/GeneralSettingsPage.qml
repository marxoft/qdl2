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
    
    title: qsTr("General")
    tools: ToolBarLayout {
        BackToolButton {}
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
            }

            ValueSelector {
                id: orientationSelector

                width: parent.width
                title: qsTr("Screen orientation")
                model: ScreenOrientationModel {}
                value: settings.screenOrientation
                focusItem: flickable
                visible: !inputContext.visible
                onAccepted: settings.screenOrientation = value
            }
            
            ValueListItem {
                id: pathSelector
                
                width: parent.width
                title: qsTr("Download path")
                subTitle: settings.downloadPath
                visible: !inputContext.visible
                onClicked: {
                    var page = appWindow.pageStack.push(Qt.resolvedUrl("FileBrowserPage.qml"),
                    {startFolder: settings.downloadPath});
                    page.fileChosen.connect(function (f) { settings.downloadPath = f; });
                }
            }
            
            ValueSelector {
                id: concurrentSelector
                
                width: parent.width
                title: qsTr("Maximum concurrent DLs")
                model: ConcurrentTransfersModel {}
                value: settings.maximumConcurrentTransfers
                focusItem: flickable
                visible: !inputContext.visible
                onAccepted: settings.maximumConcurrentTransfers = value
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                wrapMode: Text.WordWrap
                text: qsTr("Custom command (%f for filename)")
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            MyTextField {
                id: commandEdit
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: settings.customCommand
                onTextChanged: settings.customCommand = text
                onAccepted: closeSoftwareInputPanel()
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
                visible: !inputContext.visible
            }
            
            MySwitch {
                id: commandSwitch
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Enable custom command")
                checked: settings.customCommandEnabled
                visible: !inputContext.visible
                onCheckedChanged: settings.customCommandEnabled = checked
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
                visible: !inputContext.visible
            }
            
            MySwitch {
                id: automaticSwitch
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Start DLs automatically")
                checked: settings.startTransfersAutomatically
                visible: !inputContext.visible
                onCheckedChanged: settings.startTransfersAutomatically = checked
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
                visible: !inputContext.visible
            }
            
            MySwitch {
                id: subfoldersSwitch
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Create subfolders for DLs")
                checked: settings.createSubfolders
                visible: !inputContext.visible
                onCheckedChanged: settings.createSubfolders = checked
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
