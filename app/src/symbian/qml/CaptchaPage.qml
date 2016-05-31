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
    
    title: qsTr("Enter captcha")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: transfer.submitCaptchaResponse("")
        }

        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            enabled: responseEdit.text != ""
            onClicked: {
                transfer.submitCaptchaResponse(responseEdit.text);
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
            
            Image {
                id: image
                
                width: parent.width
                visible: !inputContext.visible
            }
            
            Label {
                width: parent.width
                text: utils.formatMSecs(timer.timeRemaining)
            }
            
            MyTextField {
                id: responseEdit
                
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onAccepted: {
                    closeSoftwareInputPanel();
                    transfer.submitCaptchaResponse(responseEdit.text);
                    appWindow.pageStack.pop();
                }
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    Timer {
        id: timer

        property int timeRemaining: CAPTCHA_TIMEOUT
        
        interval: 1000
        repeat: true
        onTriggered: {
            timeRemaining -= interval;
            
            if (timeRemaining <= 0) {
                transfer.submitCaptchaResponse("");
                appWindow.pageStack.pop();
            }
        }
    }

    Connections {
        target: transfer
        onFinished: appWindow.pageStack.pop()
    }

    onTransferChanged: {
        if (transfer) {
            image.source = "data:image/jpeg;base64," + transfer.captchaImage;
            timer.timeRemaining = CAPTCHA_TIMEOUT;
            timer.restart();
        }
    }
}
