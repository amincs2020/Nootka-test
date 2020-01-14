/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017-2019 by Tomasz Bojczuk (seelook@gmail.com)     *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

import Nootka 1.0
import "sound"


TmobileMenu {
  id: root

  property Item toolBar: null // fake, for main window information
  property alias drawer: mainDrawer

  function open() { mainDrawer.scoreMenu.open() }

  z: 250
  width: fingerPixels()
  height: fingerPixels()

  Rectangle {
    id: bg
    width: fingerPixels() / 2; height: fingerPixels(); x: fingerPixels() / 10; y:  fingerPixels() / 10;
    color: Noo.alpha(activPal.highlight, pressed ? 255 : 25)
    radius: fingerPixels() / 10
    Column {
      width: parent.width
      spacing: fingerPixels() * 0.15625
      topPadding: spacing
      Rectangle {
        width: bg.width / 4; height: width; radius: width / 2
        anchors.horizontalCenter: parent.horizontalCenter
        color: SOUND.playing ? "lime" : "black"
      }
      Rectangle {
        width: bg.width / 4; height: width; radius: width / 2
        anchors.horizontalCenter: parent.horizontalCenter
        color: score.recordMode ? "red" : "black"
      }
      Rectangle {
        width: bg.width / 4; height: width; radius: width / 2
        anchors.horizontalCenter: parent.horizontalCenter
        color: SOUND.listening ? "blue" : "black"
      }
    }
  }

  Connections {
    target: pitchDetectAct
    onTriggered: {
      SOUND.stoppedByUser = !SOUND.stoppedByUser
      if (SOUND.listening)
        SOUND.stopListen()
      else
        SOUND.startListen()
    }
  }

  Taction {
    id: tunerAct
    text: Noo.TR("TunerDialog", "Nooter - Nootka tuner").replace("-", "<br><font size=\"1\">") + "</font>"
    icon: "fork"
    onTriggered: {
      nootkaWindow.showDialog(Nootka.Tuner)
      SOUND.startListen()
    }
  }

  Drawer {
    id: mainDrawer
    property Item scoreMenu: null
    property NootkaLabel label: null
    width: Noo.fontSize() * 20
    height: nootkaWindow.height
    onVisibleChanged: {
      if (visible) {
        if (!drawerLoad.active)
          drawerLoad.active = true
        label.bgColor = Noo.randomColor()
      }
    }
    Loader {
      id: drawerLoad
      active: false
      anchors.fill: parent
      sourceComponent: Component {
        Flickable {
          anchors.fill: parent
          clip: true
          contentHeight: drawerColumn.height
          Column {
            id: drawerColumn
            width: parent.width
            spacing: fingerPixels() / 8
            NootkaLabel {
              id: nooLabel
              height: Noo.fontSize() * 7.91015625 // (logo ratio) 0.3955078125 * 20
              onClicked: {
                mainDrawer.close()
                Noo.aboutAct.trigger()
              }
              Component.onCompleted: mainDrawer.label = this
            }
            MenuButton { action: pitchDetectAct; onClicked: mainDrawer.close() }
            MenuButton { action: tunerAct; onClicked: mainDrawer.close() }
            MenuButton { action: Noo.levelAct; onClicked: mainDrawer.close() }
            MenuButton { action: Noo.examAct; onClicked: mainDrawer.close() }
            MenuButton {
              action: Taction {
                text: Noo.settingsAct.text
                icon: GLOB.isExam ? "exam-settings" : "systemsettings"
                onTriggered: Noo.settingsAct.trigger()
              }
              onClicked: mainDrawer.close()
            }
            MenuButton { action: Noo.scoreAct }
            Column { // drop-down menu with score actions
              id: scoreMenu
              function open() { visible ? state = "Invisible" : state = "Visible" }
              spacing: fingerPixels() / 8
              width: parent.width - Noo.fontSize() / 2
              x: -parent.width
              visible: false
              MenuButton { action: score.playAct; onClicked: mainDrawer.close() }
              MenuButton { action: score.showNamesAct; onClicked: mainDrawer.close() }
//               MenuButton { action: score.extraAccidsAct; onClicked: mainDrawer.close() } // Not implemented yet
              MenuButton { action: score.zoomInAct; onClicked: mainDrawer.close() }
              MenuButton { action: score.zoomOutAct; onClicked: mainDrawer.close() }
              MenuButton { action: score.openXmlAct; onClicked: mainDrawer.close() }
              MenuButton { action: score.saveXmlAct; onClicked: mainDrawer.close() }
              MenuButton { action: score.clearScoreAct; onClicked: mainDrawer.close() }
              states: [ State { name: "Visible"; when: scoreMenu.visible }, State { name: "Invisible"; when: !scoreMenu.visible } ]

              transitions: [
                Transition {
                  from: "Invisible"; to: "Visible"
                  SequentialAnimation {
                    PropertyAction { target: scoreMenu; property: "visible"; value: true }
                    NumberAnimation { target: scoreMenu; property: "x"; to: Noo.fontSize() / 2; duration: 300 }
                  }
                },
                Transition {
                  from: "Visible"; to: "Invisible"
                  SequentialAnimation {
                    NumberAnimation { target: scoreMenu; property: "x"; to: -parent.width; duration: 300 }
                    PropertyAction { target: scoreMenu; property: "visible"; value: false }
                  }
                }
              ]
              Component.onCompleted: mainDrawer.scoreMenu = this
            }
            MenuButton { onClicked: nootkaWindow.close(); action: Taction { icon: "close"; text: Noo.TR("QShortcut", "Close") } }
          }
        }
      }
    }
  }

  onClicked: mainDrawer.open()

  onFlyClicked: {
    if (currentFly)
      currentFly.taction.trigger()
  }

  Connections {
    target: SOUND
    onVolumeKeyPressed: nootkaWindow.showDialog(Nootka.Tuner)
  }

  FlyItem { taction: fly1act; index: 0 }
  FlyItem { taction: fly2act; index: 1 }
  FlyItem { taction: fly3act; index: 2 }
  FlyItem { taction: fly4act; index: 3 }
  FlyItem { taction: fly5act; index: 4 }

}
