/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2018 by Tomasz Bojczuk (seelook@gmail.com)          *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9
import QtGraphicalEffects 1.0


Item {
  id: resultTip

  property alias color: txt.color
  property alias text: txt.text

  y: GLOB.useAnimations ? -2 * height : parent.height / 50
  anchors.horizontalCenter: parent.horizontalCenter

  width: txt.width; height: txt.height
  
  Text {
    id: txt
    font { pixelSize: (executor.height / 15) * (1 - 0.2 * (lineCount - 1)); family: "Sans"; bold: true }
    visible: false
    horizontalAlignment: Text.AlignHCenter; textFormat: Text.StyledText
  }

  DropShadow {
    anchors.fill: txt
    horizontalOffset: executor.height / 200
    verticalOffset: executor.height / 200
    radius: executor.height / 100
    samples: 17
    color: activPal.shadow
    source: txt
  }

  transformOrigin: Item.Top

  Connections {
    target: executor.tipHandler
    onDestroyResultTip: resultTip.destroy()
  }

  Component.onCompleted: {
    if (GLOB.useAnimations)
      anim.running = true
  }

  SequentialAnimation {
    id: anim
    NumberAnimation { target: resultTip; property: "y"; to: executor.height / 50; duration: 200 }
    NumberAnimation { target: resultTip; property: "scale"; to: 2; duration: 200 }
    NumberAnimation { target: resultTip; property: "scale"; to: 1; duration: 300 }
  }
}