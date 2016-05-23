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
    
    property QtObject transfer
    
    title: qsTr("Transfer properties")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: if (transfer) transfer.customCommand = commandEdit.text;
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
            
            Row {
                width: parent.width
                spacing: platformStyle.paddingLarge
                visible: !inputContext.visible
                
                IconImage {
                    id: icon
                    
                    width: 64
                    height: 64
                }
                
                Label {
                    id: pluginLabel
                    
                    width: parent.width - icon.width - platformStyle.paddingLarge
                    elide: Text.ElideRight
                }
            }
            
            Label {
                id: nameLabel
                
                width: parent.width
                wrapMode: Text.Wrap
                visible: !inputContext.visible
            }

            Label {
                id: urlLabel

                width: parent.width
                wrapMode: Text.Wrap
                visible: !inputContext.visible
            }
            
            ValueSelector {
                id: prioritySelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                title: qsTr("Priority")
                model: TransferItemPriorityModel {}
                visible: !inputContext.visible
                onAccepted: if (transfer) transfer.priority = value;
            }
            
            Label {
                width: parent.width
                text: qsTr("Custom command (%f for filename)")
                visible: commandEdit.visible
            }
            
            MyTextField {
                id: commandEdit
                
                width: parent.width
            }

            Label {
                id: progressLabel

                width: parent.width
                wrapMode: Text.Wrap
                visible: !inputContext.visible
            }

            ProgressBar {
                id: progressBar

                width: parent.width
                minimumValue: 0
                maximumValue: 100
                visible: !inputContext.visible
            }

            Label {
                id: statusLabel

                width: parent.width
                wrapMode: Text.Wrap
                visible: !inputContext.visible
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }

    QtObject {
        id: internal

        function init() {
            updateCustomCommand();
            updateIcon();
            updateName();
            updatePluginName();
            updatePriority();
            updateProgress();
            updateStatus();
            updateUrl();
        }

        function updateCustomCommand() {
            commandEdit.text = transfer.customCommand;
        }

        function updateIcon() {
            icon.source = "file:///" + transfer.pluginIconPath;
        }

        function updateName() {
            nameLabel.text = transfer.fileName;
        }

        function updatePluginName() {
            pluginLabel.text = transfer.pluginName;
        }

        function updatePriority() {
            prioritySelector.value = transfer.priority;
        }

        function updateProgress() {
            progressBar.value = transfer.progress;
            progressLabel.text = transfer.progressString + " - " + transfer.speedString;
        }

        function updateStatus() {
            statusLabel.text = transfer.statusString;
        }

        function updateUrl() {
            urlLabel.text =  "<a href='" + transfer.url + "'>" + transfer.url + "</a>";
        }
    }

    Connections {
        target: transfer
        onDataChanged: {
            switch (role) {
            case TransferItem.BytesTransferredRole:
            case TransferItem.ProgressRole:
            case TransferItem.SizeRole:
            case TransferItem.SpeedRole:
                internal.updateProgress();
                break;
            case TransferItem.CustomCommandRole:
                internal.updateCustomCommand();
                break;
            case TransferItem.FileNameRole:
            case TransferItem.NameRole:
                internal.updateName();
                break;
            case TransferItem.PluginIconPathRole:
                internal.updateIcon();
                break;
            case TransferItem.PluginNameRole:
                internal.updatePluginName();
                break;
            case TransferItem.PriorityRole:
                internal.updatePriority();
                break;
            case TransferItem.StatusRole:
            case TransferItem.WaitTimeRole:
                internal.updateStatus();
                break;
            case TransferItem.UrlRole:
                internal.updateUrl();
                break;
            default:
                break;
            }
        }
    }

    onTransferChanged: if (transfer) internal.init();
}
