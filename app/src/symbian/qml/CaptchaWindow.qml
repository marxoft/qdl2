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

Window {
    id: root
    
    property alias image: image.source
    property alias response: responseEdit.text
    property alias title: statusBar.title
    property int timeout
    property int timeRemaining
    
    signal accepted
    signal rejected
    
    function startTimer() {
        timeRemaining = timeout;
        timer.restart();
    }
    
    function stopTimer() {
        timer.stop();
    }
    
    MyStatusBar {
        id: statusBar

        anchors.top: parent.top
        title: qsTr("Captcha")
        width: parent.width
    }
    
    KeyNavFlickable {
        id: flickable
  
        anchors {
            left: parent.left
            right: parent.right
            top: statusBar.bottom
            bottom: toolBar.top
        }
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
            
            Pixmap {
                id: image
                
                width: parent.width
                height: Math.floor(width / 2)
                visible: !inputContext.visible
            }
            
            Label {
                width: parent.width
                text: utils.formatMSecs(timeRemaining)
            }
            
            MyTextField {
                id: responseEdit
                
                width: parent.width
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    ToolBar {
        id: toolBar

        anchors.bottom: parent.bottom
        states: State {
            name: "hide"
            when: (inputContext.softwareInputPanelVisible) || (inputContext.customSoftwareInputPanelVisible)
            PropertyChanges { target: toolBar; height: 0; opacity: 0.0 }
        }
        tools: ToolBarLayout {
            MyToolButton {
                iconSource: "toolbar-back"
                toolTip: qsTr("Exit")
                onClicked: root.rejected()
            }
            
            MyToolButton {
                iconSource: "images/yes.png"
                toolTip: qsTr("Done")
                enabled: responseEdit.text != ""
                onClicked: root.accepted()
            }
        }
    }
    
    Timer {
        id: timer
        
        interval: 1000
        repeat: true
        onTriggered: {
            timeRemaining -= interval;
            
            if (timeRemaining <= 0) {
                root.rejected();
            }
        }
    }
}
