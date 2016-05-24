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

Item {
    id: root
    
    property bool expanded: transferView.expandedIndexes.indexOf(index) != -1
        
    width: ListView.view ? ListView.view.width : screen.width
    height: packageItem.height + (loader.item ? loader.item.height : 0)
    
    Item {
        id: packageItem
        
        property string mode: "normal"
                
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 64 + platformStyle.paddingLarge * 4
        focus: true
        
        Rectangle {
            height: 1
            color: platformStyle.colorDisabledMid
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
        }
        
        Loader {
            id: faderLoader
            
            opacity: 0
            anchors.fill: parent
            sourceComponent: packageItem.mode != "normal" && packageItem.mode != "pressed" ? fader : undefined
        }

        BorderImage {
            id: highlight
            
            border {
                left: platformStyle.borderSizeMedium
                top: platformStyle.borderSizeMedium
                right: platformStyle.borderSizeMedium
                bottom: platformStyle.borderSizeMedium
            }
            opacity: 0
            anchors.fill: parent
        }
        
        Image {
            id: indicator
            
            anchors {
                left: parent.left
                leftMargin: platformStyle.paddingLarge
                verticalCenter: parent.verticalCenter
            }
            sourceSize.width: platformStyle.graphicSizeSmall
            sourceSize.height: platformStyle.graphicSizeSmall
            smooth: true
            source: expanded ? "images/down.png" : "images/right.png"
        }
        
        MyListItemText {
            anchors {
                left: indicator.right
                right: parent.right
                top: parent.top
                margins: platformStyle.paddingLarge
            }
            role: "Title"
            elide: Text.ElideRight
            text: name
        }
        
        MyListItemText {
            anchors {
                left: indicator.right
                right: parent.right
                bottom: parent.bottom
                margins: platformStyle.paddingLarge
            }
            role: "SubTitle"
            elide: Text.ElideRight
            text: status == TransferItem.Null ? progressString : statusString
        }
        
        MouseArea {
            id: mouseArea
            
            z: 1
            anchors.fill: parent
            enabled: root.enabled
            onPressed: {
                symbian.listInteractionMode = Symbian.TouchInteraction;
                internal.state = "Pressed";
            }
            onClicked: {
                internal.state = "";
                transferView.toggleExpanded(index);
            }
            onCanceled: {
                internal.state = "Canceled";
            }
            onPressAndHold: {
                internal.state = "PressAndHold";
                popups.open(packageMenu, root);
            }
            onReleased: {
                internal.state = "";
            }
            onExited: {
                internal.state = "";
            }
        }
        
        Keys.onReleased: {
            if (!event.isAutoRepeat && root.enabled) {
                if (event.key == Qt.Key_Select || event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                    event.accepted = true;
                    internal.state = "Focused";
                }
            }
        }

        Keys.onPressed: {
            if (!event.isAutoRepeat) {
                switch (event.key) {
                    case Qt.Key_Select:
                    case Qt.Key_Enter:
                    case Qt.Key_Return: {
                        if (symbian.listInteractionMode != Symbian.KeyNavigation)
                        symbian.listInteractionMode = Symbian.KeyNavigation;
                        else
                        if (root.enabled) {
                            highlight.source = privateStyle.imagePath("qtg_fr_list_pressed", false);
                            highlight.opacity = 1;
                            releasedEffect.restart();
                            mouseArea.clicked();
                        }
                        event.accepted = true;
                        break;
                    }

                    case Qt.Key_Space: {
                        if (symbian.listInteractionMode != Symbian.KeyNavigation)
                        symbian.listInteractionMode = Symbian.KeyNavigation;
                        else
                        event.accepted = true;
                        break;
                    }

                    case Qt.Key_Up: {
                        if (symbian.listInteractionMode != Symbian.KeyNavigation) {
                            symbian.listInteractionMode = Symbian.KeyNavigation;
                            internal.state = "Focused";
                            root.ListView.view.positionViewAtIndex(index, ListView.Beginning);
                        } else {
                            root.ListView.view.decrementCurrentIndex();
                            event.accepted = true;
                        }
                        break;
                    }

                    case Qt.Key_Down: {
                        if (symbian.listInteractionMode != Symbian.KeyNavigation) {
                            symbian.listInteractionMode = Symbian.KeyNavigation;
                            internal.state = "Focused";
                            root.ListView.view.positionViewAtIndex(index, ListView.Beginning);
                        } else {
                            root.ListView.view.incrementCurrentIndex();
                            event.accepted = true;
                        }
                        break;
                    }

                    default: {
                        event.accepted = false;
                        break;
                    }
                }
            }

            if ((event.key == Qt.Key_Up || event.key == Qt.Key_Down))
            symbian.privateListItemKeyNavigation(root.ListView.view)
        }

        SequentialAnimation {
            id: releasedEffect
            PropertyAnimation {
                target: highlight
                property: "opacity"
                to: 0
                easing.type: Easing.Linear
                duration: 150
            }
        }

        StateGroup {
            id: internal

            function getMode() {
                if (internal.state == "Pressed" || internal.state == "PressAndHold")
                return "pressed"
                else if (internal.state == "Focused")
                return "highlighted"
                else if (internal.state == "Disabled")
                return "disabled"
                else
                return "normal"
            }

            // Performance optimization:
            // Use value assignment when property changes instead of binding to js function
            onStateChanged: { packageItem.mode = internal.getMode() }

            function press() {
                privateStyle.play(Symbian.BasicItem);
                highlight.source = privateStyle.imagePath("qtg_fr_list_pressed", false);
                highlight.opacity = 1;
                if (root.ListView.view)
                root.ListView.view.currentIndex = index;
            }

            function release() {
                if (symbian.listInteractionMode != Symbian.KeyNavigation)
                privateStyle.play(Symbian.BasicItem);
                releasedEffect.restart();
            }

            function releaseHold() {
                releasedEffect.restart();
            }

            function disable() {
                faderLoader.opacity = 1;
            }

            function focus() {
                faderLoader.opacity = 1;
            }

            function canceled() {
                releasedEffect.restart();
            }

            states: [
            State { name: "Pressed" },
            State { name: "PressAndHold" },
            State { name: "Disabled"; when: !root.enabled },
            State { name: "Focused"; when: ((root.ListView.isCurrentItem || (packageItem.activeFocus)) &&
                symbian.listInteractionMode == Symbian.KeyNavigation) },
            State { name: "Canceled" },
            State { name: "" }
            ]

            transitions: [
            Transition {
                to: "Pressed"
                ScriptAction { script: internal.press() }
            },
            Transition {
                from: "PressAndHold"
                to: ""
                ScriptAction { script: internal.releaseHold() }
            },
            Transition {
                to: ""
                ScriptAction { script: internal.release() }
            },
            Transition {
                to: "Disabled"
                ScriptAction { script: internal.disable() }
            },
            Transition {
                to: "Focused"
                ScriptAction { script: internal.focus() }
            },
            Transition {
                to: "Canceled"
                ScriptAction { script: internal.canceled() }
            }
            ]
        }
    }
    
    Loader {
        id: loader
        
        anchors {
            left: parent.left
            right: parent.right
            top: packageItem.bottom
        }
        sourceComponent: (expanded) && (count > 0) ? transfers : undefined
    }
    
    Component {
        id: transfers
        
        Column {
            id: column
            
            anchors {
                left: parent ? parent.left : undefined
                right: parent ? parent.right : undefined
            }
            
            Repeater {
                id: repeater
                
                property int currentIndex
                
                function decrementCurrentIndex() {
                    currentIndex = Math.max(0, currentIndex - 1);
                }
                
                function incrementCurrentIndex() {
                    currentIndex = Math.min(count - 1, currentIndex + 1);
                }
                
                model: VisualDataModel {
                    id: dataModel
                    
                    model: transferModel
                    rootIndex: transferModel.modelIndex(index, 0)
                    delegate: Item {
                        id: transferItem
                        
                        property string mode: "normal"
                        property bool isCurrentItem: repeater.currentIndex == index
                        
                        width: column.width
                        height: 64 + platformStyle.paddingLarge * 4
                        focus: true
                        
                        Rectangle {
                            height: 1
                            color: platformStyle.colorDisabledMid
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                right: parent.right
                            }
                        }
                        
                        Loader {
                            id: faderLoader
                            
                            opacity: 0
                            anchors.fill: parent
                            sourceComponent: transferItem.mode != "normal" && transferItem.mode != "pressed"
                            ? fader : undefined
                        }

                        BorderImage {
                            id: highlight
                            
                            border {
                                left: platformStyle.borderSizeMedium
                                top: platformStyle.borderSizeMedium
                                right: platformStyle.borderSizeMedium
                                bottom: platformStyle.borderSizeMedium
                            }
                            opacity: 0
                            anchors.fill: parent
                        }
                        
                        IconImage {
                            id: icon
                            
                            anchors {
                                left: parent.left
                                leftMargin: platformStyle.paddingLarge
                                verticalCenter: parent.verticalCenter
                            }
                            width: 64
                            height: 64
                            source: "file:///" + pluginIconPath
                        }
                        
                        MyListItemText {
                            anchors {
                                left: icon.right
                                right: parent.right
                                top: parent.top
                                margins: platformStyle.paddingLarge
                            }
                            role: "Title"
                            elide: Text.ElideRight
                            text: name
                        }
                        
                        MyListItemText {
                            anchors {
                                left: icon.right
                                right: parent.right
                                bottom: progressBar.top
                                margins: platformStyle.paddingLarge
                            }
                            role: "SubTitle"
                            elide: Text.ElideRight
                            text: status == TransferItem.Downloading ? progressString + " - " + speedString : statusString
                        }
                        
                        ProgressBar {
                            id: progressBar
                            
                            anchors {
                                left: icon.right
                                right: parent.right
                                bottom: parent.bottom
                                margins: platformStyle.paddingLarge
                            }
                            minimumValue: 0
                            maximumValue: 100
                            value: progress
                        }
                        
                        MouseArea {
                            id: mouseArea
                            
                            z: 1
                            anchors.fill: parent
                            enabled: root.enabled
                            onPressed: {
                                symbian.listInteractionMode = Symbian.TouchInteraction;
                                internal.state = "Pressed";
                            }
                            onClicked: {
                                internal.state = "";
                            }
                            onCanceled: {
                                internal.state = "Canceled";
                            }
                            onPressAndHold: {
                                internal.state = "PressAndHold";
                                popups.open(transferMenu, mainPage);
                            }
                            onReleased: {
                                internal.state = "";
                            }
                            onExited: {
                                internal.state = "";
                            }
                        }
                        
                        Keys.onReleased: {
                            if (!event.isAutoRepeat && root.enabled) {
                                if (event.key == Qt.Key_Select || event.key == Qt.Key_Return
                                || event.key == Qt.Key_Enter) {
                                    event.accepted = true;
                                    internal.state = "Focused";
                                }
                            }
                        }

                        Keys.onPressed: {
                            if (!event.isAutoRepeat) {
                                switch (event.key) {
                                    case Qt.Key_Select:
                                    case Qt.Key_Enter:
                                    case Qt.Key_Return: {
                                        if (symbian.listInteractionMode != Symbian.KeyNavigation)
                                        symbian.listInteractionMode = Symbian.KeyNavigation;
                                        else
                                        if (root.enabled) {
                                            highlight.source = privateStyle.imagePath("qtg_fr_list_pressed", false);
                                            highlight.opacity = 1;
                                            releasedEffect.restart();
                                            mouseArea.clicked();
                                        }
                                        event.accepted = true;
                                        break;
                                    }

                                    case Qt.Key_Space: {
                                        if (symbian.listInteractionMode != Symbian.KeyNavigation)
                                        symbian.listInteractionMode = Symbian.KeyNavigation;
                                        else
                                        event.accepted = true;
                                        break;
                                    }

                                    case Qt.Key_Up: {
                                        if (symbian.listInteractionMode != Symbian.KeyNavigation) {
                                            symbian.listInteractionMode = Symbian.KeyNavigation;
                                            internal.state = "Focused";
                                        } else {
                                            if (!repeater.decrementCurrentIndex()) {
                                                packageItem.forceActiveFocus();
                                            }
                                            
                                            event.accepted = true;
                                        }
                                        break;
                                    }

                                    case Qt.Key_Down: {
                                        if (symbian.listInteractionMode != Symbian.KeyNavigation) {
                                            symbian.listInteractionMode = Symbian.KeyNavigation;
                                            internal.state = "Focused";
                                        } else {
                                            if (!repeater.incrementCurrentIndex()) {
                                                root.ListView.view.incrementCurrentIndex();
                                            }
                                            
                                            event.accepted = true;
                                        }
                                        break;
                                    }

                                    default: {
                                        event.accepted = false;
                                        break;
                                    }
                                }
                            }
                        }

                        SequentialAnimation {
                            id: releasedEffect
                            PropertyAnimation {
                                target: highlight
                                property: "opacity"
                                to: 0
                                easing.type: Easing.Linear
                                duration: 150
                            }
                        }

                        StateGroup {
                            id: internal

                            function getMode() {
                                if (internal.state == "Pressed" || internal.state == "PressAndHold")
                                return "pressed"
                                else if (internal.state == "Focused")
                                return "highlighted"
                                else if (internal.state == "Disabled")
                                return "disabled"
                                else
                                return "normal"
                            }

                            // Performance optimization:
                            // Use value assignment when property changes instead of binding to js function
                            onStateChanged: { transferItem.mode = internal.getMode() }

                            function press() {
                                privateStyle.play(Symbian.BasicItem);
                                highlight.source = privateStyle.imagePath("qtg_fr_list_pressed", false);
                                highlight.opacity = 1;
                                repeater.currentIndex = index;
                            }

                            function release() {
                                if (symbian.listInteractionMode != Symbian.KeyNavigation)
                                privateStyle.play(Symbian.BasicItem);
                                releasedEffect.restart();
                            }

                            function releaseHold() {
                                releasedEffect.restart();
                            }

                            function disable() {
                                faderLoader.opacity = 1;
                            }

                            function focus() {
                                faderLoader.opacity = 1;
                            }

                            function canceled() {
                                releasedEffect.restart();
                            }

                            states: [
                            State { name: "Pressed" },
                            State { name: "PressAndHold" },
                            State { name: "Disabled"; when: !root.enabled },
                            State { name: "Focused"; when: ((transferItem.isCurrentItem ||
                                (transferItem.activeFocus))
                                && symbian.listInteractionMode == Symbian.KeyNavigation) },
                            State { name: "Canceled" },
                            State { name: "" }
                            ]

                            transitions: [
                            Transition {
                                to: "Pressed"
                                ScriptAction { script: internal.press() }
                            },
                            Transition {
                                from: "PressAndHold"
                                to: ""
                                ScriptAction { script: internal.releaseHold() }
                            },
                            Transition {
                                to: ""
                                ScriptAction { script: internal.release() }
                            },
                            Transition {
                                to: "Disabled"
                                ScriptAction { script: internal.disable() }
                            },
                            Transition {
                                to: "Focused"
                                ScriptAction { script: internal.focus() }
                            },
                            Transition {
                                to: "Canceled"
                                ScriptAction { script: internal.canceled() }
                            }
                            ]
                        }

                        Component {
                            id: transferMenu

                            MyMenu {
                                focusItem: transferView

                                MenuLayout {
                                    MenuItem {
                                        text: qsTr("Properties")
                                        onClicked: appWindow.pageStack.push(Qt.resolvedUrl("TransferPropertiesPage.qml"),
                                                                            {transfer: transferModel.get(dataModel.modelIndex(index))})
                                    }

                                    MenuItem {
                                        text: qsTr("Start")
                                        enabled: canStart
                                        onClicked: transferModel.setData(dataModel.modelIndex(index),
                                        TransferItem.Queued, "status")
                                    }

                                    MenuItem {
                                        text: qsTr("Pause")
                                        enabled: canPause
                                        onClicked: transferModel.setData(dataModel.modelIndex(index),
                                        TransferItem.Paused, "status")
                                    }

                                    MenuItem {
                                        text: qsTr("Remove")
                                        enabled: canCancel
                                        onClicked: transferModel.setData(dataModel.modelIndex(index),
                                        TransferItem.Canceled, "status")
                                    }

                                    MenuItem {
                                        text: qsTr("Remove and delete files")
                                        enabled: canCancel
                                        onClicked: transferModel.setData(dataModel.modelIndex(index),
                                        TransferItem.CanceledAndDeleted, "status")
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    Component {
        id: packageMenu
        
        MyMenu {
            focusItem: transferView
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Properties")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("PackagePropertiesPage.qml"),
                                                        {packageItem: transferModel.get(transferModel.modelIndex(index, 0))})
                }
                
                MenuItem {
                    text: qsTr("Start")
                    enabled: canStart
                    onClicked: transferModel.setData(transferModel.modelIndex(index, 0), TransferItem.Queued,
                    "status")
                }
                
                MenuItem {
                    text: qsTr("Pause")
                    enabled: canPause
                    onClicked: transferModel.setData(transferModel.modelIndex(index, 0), TransferItem.Paused,
                    "status")
                }
                
                MenuItem {
                    text: qsTr("Remove")
                    enabled: canCancel
                    onClicked: transferModel.setData(transferModel.modelIndex(index, 0), TransferItem.Canceled,
                    "status")
                }
                
                MenuItem {
                    text: qsTr("Remove and delete files")
                    enabled: canCancel
                    onClicked: transferModel.setData(transferModel.modelIndex(index, 0),
                    TransferItem.CanceledAndDeleted, "status")
                }
            }
        }
    }
    
    Component {
        id: fader

        BorderImage {
            source: privateStyle.imagePath("qtg_fr_list_" + mode, root.platformInverted)
            border {
                left: platformStyle.borderSizeMedium
                top: platformStyle.borderSizeMedium
                right: platformStyle.borderSizeMedium
                bottom: platformStyle.borderSizeMedium
            }
        }
    }
}
