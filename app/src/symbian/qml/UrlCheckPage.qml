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
    
    title: qsTr("Check URLs")
    tools: ToolBarLayout {
        BackToolButton {            
            onClicked: urlCheckModel.clear()
        }
    }
    
    ListView {
        id: view
        
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: progressBar.top
            bottomMargin: platformStyle.paddingLarge
        }
        model: urlCheckModel
        delegate: MyListItem {
            MyListItemText {
                anchors {
                    left: paddingItem.left
                    right: indicator.left
                    rightMargin: platformStyle.paddingLarge
                    verticalCenter: paddingItem.verticalCenter
                }
                role: "Title"
                elide: Text.ElideRight
                text: url
            }
            
            Image {
                id: indicator
                
                anchors {
                    right: paddingItem.right
                    verticalCenter: paddingItem.verticalCenter
                }
                sourceSize.width: platformStyle.graphicSizeSmall
                sourceSize.height: platformStyle.graphicSizeSmall
                source: checked ? ok ? "images/yes.png" : "images/no.png" : ""
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: view
    }
    
    ProgressBar {
        id: progressBar
        
        anchors {
            left: parent.left
            right: parent.right
            bottom: statusLabel.top
            margins: platformStyle.paddingLarge
        }
        minimumValue: 0
        maximumValue: 100
        value: urlCheckModel.progress
    }
    
    Label {
        id: statusLabel
        
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: platformStyle.paddingMedium
        }
        wrapMode: Text.WordWrap
        text: urlCheckModel.statusString
    }
}
