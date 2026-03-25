import QtQuick

QtObject {
    id: userConfig
    property string defaultWorkspaceIcon: ""

    property var scriptPaths: ({
        wifiMenu: "~/.config/quickshell/wifi-menu.sh",
        bluetoothMenu: "~/.config/quickshell/bluetooth-menu.sh",
        wallpaperSwitcher: "~/.config/quickshell/wallpaper-switch.sh",
        powerMenu: "~/.config/quickshell/powermenu"
    })

    property var controlCenterActions: ([
        { icon: "", command: scriptPaths.wifiMenu },
        { icon: "", command: scriptPaths.bluetoothMenu },
        { icon: "󰋩", command: scriptPaths.wallpaperSwitcher },
        { icon: "󰣇", command: scriptPaths.powerMenu }
    ])

    property var controlCenterIcons: ({
        "charging": "",
        "brightness": "󰃟",
        "volume": "󰕾"
    })

    property var workspaceIcons: ({
        "1": "",
        "2": "",
        "3": "",
        "4": "",
        "5": "",
        "6": "󰙯",
        "7": "󰈙",
        "8": "󰇮",
        "9": "󰊴",
        "10": "",
        "urgent": "",
        "default": defaultWorkspaceIcon
    })

    property var statusIcons: ({
        "default": "🎧",
        "volume": "󰕾",
        "mute": "󰝟",
        "brightnessLow": "󰃞",
        "brightnessMedium": "󰃟",
        "brightnessHigh": "󰃠",
        "charging": "",
        "discharging": "",
        "capsLockOn": "",
        "capsLockOff": "",
        "bluetooth": "󰋋"
    })

    function workspaceIcon(wsId) {
        const key = String(wsId);
        return workspaceIcons[key] || workspaceIcons["default"];
    }
}
