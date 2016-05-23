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
    
    title: transferModel.activeTransfers + "DLs - " + transferModel.totalSpeedString
    tools: ToolBarLayout {
        MyToolButton {
            iconSource: "toolbar-add"
            toolTip: qsTr("Add URLs")
            onClicked: appWindow.pageStack.push(Qt.resolvedUrl("AddUrlsPage.qml"))
        }
        
        MyToolButton {
            iconSource: "toolbar-search"
            toolTip: qsTr("Retrieve URLs")
            onClicked: appWindow.pageStack.push(Qt.resolvedUrl("RetrieveUrlsPage.qml"))
        }
        
        MyToolButton {
            iconSource: "toolbar-view-menu"
            toolTip: qsTr("Options")
            onClicked: popups.open(mainMenu, root)
        }
    }
    
    MyListView {
        id: transferView
        
        property variant expandedIndexes: []
        
        function toggleExpanded(index) {
            var e = expandedIndexes;
            var i = e.indexOf(index);
            
            if (i == -1) {
                e.push(index);
            }
            else {
                e.splice(i, 1);
            }
            
            expandedIndexes = e;
        }
        
        anchors.fill: parent
        model: transferModel
        delegate: TransferDelegate {}
    }
    
    ScrollDecorator {
        flickableItem: transferView
    }

    Label {
        id: label

        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        color: platformStyle.colorNormalMid
        font.bold: true
        font.pixelSize: 32
        text: qsTr("No DLs")
        visible: transferView.count == 0
    }
    
    PopupLoader {
        id: popups
    }
    
    Component {
        id: concurrentTransfersDialog
        
        ValueDialog {
            focusItem: transferView
            titleText: qsTr("Maximum concurrent DLs")
            model: ConcurrentTransfersModel {}
            value: settings.maximumConcurrentTransfers
            onAccepted: settings.maximumConcurrentTransfers = value
        }
    }
    
    Component {
        id: nextActionDialog
        
        ValueDialog {
            focusItem: transferView
            titleText: qsTr("After current DLs")
            model: ActionModel {}
            value: settings.nextAction
            onAccepted: settings.nextAction = value
        }
    }
    
    Component {
        id: confirmExitDialog
        
        MyQueryDialog {
            focusItem: transferView
            titleText: qsTr("Quit?")
            message: qsTr("Some download(s) are stil active. Do you want to quit?");
            onAccepted: qdl.quit()
        }
    }
    
    Component {
        id: mainMenu
        
        MyMenu {
            focusItem: transferView
            
            MenuLayout {
                ValueMenuItem {                    
                    title: qsTr("Maximum concurrent DLs")
                    subTitle: settings.maximumConcurrentTransfers
                    onClicked: popups.open(concurrentTransfersDialog, root)
                }
                
                ValueMenuItem {                    
                    title: qsTr("After current DLs")
                    subTitle: settings.nextAction == 0 ? qsTr("Continue") : settings.nextAction == 1 ? qsTr("Stop") : qsTr("Quit")
                    onClicked: popups.open(nextActionDialog, root)
                }
                
                MenuItem {
                    text: qsTr("Settings")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
                }
                
                MenuItem {
                    text: qsTr("Load plugins")
                    onClicked: {
                        var count = decaptchaPluginManager.load() + recaptchaPluginManager.load()
                        + servicePluginManager.load();
                        infoBanner.information(count > 0 ? count + " " + qsTr("new plugins found")
                        : qsTr("No new plugins found"))
                    }
                }       
                
                MenuItem {
                    text: qsTr("About")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
                
                MenuItem {
                    text: qsTr("Exit")
                    onClicked: transferModel.activeTransfers > 0 ? popups.open(confirmExitDialog, root) : qdl.quit()
                }
            }
        }
    }    
}
