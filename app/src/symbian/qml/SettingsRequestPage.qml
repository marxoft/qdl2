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
    property variant response: ({})
    
    function setValue(key, value) {
        var r = response;
        r[key] = value;
        response = r;
    }
    
    title: qsTr("Enter settings")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: transfer.submitSettingsResponse(null)
        }

        MyToolButton {
            iconSource: "images/yes.png"
            toolTip: qsTr("Done")
            onClicked: {
                transfer.submitSettingsResponse(response);
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
                text: utils.formatMSecs(timer.timeRemaining)
            }
            
            Repeater {
                id: repeater
                
                Loader {
                    function initSourceComponent() {
                        switch (modelData.type) {
                        case "boolean":
                            sourceComponent = checkBox;
                            break;
                        case "group":
                            sourceComponent = group;
                            break;
                        case "integer":
                            sourceComponent = integerField;
                            break;
                        case "list":
                            sourceComponent = valueSelector;
                            break;
                        case "password":
                            sourceComponent = passwordField;
                            break;
                        case "text":
                            sourceComponent = textField;
                            break;
                        default:
                            break;
                        }

                        if (item) {
                            item.init(modelData);
                        }
                    }
                    
                    Component.onCompleted: initSourceComponent()
                }
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    Component {
        id: checkBox

        MySwitch {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key;
                text = modelData.label;
                checked = modelData.value === true;
            }

            width: column.width
            visible: !inputContext.visible
            onCheckedChanged: setValue(key, checked)
        }
    }

    Component {
        id: group

        Column {
            function init(modelData) {
                label.text = modelData.label;
                repeater.key = modelData.key;
                repeater.model = modelData.settings;
            }

            width: column.width
            spacing: platformStyle.paddingLarge

            SeparatorLabel {
                id: label

                width: parent.width
                visible: !inputContext.visible
            }

            Repeater {
                id: repeater

                property string key

                Loader {
                    function initSourceComponent() {
                        switch (modelData.type) {
                        case "boolean":
                            sourceComponent = checkBox;
                            break;
                        case "integer":
                            sourceComponent = integerField;
                            break;
                        case "list":
                            sourceComponent = valueSelector;
                            break;
                        case "password":
                            sourceComponent = passwordField;
                            break;
                        case "text":
                            sourceComponent = textField;
                            break;
                        default:
                            break;
                        }

                        if (item) {
                            item.init(modelData, repeater.key);
                        }
                    }

                    Component.onCompleted: initSourceComponent()
                }
            }
        }
    }

    Component {
        id: integerField

        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.validator.bottom = Math.max(0, parseInt(modelData.minimum));
                field.validator.top = Math.max(1, parseInt(modelData.maximum));
                field.text = modelData.value.toString();
            }

            width: column.width
            spacing: platformStyle.paddingLarge

            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                validator: IntValidator {}
                inputMethodHints: Qt.ImhDigitsOnly
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }

    Component {
        id: passwordField

        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = modelData.value.toString();
            }

            width: column.width
            spacing: platformStyle.paddingLarge

            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                echoMode: TextInput.Password
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }

    Component {
        id: textField

        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = modelData.value.toString();
            }

            width: column.width
            spacing: platformStyle.paddingLarge

            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }

    Component {
        id: valueSelector

        ValueSelector {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key
                title = modelData.label;

                for (var i = 0; i < modelData.options.length; i++) {
                    var option = modelData.options[i];
                    model.append(option.label, option.value);
                }

                value = modelData.value;
            }

            x: -platformStyle.paddingLarge
            width: column.width + platformStyle.paddingLarge * 2
            model: SelectionModel {}
            visible: !inputContext.visible
            onValueChanged: setValue(key, value)
        }
    }
    
    Timer {
        id: timer

        property int timeRemaining: CAPTCHA_TIMEOUT
        
        interval: 1000
        repeat: true
        onTriggered: {
            timeRemaining -= interval;
            
            if (timeRemaining <= 0) {
                transfer.submitSettingsResponse(null);
            }
        }
    }

    Connections {
        target: transfer
        onFinished: appWindow.pageStack.pop()
    }

    onTransferChanged: {
        if (transfer) {
            repeater.model = transfer.requestedSettings;
            timer.timeRemaining = CAPTCHA_TIMEOUT;
            timer.restart();
        }
    }
}
