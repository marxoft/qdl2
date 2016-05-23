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
    
    property variant response: ({})
    property alias settings: repeater.model
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
    
    function setValue(key, value) {
        var r = response;
        r[key] = value;
        response = r;
    }
    
    MyStatusBar {
        id: statusBar

        anchors.top: parent.top
        title: qsTr("Settings")
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

            Label {
                width: parent.width
                text: utils.formatMSecs(timeRemaining)
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
                onClicked: root.accepted()
            }
        }
    }
    
    Component {
        id: checkBox

        MyCheckBox {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key;
                text = modelData.label;
                checked = modelData.value === true;
            }

            width: column.width
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
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                validator: IntValidator {}
                onTextChanged: setValue(key, text)
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
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                echoMode: TextInput.Password
                onTextChanged: setValue(key, text)
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
            }

            MyTextField {
                id: field

                property string key

                width: parent.width
                onTextChanged: setValue(key, text)
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
            onAccepted: setValue(key, value)
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
