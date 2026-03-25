import QtQuick

Item {
    UserConfig {
        id: userConfig
    }

    property int workspaceId: 1
    property string workspaceIcon: userConfig.defaultWorkspaceIcon
    property string displayIcon: workspaceIcon
    property string displayText: "Workspace " + workspaceId
    property string iconFontFamily: "JetBrainsMono Nerd Font"
    property string textFontFamily: "Inter"
    property bool showCondition: false
    property real contentOffsetX: 0
    property int textPixelSize: 16
    readonly property bool showIcon: displayIcon !== ""

    anchors.fill: parent
    opacity: showCondition ? 1 : 0

    Behavior on opacity {
        NumberAnimation {
            duration: showCondition ? 300 : 100
            easing.type: Easing.InOutQuad
        }
    }

    Item {
        width: parent.width
        height: parent.height
        x: contentOffsetX
        clip: true

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: showIcon ? 14 : 0

            Item {
                visible: showIcon
                width: iconText.implicitWidth
                height: 24

                Text {
                    id: iconText
                    anchors.centerIn: parent
                    text: displayIcon
                    font.pixelSize: 19
                    font.family: iconFontFamily
                    color: "white"
                }
            }

            Item {
                width: labelText.implicitWidth
                height: 24

                Text {
                    id: labelText
                    anchors.centerIn: parent
                    text: displayText
                    color: "white"
                    font.pixelSize: textPixelSize
                    font.family: textFontFamily
                    font.weight: Font.DemiBold
                    font.letterSpacing: -0.15
                    wrapMode: Text.NoWrap
                }
            }
        }
    }
}
