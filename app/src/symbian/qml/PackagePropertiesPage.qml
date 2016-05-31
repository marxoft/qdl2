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
    
    property QtObject packageItem
    
    title: qsTr("Package properties")
    tools: ToolBarLayout {
        BackToolButton {}
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height + platformStyle.paddingLarge
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: platformStyle.paddingLarge
            }
            
            Label {
                id: nameLabel
                
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            MySwitch {
                id: subfolderCheckBox
                
                width: parent.width
                text: qsTr("Create subfolder")
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            ValueSelector {
                id: categorySelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                title: qsTr("Category")
                model: CategorySelectionModel {}
                onAccepted: if (packageItem) packageItem.category = value;
            }
            
            ValueSelector {
                id: prioritySelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                title: qsTr("Priority")
                model: TransferItemPriorityModel {}
                onAccepted: if (packageItem) packageItem.priority = value;
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }

            Label {
                id: progressLabel

                width: parent.width
                wrapMode: Text.Wrap
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }

            Label {
                id: statusLabel

                width: parent.width
                wrapMode: Text.Wrap
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }

    QtObject {
        id: internal

        function init() {
            updateCategory();
            updateCreateSubfolder();
            updateName();
            updatePriority();
            updateProgress();
            updateStatus();
        }

        function updateCategory() {
            categorySelector.value = packageItem.category;
        }

        function updateCreateSubfolder() {
            subfolderCheckBox.checked = packageItem.createSubfolder;
        }

        function updateName() {
            nameLabel.text = packageItem.name;
        }

        function updatePriority() {
            prioritySelector.value = packageItem.priority;
        }

        function updateProgress() {
            progressLabel.text = packageItem.progressString;
        }

        function updateStatus() {
            statusLabel.text = packageItem.statusString;
        }
    }

    Connections {
        target: packageItem
        onDataChanged: {
            switch (role) {
            case TransferItem.CategoryRole:
                internal.updateCategory();
                break;
            case TransferItem.CreateSubfolderRole:
                internal.updateCreateSubfolder();
                break;
            case TransferItem.NameRole:
                internal.updateName();
                break;
            case TransferItem.PriorityRole:
                internal.updatePriority();
                break;
            case TransferItem.ProgressRole:
            case TransferItem.RowCountRole:
                internal.updateProgress();
                break;
            case TransferItem.StatusRole:
                internal.updateStatus();
                break;
            default:
                break;
            }
        }
    }

    onPackageItemChanged: if (packageItem) internal.init();
}
