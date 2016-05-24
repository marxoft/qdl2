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

MyPage {
    id: root

    title: qsTr("Plugins")
    tools: ToolBarLayout {
        BackToolButton {}
    }

    MyListView {
        id: view

        anchors.fill: parent
        model: [qsTr("Services"), qsTr("Recaptcha"), qsTr("Decaptcha")]
        delegate: MyListItem {
            subItemIndicator: true

            MyListItemText {
                anchors {
                    left: paddingItem.left
                    right: paddingItem.right
                    verticalCenter: paddingItem.verticalCenter
                }
                role: "Title"
                elide: Text.ElideRight
                text: modelData
            }

            onClicked: {
                switch (index) {
                case 0:
                    appWindow.pageStack.push(Qt.resolvedUrl("AboutServicesPage.qml"));
                    break;
                case 1:
                    appWindow.pageStack.push(Qt.resolvedUrl("AboutRecaptchaPage.qml"));
                    break;
                case 2:
                    appWindow.pageStack.push(Qt.resolvedUrl("AboutDecaptchaPage.qml"));
                    break;
                default:
                    break;
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: view
    }
}
