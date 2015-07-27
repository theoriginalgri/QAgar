import QtQuick 2.3
import QtQuick.Window 2.2
import agar 1.0

Window {
    id: root

    visible: true
    width: 360
    height: 360

    Component.onCompleted: {
        client.connectToHost("ws://localhost:8002")
    }

    Client {
        id: client
        nodeDelegate: Rectangle {
            parent: board
            color: node.color

            transform: Translate {
                x: -width/2
                y: -height/2
            }

            width: 100
            height: 100
            radius: width/2

            Text {
                anchors.centerIn: parent
                text: node.name
                fontSizeMode: Text.Fit
                color: "white"
                font.pixelSize: parent.radius / 2
            }
        }
    }

    Rectangle {
        id: board

        width: client.border.width
        height: client.border.height
        color: "lightgray"

        scale: Math.min(root.width, root.height) / Math.min(width, height)

        anchors.centerIn: parent

        Rectangle {
            color: "red"
            width: size
            height: size
            opacity: 0.3

            property real size: client.camera.zoom * Math.max(board.width, board.height)

            x: client.camera.x - size/2
            y: client.camera.y - size/2
        }
    }
}
