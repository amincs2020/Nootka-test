/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017-2020 by Tomasz Bojczuk (seelook@gmail.com)     *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9

import Nootka 1.0


TpianoBg {
  id: instrItem

  anchors.fill: parent
  keyWidth: height / (Noo.isAndroid() ? 5.5 : 5)
  onWantKeyToSelect: selectKey(k > -1 ? (isWhite ? whiteRep.itemAt(k) : whiteRep.itemAt(k).black) : null)

  function getKey(keyNr) { return whiteRep.itemAt(keyNr) }

//   private
  property var activeKey: null
  property var correctAnim: null
  property var pianoZoom: null

  onCorrectInstrument: {
    if (!correctAnim) {
      var c = Qt.createComponent("qrc:/exam/CorrectInstrAnim.qml")
      correctAnim = c.createObject(instrItem)
    }
    correctAnim.doCross = !wrongItem.parent
    correctAnim.start()
  }

  Image { // piano background
    cache: false
    source: Noo.pix("pianoBg")
    width: score.width; height: width * (sourceSize.height / sourceSize.width)
    y: -height
    z: -1
  }

  Row {
    Rectangle {
      width: (instrItem.width - keysNumber * Math.floor(keyWidth)) / 2
      height: instrItem.height
      color: "black"
    }
    Repeater {
      id: whiteRep
      model: keysNumber
      PianoKeyWhite {
        nr: index
        onEntered: activeKey = key
        onClicked: selectedKey = key
      }
    }
    Rectangle {
      width: (instrItem.width - keysNumber * Math.floor(keyWidth)) / 2
      height: instrItem.height
      color: "black"
    }
  }

  keyHighlight: Rectangle { // piano key highlight
    parent: selectedKey
    anchors.fill: parent ? parent : undefined
    color: GLOB.selectedColor
    border { width: Math.round(keyWidth / 16); color: "black" }
    radius: width / 5
    z: 2
  }

  Rectangle { // piano key cursor
    anchors.fill: parent ? parent : undefined
    parent: activeKey
    z: 2
    color: GLOB.fingerColor
    border { width: Math.round(keyWidth / 16); color: "black" }
    radius: width / 5
    visible: active
  }

  Rectangle {
    id: octaveCover
    width: instrItem.width; height: Noo.fontSize() * (Noo.isAndroid() ? 1 : 1.5)
    color: "black"
    Repeater {
      model: Math.floor(keysNumber / 7)
      Rectangle {
        x: margin + index * width
        width: keyWidth * 7; height: parent.height
        color: index % 2 ? "#303030" : "black"
        Text {
          anchors.centerIn: parent
          text: octaveName(firstOctave + index) + (GLOB.scientificOctaves ? "  [%1]".arg(firstOctave + index + 3) : "")
          font { pixelSize: parent.height * 0.8 }
          color: "white"
        }
      }
    }
  }

  Component.onCompleted: {
    if (Noo.isAndroid() && Noo.fingerPixels() * 4 > height * 1.1) {
      var pz = Qt.createComponent("qrc:/instruments/PianoZoom.qml")
      pianoZoom = pz.createObject(nootkaWindow.contentItem.parent)
    }
  }

  Component.onDestruction: {
    if (pianoZoom)
      pianoZoom.destroy() // it belongs to another parent
  }
}
