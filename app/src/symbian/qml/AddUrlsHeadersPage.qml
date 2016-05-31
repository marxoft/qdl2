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
    
    property variant requestHeaders: ({})
    
    signal accepted()
    
    title: qsTr("Headers")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "toolbar-add"
            toolTip: qsTr("Add header")
            onClicked: {
                var page = appWindow.pageStack.push(Qt.resolvedUrl("AddUrlsHeaderPage.qml"));
                page.accepted.connect(function () { headerModel.append(page.name, page.value); });
            }
        }
        
        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            onClicked: {
                var headers = {};
                
                for (var i = 0; i < headerModel.count; i++) {
                    headers[headerModel.data(i, "name")] = headerModel.data(i, "value");
                }
                
                requestHeaders = headers;
                root.accepted();
                appWindow.pageStack.pop();
            }
        }
    }
    
    MyListView {
        id: view
        
        anchors.fill: parent
        model: SelectionModel {
            id: headerModel
        }
        delegate: ListItem {
            ListItemText {
                anchors {
                    left: paddingItem.left
                    right: paddingItem.right
                    top: paddingItem.top
                }
                role: "Title"
                elide: Text.ElideRight
                text: name
            }
            
            ListItemText {
                anchors {
                    left: paddingItem.left
                    right: paddingItem.right
                    bottom: paddingItem.bottom
                }
                role: "SubTitle"
                elide: Text.ElideRight
                text: value
            }
            
            onPressAndHold: popups.open(contextMenu)
        }
    }
    
    ScrollDecorator {
        flickableItem: view
    }
    
    PopupLoader {
        id: popups
    }
    
    Component {
        id: contextMenu
        
        MyContextMenu {
            focusItem: view
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Remove")
                    onClicked: headerModel.remove(view.currentIndex)
                }
                
                MenuItem {
                    text: qsTr("Clear")
                    onClicked: headerModel.clear()
                }
            }
        }
    }
    
    Component.onCompleted: {
        for (var header in requestHeaders) {
            headerModel.append(header, requestHeaders[header]);
        }
    }
}
