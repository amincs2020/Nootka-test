/***************************************************************************
 *   Copyright (C) 2013 by Tomasz Bojczuk                                  *
 *   tomaszbojczuk@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/


#include "tintonationview.h"
#include <QPainter>
#include <QDebug>
#include <math.h>


#define THWIDTH (2)
#define THGAP (3)
QColor tc = Qt::black;

TintonationView::TintonationView(int accuracy, QWidget* parent) :
  QWidget(parent),
  m_pitchDiff(0.0f)
{
  setAccuracy(accuracy);
  setMinimumSize(200, 17);
  tc = palette().text().color();
  resizeEvent(0);
}

TintonationView::~TintonationView()
{

}

void TintonationView::setAccuracy(int accuracy) {
  m_accuracy = (Eaccuracy)accuracy;
  switch(m_accuracy) {
    case e_perfect:
      m_accurValue = 0.1; break;
    case e_normal:
      m_accurValue = 0.2; break;
    case e_sufficient:
      m_accurValue = 0.3; break;
  }
}


void TintonationView::pitchSlot(float pitch) {
  bool doUpdate = false;
  if (m_pitchDiff != 0.0)
      doUpdate = true;
  m_pitchDiff = pitch - (float)qRound(pitch);
  if (doUpdate)
      update();
}


//################################################################################
//############################### protected ######################################
//################################################################################


void TintonationView::paintEvent(QPaintEvent* ) {
  int lastColorThick = (qAbs(m_pitchDiff) / 0.5) * m_ticksCount;
  QPainter painter(this);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QBrush(palette().window().color()));
//   painter.setBrush(QBrush(Qt::white));
  painter.drawRoundedRect(painter.viewport(), 4, 4);
  if (m_pitchDiff == 0.0)
    painter.setPen(QPen(tc));
  else
    painter.setPen(QPen(m_tickColors[lastColorThick]));
  painter.setFont(m_nooFont);
  painter.drawText(0, 0, width(), height(), Qt::AlignCenter, "n");
  for (int i = 0; i < m_ticksCount - 1; i++) {
    QColor thickColor, leftThickColor, rightThickColor;
    if (i < lastColorThick)
      thickColor = m_tickColors[i];
    else
      thickColor = tc;
    if (m_pitchDiff < 0) {
      leftThickColor = thickColor; rightThickColor = tc;
    } else {
      leftThickColor = tc; rightThickColor = thickColor;
    }
    int xx = m_noteX - ((i + 1) * (THGAP + THWIDTH));
    float yy = (float)(m_ticksCount - i) * m_hiTickStep + 1;
//     int yy = 1;
    painter.setPen(QPen(leftThickColor, THWIDTH, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QLineF(xx, yy, xx, height() - 2));
    painter.setPen(QPen(rightThickColor, THWIDTH, Qt::SolidLine, Qt::RoundCap));
    xx = (width() - m_noteX) + ((i + 1) * (THGAP + THWIDTH)) - THWIDTH;
    painter.drawLine(QLineF(xx, yy, xx, height() - 2));
  }
}


void TintonationView::resizeEvent(QResizeEvent* ) {
  m_nooFont = QFont("nootka");
  m_nooFont.setPointSizeF(height());
  QFontMetrics fm(m_nooFont);
  QRect noteBound = fm.boundingRect("n");
//   float factor = (float)height() / (float)noteBound.height();
//   m_nooFont.setPointSizeF(m_nooFont.pointSizeF() * factor);
//   noteBound = fm.boundingRect("n");
  m_noteX = (width() - noteBound.width() * 2) / 2;
  m_ticksCount = m_noteX / (THWIDTH + THGAP);
  m_hiTickStep = ((float)height() * 0.66) / m_ticksCount;
  m_tickColors.clear();
  for (int i = 0; i < m_ticksCount; i++) {
    if (i <= m_ticksCount*m_accurValue) {
//       m_tickColors << Qt::green;
//       qDebug("green");
      m_tickColors << gradColorAtPoint(0, m_noteX, Qt::green, Qt::yellow, (i + 1) * m_noteX / m_ticksCount);
    }
    else if (i <= m_ticksCount*0.5) {
//       qDebug("yellow");
//       m_tickColors << gradColorAtPoint(0, m_noteX, Qt::green, Qt::yellow, (i + 1) * m_noteX / m_ticksCount);
      m_tickColors << Qt::yellow;
      } else {
//           qDebug("red");
          m_tickColors << gradColorAtPoint(0, m_noteX, Qt::yellow, Qt::red, (i + 1) * m_noteX / m_ticksCount);
//       m_thickColors << Qt::red;
        }
  }
}

/** Implementation of linear gradient color at given point taken from:
 http://www.qtcentre.org/threads/14307-How-to-get-the-specified-position-s-QColor-in-QLinearGradient */
QColor TintonationView::gradColorAtPoint(float lineX1, float lineX2, QColor endC, QColor startC, float posC) {
  float segmentLength = sqrt((lineX2 - lineX1) * (lineX2 - lineX1));
  double pdist = sqrt((posC - lineX1) * (posC - lineX1));
  double ratio = pdist / segmentLength;
  int red = (int)(ratio * startC.red() + ( 1 - ratio) * endC.red()); //in your case, the values are 12 and 122
  int green = (int)(ratio * startC.green() + (1 - ratio) * endC.green()); //in your case, the values are 23 and 233
  int blue = (int)(ratio * startC.blue() + (1 - ratio) * endC.blue()); //in your case, the values are 24 and 244
  return QColor(red, green, blue);
}



