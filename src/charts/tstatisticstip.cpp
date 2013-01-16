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


#include "tstatisticstip.h"
#include "tgroupedqaunit.h"
#include "texamview.h"
#include "tqaunit.h"
#include "tquestionpoint.h"
#include "tglobals.h"
#include <QApplication>

QString trStyle(QColor c) {
    c.setAlpha(30);
    return QString("style=\"%1\"").arg(Tglobals::getBGcolorText(c));
}

/*static*/
QString TstatisticsTip::getTipText(TgroupedQAunit* qaGroup)  {
    QString tipText = QApplication::translate("TstatisticTip", "Statistics") + "<br>";
    tipText += "<b>" + qaGroup->fullDescription() + "</b><hr>";
    tipText += "<table><tr><td>";
    tipText += TexamView::effectTxt() + ": </td><td> <b>" + QString("%1 %").arg(qaGroup->effectiveness(), 2, 'f', 0, '0') + "</b></td></tr><tr><td>";
    tipText += TexamView::averAnsverTimeTxt() + ": </td><td> <b>" + TexamView::formatReactTime(qaGroup->averTime(), true) + "</b></td></tr><tr><td>";
    tipText += QApplication::translate("TstatisticTip", "Questions number:") + QString(" </td><td> <b>%1</b></td></tr>").arg(qaGroup->size());
    tipText += "<tr " + trStyle(TquestionPoint::goodColor()) + "><td>";
    tipText += TexamView::corrAnswersNrTxt() + QString(": </td><td> <b>%1</b></td></tr>")
        .arg(qaGroup->size() - qaGroup->mistakes() - qaGroup->notBad());
    if (qaGroup->mistakes())
      tipText += "<tr " + trStyle(TquestionPoint::wrongColor())  + "><td>" + TexamView::mistakesNrTxt() +
              QString(": </td><td> <b>%1</b></td></tr>").arg(qaGroup->mistakes());
    if (qaGroup->notBad())
      tipText += "<tr " + trStyle(TquestionPoint::notBadColor()) + "><td>" + TexamView::halfMistakenTxt() +
              QString(": </td><td> <b>%1</b></td></tr>").arg(qaGroup->notBad());
    tipText += "</table>";
    return tipText;
}

QString TstatisticsTip::getAverTimeStat(TgroupedQAunit* qaGroup, QString ofSomething) {
    if (qaGroup)
      return "<p>" + TexamView::averAnsverTimeTxt() + "<br>" + ofSomething + "<br>" + 
        TexamView::formatReactTime(qRound(qaGroup->averTime()), true) +"</p>";
    else 
      return ofSomething;
}




TstatisticsTip::TstatisticsTip(TgroupedQAunit* qaGroup, Ekind kind, QString desc) :
    TgraphicsTextTip(),
    m_qaGroup(qaGroup)
{
    setBgColor(QColor(0, 192, 192)); // light blue
    if (kind == e_full)
      setHtml(getTipText(qaGroup));
    else
      setHtml(getAverTimeStat(qaGroup, desc));
}

TstatisticsTip::~TstatisticsTip() {}




