/***************************************************************************
 *   Copyright (C) 2011-2012 by Tomasz Bojczuk                             *
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


#include "texamsummary.h"
#include <QtGui>
#include "texam.h"
#include "tlevelselector.h"
#include "texamview.h"
#include "tqaunit.h"

  /** returns 2 columns row of table */
QString row2(QString S1, QString S2) {
  return QString("<tr><td>%1: </td><td><b>%2</b></td></tr>").arg(S1).arg(S2);
}


TexamSummary::TexamSummary(Texam* exam, QString &path, bool cont, QWidget *parent) :
  QDialog(parent)
{
  QHBoxLayout *lay = new QHBoxLayout();
//-------  left layout -----------------------
	QVBoxLayout *leftLay = new QVBoxLayout();
	
	QLabel *studentLab = new QLabel(tr("student:"), this);
	leftLay->addWidget(studentLab);
    QLabel *userNameLab = new QLabel(QString("<b style=\"font-size: 20px\">%1</b>").arg(exam->userName()), this);
    leftLay->addWidget(userNameLab, 0, Qt::AlignCenter);
	QLabel *questNrLAb = new QLabel(tr("Questions number:") + 
                    QString("<b style=\"font-size: 20px\">  %1</b>").arg(exam->count()), this);
	leftLay->addWidget(questNrLAb);
    QVBoxLayout *timeLay = new QVBoxLayout();
    QGroupBox *timeGr = new QGroupBox(tr("times:"), this);
    QLabel *timeLab = new QLabel("<table>" +
    row2(TexamView::totalTimetxt(), TexamView::formatedTotalTime(exam->totalTime()*1000)) +
    row2(tr("Time of work"), TexamView::formatedTotalTime(exam->workTime()*1000)) +
    row2(TexamView::averAnsverTimeTxt(), QString("%1 s").
        arg((qreal)exam->averageReactonTime()/10.0, 0, 'f', 1, '0')) +
    "</table>", this);
    timeLay->addWidget(timeLab);
  
    timeGr->setLayout(timeLay);
    leftLay->addWidget(timeGr);
	
    QHBoxLayout *buttLay =new QHBoxLayout;

    QPushButton *analyseButt = new QPushButton(tr("Analyse"), this);
    analyseButt->setIcon(QIcon(path + "picts/charts.png"));
    analyseButt->setIconSize(QSize(48, 48));
    QPushButton *okButt = new QPushButton(tr("Close"), this);
    if (cont) {
        okButt->setText(tr("Continue"));
        okButt->setIcon(QIcon(path + "picts/startExam.png"));
    }
    okButt->setIcon(QIcon(style()->standardIcon(QStyle::SP_DialogCloseButton)));
    okButt->setIconSize(QSize(48, 48));


    buttLay->addWidget(okButt);
    buttLay->addWidget(analyseButt);

    leftLay->addStretch(1);
    leftLay->addLayout(buttLay);

	lay->addLayout(leftLay);
  
//-------  right layout -----------------------	
	QVBoxLayout *rightLay = new QVBoxLayout();
	TlevelSummaryWdg *levelWdg = new TlevelSummaryWdg(this);
	rightLay->addWidget(levelWdg);
	levelWdg->setLevel(*(exam->level()));
	QVBoxLayout *resLay = new QVBoxLayout();
	QGroupBox *resGr = new QGroupBox(tr("Results:"), this);
	qreal eff = (((qreal)exam->count() - (qreal)exam->mistakes()) / (qreal)exam->count()) * 100;
  QString effStr = "";
  if (exam->mistakes()) {
    effStr = row2(TexamView::mistakesNrTxt(), QString::number(exam->mistakes()));
    effStr += row2(TexamView::corrAnswersNrTxt(), QString::number(exam->count()-exam->mistakes()));
    float wAccid = 0.0, wKey = 0.0, wNote = 0.0, wOctave = 0.0, wStyle = 0.0, wPos = 0.0, wTotal;
    for(int i=0; i<exam->count(); i++) {
      if (!exam->qusetion(i).correct()) {
          if(exam->qusetion(i).wrongAccid())  wAccid++;
          if(exam->qusetion(i).wrongKey())    wKey++;
          if(exam->qusetion(i).wrongNote())   wNote++;
          if(exam->qusetion(i).wrongOctave()) wOctave++;
          if(exam->qusetion(i).wrongStyle())  wStyle++;
          if(exam->qusetion(i).wrongPos())    wPos++;
      }
    }
    effStr += "<tr><td colspan=\"2\">----- " + tr("Kinds of mistakes") + ": -----</td></tr>";
    wTotal = wAccid + wKey + wNote + wOctave + wStyle + wPos;
    if (wNote)
      effStr += row2(tr("Wrong notes"), QString("%1 (").arg(wNote) + QString::number(qRound(wNote*100.0 / wTotal)) + "%)");
    if (wAccid)
      effStr += row2(tr("Wrong accidentals"), QString("%1 (").arg(wAccid) + QString::number(qRound(wAccid*100.0 / wTotal)) + "%)");
    if (wKey)
      effStr += row2(tr("Wrong key signatures"), QString("%1 (").arg(wKey) + QString::number(qRound(wKey*100.0 /wTotal)) + "%)");
    if (wOctave)
      effStr += row2(tr("Wrong octaves"), QString("%1 (").arg(wOctave) + QString::number(qRound(wOctave*100.0 /wTotal)) + "%)");
    if (wStyle)
      effStr += row2(tr("Wrong note name-calling"), QString("%1 (").arg(wStyle)) + 
          QString::number(qRound(wStyle*100.0 /wTotal)) + "%)";
    if (wPos)
      effStr += row2(tr("Wrong frets or strings"), QString("%1 (").arg(wPos) + QString::number(qRound(wPos*100.0 / wTotal)) + "%)");
  }
	QLabel *resLab = new QLabel("<table>" +
    row2(TexamView::effectTxt(), QString::number(qRound(eff)) + "%") + effStr + "</table>", this);
	resLay->addWidget(resLab);
	
	resGr->setLayout(resLay);
	rightLay->addWidget(resGr);
	
	lay->addLayout(rightLay);
	
  setLayout(lay);
  
  connect(okButt, SIGNAL(clicked()), this, SLOT(close()));
  connect(analyseButt, SIGNAL(clicked()), this, SLOT(accept()));
  
}

TexamSummary::~TexamSummary()
{

}

