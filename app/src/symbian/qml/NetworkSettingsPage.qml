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
    
    title: qsTr("Network")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: settings.setNetworkProxy()
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
            }
            spacing: platformStyle.paddingLarge
            
            MySwitch {
                id: proxySwitch
                
                width: parent.width
                text: qsTr("Use network proxy")
                checked: settings.networkProxyEnabled
                visible: !inputContext.visible
                onCheckedChanged: settings.networkProxyEnabled = checked
            }
            
            ValueSelector {
                id: typeSelector
                
                width: parent.width
                focusItem: flickable
                title: qsTr("Type")
                model: NetworkProxyTypeModel {}
                value: settings.networkProxyType
                visible: !inputContext.visible
                onValueChanged: settings.networkProxyType = value
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                elide: Text.ElideRight
                text: qsTr("Host")
                visible: hostEdit.visible
            }
            
            MyTextField {
                id: hostEdit
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: settings.networkProxyHost
                visible: (!inputContext.visible) || (focus)
                onTextChanged: settings.networkProxyHost = text
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                elide: Text.ElideRight
                text: qsTr("Port")
                visible: portEdit.visible
            }
            
            MyTextField {
                id: portEdit
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: settings.networkProxyPort
                visible: (!inputContext.visible) || (focus)
                validator: IntValidator {
                    bottom: 0
                    top: 100000
                }
                onTextChanged: settings.networkProxyPort = parseInt(text)
            }
            
            MySwitch {
                id: authenticationSwitch
                
                width: parent.width
                text: qsTr("Use authentication")
                checked: settings.networkProxyAuthenticationEnabled
                visible: !inputContext.visible
                onCheckedChanged: settings.networkProxyAuthenticationEnabled = checked
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                elide: Text.ElideRight
                text: qsTr("Username")
                visible: usernameEdit.visible
            }
            
            MyTextField {
                id: usernameEdit
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: settings.networkProxyUsername
                visible: (!inputContext.visible) || (focus)
                onTextChanged: settings.networkProxyUsername = text
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                elide: Text.ElideRight
                text: qsTr("Password")
                visible: passwordEdit.visible
            }
            
            MyTextField {
                id: passwordEdit
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                echoMode: TextInput.Password
                text: settings.networkProxyPassword
                visible: (!inputContext.visible) || (focus)
                onTextChanged: settings.networkProxyPassword = text
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
