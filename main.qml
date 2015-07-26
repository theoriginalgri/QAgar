import QtQuick 2.3
import QtQuick.Window 2.2
import agar 1.0

Window {
    id: root

    visible: true
    width: 360
    height: 360

    Component.onCompleted: {
        client.connectToHost("ws://localhost:443")
    }

    Component {
        id: nodeComponent

        Rectangle {
            parent: board
            color: "red"

            property string name

            transform: Translate {
                x: -width/2
                y: -height/2
            }

            width: 100
            height: 100
            radius: width/2

            Text {
                anchors.centerIn: parent
                text: parent.name
                fontSizeMode: Text.Fit
                color: "white"
                font.pixelSize: parent.radius / 2
            }
        }
    }

    Client {
        id: client
        nodeDelegate: nodeComponent
    }

    Rectangle {
        id: board

        width: client.border.width
        height: client.border.height
        color: "lightgray"

        scale: Math.min(root.width, root.height) / Math.min(width, height)

        anchors.centerIn: parent
    }
}
