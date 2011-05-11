/***************************************************************************
 *   Copyright (C) 2011 by Tomasz Bojczuk  				   *
 *   tomaszbojczuk@gmail.com   						   *
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
 *  You should have received a copy of the GNU General Public License	   *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/


#include "texamexecutor.h"
#include "tglobals.h"
#include "tstartexamdlg.h"
#include "mainwindow.h"
#include <QDebug>

extern Tglobals *gl;

TexamExecutor::TexamExecutor(MainWindow *mainW)
{
    mW = mainW;

    TstartExamDlg *startDlg = new TstartExamDlg;
    QString actTxt;
    TstartExamDlg::Eactions userAct = startDlg->showDialog(actTxt, m_level);
    if (userAct == TstartExamDlg::e_newLevel) {
//        qDebug() << "Level: " << m_level.name;

    } else 
      return;

//    prepareToExam();
    createQuestionsList();

    m_prevAccid = Tnote::e_Natural;
    m_dblAccidsCntr = 0;
    m_level.questionAs.randNext(); // Randomize question and answer type
    for (int i = 0; i < 4; i++)
        m_level.answersAs[i].randNext();

    nextQuestAct = new QAction(tr("next question"), this);
    nextQuestAct->setStatusTip(nextQuestAct->text());
    nextQuestAct->setIcon(QIcon(gl->path+"picts/nextQuest.png"));
    connect(nextQuestAct, SIGNAL(triggered()), this, SLOT(askQuestion()));
    mW->nootBar->addSeparator();
    mW->nootBar->addAction(nextQuestAct);

}


void TexamExecutor::createQuestionsList() {
  // 1. creating array with guitar strings in order of their height
    char strOrder[6] = { 0,1,2,3,4,5};
    char openStr[6];
    for (int i=0; i<6; i++)
        openStr[i] = gl->Gtune[i+1].getChromaticNrOfNote();

    int i = 4;
    while (i > -1) {
        for (int j=i; j < 5 && openStr[strOrder[j]] < openStr[strOrder[j+1]]; j++) {
            char tmp = strOrder[j];
            strOrder[j] = strOrder[j+1];
            strOrder[j+1] = tmp;
        }
        i--;
    }

  // array done, stored in strOrder[6]

//    for (int i=0; i<6; i++) qDebug() << i << ": " << (int)openStr[strOrder[i]]
//            << " : " << (int)strOrder[i];

  // 2. searching all frets in range, string by string
    for(int s = 0; s < 6; s++) {
        if (m_level.usedStrings[strOrder[s]])// check string by strOrder
            for (int f = m_level.loFret; f <= m_level.hiFret; f++) {
            Tnote n = Tnote(gl->Gtune[strOrder[s]+1].getChromaticNrOfNote() + f);
            if (n.getChromaticNrOfNote() >= m_level.loNote.getChromaticNrOfNote() &&
                n.getChromaticNrOfNote() <= m_level.hiNote.getChromaticNrOfNote()) {
                bool hope = true; // we stil have hope that note is for an exam
                if (m_level.onlyLowPos) {
                    if (s > 0) {
//                    if (strOrder[s] > 0) {
                       // we have to check when note is on the lowest positions
                       // is it realy lowest pos
                       // when strOrder[s] is 0 - it is the highest sting
                       char diff = openStr[strOrder[s-1]] - openStr[strOrder[s]];
                       if( (f-diff) >= m_level.loFret && (f-diff) <= m_level.hiFret) {
                           hope = false; //There is the same note on highest string
                           qDebug() << (int)s << (int)diff << (int)f-diff << "bad";
                       }
                       else {
                           hope = true;
                           qDebug() << (int)strOrder[s] << (int)diff << (int)f-diff << "ok";
                       }
                    }
                }
                if (hope && m_level.useKeySign && m_level.onlyCurrKey) {
                  hope = false;
                  if (m_level.isSingleKey) {
                    if(m_level.loKey.inKey(n).note != 0)
                        hope = true;
                    } else {
                        for (int k = m_level.loKey.value(); k <= m_level.hiKey.value(); k++) {
                          if (TkeySignature::inKey(TkeySignature(k), n).note != 0) {
                            hope = true;
                            break;
                          }
                        }
                    }
                }
                if (hope) {
                    TQAunit::TQAgroup g;
                    g.note = n; g.pos = TfingerPos(strOrder[s]+1, f);
                    m_questList << g;
                }
            }
        }
    }

//    if (m_questList.size() == 0)
//    for (int i = 0; i < m_questList.size(); i++)
//        qDebug() << i << (int)m_questList[i].pos.str() << "f"
//                << (int)m_questList[i].pos.fret() << " note: "
//                << QString::fromStdString(m_questList[i].note.getName());
}

void TexamExecutor::askQuestion() {
    TQAunit curQ = TQAunit();
    curQ.qa = m_questList[qrand() % m_questList.size()];
    curQ.questionAs = m_level.questionAs.next();
    curQ.answerAs = m_level.answersAs[curQ.questionAs].next();

    if (curQ.questionAs == TQAtype::e_asNote || curQ.answerAs == TQAtype::e_asNote) {
        if (m_level.useKeySign) {
            Tnote tmpNote = curQ.qa.note;
            if (m_level.isSingleKey) {
                curQ.key = m_level.loKey;
                tmpNote = m_level.loKey.inKey(curQ.qa.note);
            } else {
                curQ.key = TkeySignature(qrand() % (m_level.hiKey.value() - m_level.loKey.value()));
                if (m_level.onlyCurrKey) {
                    int keyRangeWidth = m_level.hiKey.value() - m_level.loKey.value();
                    int patience = 0;
                    char keyOff = curQ.key.value() - m_level.loKey.value();
                    tmpNote = curQ.key.inKey(curQ.qa.note);
                    while(tmpNote.note == 0 && patience < keyRangeWidth) {
                        keyOff++;
                        if (keyOff > keyRangeWidth) keyOff = 0;
                        curQ.key = TkeySignature(m_level.loKey.value() + keyOff);
                        patience++;
                        tmpNote = curQ.key.inKey(curQ.qa.note);
                        if (patience >= keyRangeWidth) {
                            qDebug() << "Oops!! It should never happend. I can not find key signature!!";
                            break;
                        }
                    }
                }
            }
        }
        if ( !m_level.onlyCurrKey)
            curQ.qa.note = determineAccid(curQ.qa.note);
    }
    qDebug() << QString::fromStdString(curQ.qa.note.getName()) << (int)curQ.questionAs
            << (int)curQ.answerAs;
}

Tnote TexamExecutor::determineAccid(Tnote n) {
    return n;
}

void TexamExecutor::CheckAnswer() {

}

void TexamExecutor::prepareToExam() {
    mW->setStatusMessage("exam started on level:<br><b>" + m_level.name + "</b>");
    mW->settingsAct->setDisabled(true);
    mW->levelCreatorAct->setDisabled(true);
    mW->startExamAct->setDisabled(true);

    disconnect(mW->score, SIGNAL(noteChanged(int,Tnote)), mW, SLOT(noteWasClicked(int,Tnote)));
    disconnect(mW->noteName, SIGNAL(noteNameWasChanged(Tnote)), mW, SLOT(noteNameWasChanged(Tnote)));
    disconnect(mW->guitar, SIGNAL(guitarClicked(Tnote)), mW, SLOT(guitarWasClicked(Tnote)));

}

void TexamExecutor::restoreAfterExam() {
    mW->settingsAct->setDisabled(false);
    mW->levelCreatorAct->setDisabled(false);
    mW->startExamAct->setDisabled(false);

    connect(mW->score, SIGNAL(noteChanged(int,Tnote)), mW, SLOT(noteWasClicked(int,Tnote)));
    connect(mW->noteName, SIGNAL(noteNameWasChanged(Tnote)), mW, SLOT(noteNameWasChanged(Tnote)));
    connect(mW->guitar, SIGNAL(guitarClicked(Tnote)), mW, SLOT(guitarWasClicked(Tnote)));

}
