/***************************************************************************
 *   Copyright (C) 2012 by Tomasz Bojczuk                                  *
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


#include "tquestiontip.h"
#include "tqaunit.h"
#include "texam.h"
#include <tnotename.h>
#include "tglobals.h"
#include <texamlevel.h>
#include <ttipchart.h>

extern Tglobals *gl;




/* static */
QString TquestionTip::getTextHowAccid(Tnote::Eacidentals accid) {
    QString S = QString("<br><span style=\"color: %1\">").arg(gl->GfingerColor.name());
    if (accid) S += tr("Use %1").arg(QString::fromStdString(signsAcid[accid + 2]));
    else S += tr(" Don't use accidentals!");
    S +=  "</span>";
    return S;
}


QString TquestionTip::onStringTxt(quint8 strNr) {
    return tr("on <span style=\"font-family: nootka;\">%1</span> string.").arg(strNr);
}

//##########################################################################################
//#################################### CONSTRUCTOR #########################################
//##########################################################################################

TquestionTip::TquestionTip(Texam* exam, Tnote::EnameStyle style) :
  TgraphicsTextTip(),
  m_scoreFree(true),
  m_nameFree(true),
  m_guitarFree(true)
{
  setBgColor(gl->EquestionColor);
  setHtml(getQuestion(exam->qusetion(exam->count()-1), exam->count(), exam->level(), style));
}

TquestionTip::~TquestionTip() {}


//##########################################################################################
//#################################### PROTECTED ###########################################
//##########################################################################################

QString TquestionTip::getNiceNoteName(Tnote note) {
  return QString("<b><span style=\"%1\">&nbsp;").arg(gl->getBGcolorText(gl->EquestionColor)) +
            TnoteName::noteToRichText(note) + " </span></b>";
}

QString TquestionTip::getQuestion(TQAunit& question, int questNr, TexamLevel* level, Tnote::EnameStyle style) {
  QString quest = QString("<b><u>&nbsp;%1.&nbsp;</u></b><br>").arg(questNr);
    QString apendix = "";
    QString noteStr;
    switch (question.questionAs) {
      case TQAtype::e_asNote:
        m_scoreFree = false;
        if (question.answerAs == TQAtype::e_asNote) {
            if (question.qa.note.acidental != question.qa_2.note.acidental)
                quest += tr("Change enharmonicaly and show in the score");
            else
                quest += tr("Given note show in the score");
          if (level->useKeySign && level->manualKey) {
            apendix = tr("<br><b>in %1 key.</b>", "in key signature").arg(question.key.getName());
          }
          if (level->forceAccids)
            quest += getTextHowAccid((Tnote::Eacidentals)question.qa_2.note.acidental);
        } else
          if (question.answerAs == TQAtype::e_asName) {
            m_nameFree = false;
            quest += tr("Give name of");
          } else
            if (question.answerAs == TQAtype::e_asFretPos) {
              m_guitarFree = false;
              quest += tr("Show on the guitar");
              if (level->showStrNr)
                apendix = "<br><b> " + onStringTxt(question.qa.pos.str()) + "</b>";
            } else
              if (question.answerAs == TQAtype::e_asSound) {
                quest += tr("Play or sing");
              }
        if (level->useKeySign && level->manualKey) // hide key signature
            quest += "<br>" + TtipChart::wrapPixToHtml(question.qa.note, true, TkeySignature(0));
        else
            quest += "<br>" + TtipChart::wrapPixToHtml(question.qa.note, true, question.key);
        if (apendix != "")
          quest += apendix;
      break;
      
      case TQAtype::e_asName:
        m_nameFree = false;
        noteStr = "<br>" + getNiceNoteName(question.qa.note);
        if (question.answerAs == TQAtype::e_asNote) {
          m_nameFree = false;
          quest += tr("Show in the score") + noteStr;
          if (level->useKeySign && level->manualKey) {
            quest += tr("<br><b>in %1 key.</b>", "in key signature").arg(question.key.getName());
          }
        } else
          if (question.answerAs == TQAtype::e_asName) {
            m_nameFree = false;
            Tnote::EnameStyle tmpStyle = gl->NnameStyleInNoteName;
            gl->NnameStyleInNoteName = style;
            noteStr = "<br>" + getNiceNoteName(question.qa.note);
            if (question.qa.note.acidental != question.qa_2.note.acidental)
                quest += tr("Change enharmonicaly and give name of");
            else
                quest += tr("Use another style to give name of");
            quest += noteStr + getTextHowAccid((Tnote::Eacidentals)question.qa_2.note.acidental);
            gl->NnameStyleInNoteName = tmpStyle;
            // It is not so elegant to get note name in different style this way
            // but there is no other way
          } else
            if (question.answerAs == TQAtype::e_asFretPos) {
              m_guitarFree = false;
              quest += tr("Show on the guitar") + noteStr;
              if (level->showStrNr)
                quest += "<br><b> " + onStringTxt(question.qa.pos.str()) + "</b>";
            } else
              if (question.answerAs == TQAtype::e_asSound) {
                quest += "<br>" + tr("Play or sing") + noteStr;
              }
      break;
      
      case TQAtype::e_asFretPos:
        quest += "";
        m_guitarFree = false;
        if (question.answerAs == TQAtype::e_asNote) {
          m_scoreFree = false;
          quest += tr("Show on the score note played on");
          if (level->useKeySign && level->manualKey) {
            apendix = tr("<b>in %1 key.</b>", "in key signature").arg(question.key.getName());
          }
        } else
          if (question.answerAs == TQAtype::e_asName) {
            m_nameFree = false;
            quest += tr("Give name of");
          } else
            if (question.answerAs == TQAtype::e_asFretPos) {
              quest += tr("Show sound from position:", "... and string + fret numbers folowing");
              apendix = "<br><b> " + onStringTxt(question.qa_2.pos.str()) + "</b>";
            } else
              if (question.answerAs == TQAtype::e_asSound) {
                  quest += tr("Play or sing");
              }
        quest += QString("<br><span style=\"font-size: 30px; %1\">&nbsp;").arg(gl->getBGcolorText(gl->EquestionColor)) +
                    question.qa.pos.toHtml() + " </span>";
        if (apendix != "")
          quest += "<br>" + apendix;
        if (question.answerAs == TQAtype::e_asNote || question.answerAs == TQAtype::e_asName)
          if (level->forceAccids)
            quest += "<br" + getTextHowAccid((Tnote::Eacidentals)question.qa.note.acidental);
        
      break;
      
      case TQAtype::e_asSound:
        if (question.answerAs == TQAtype::e_asNote) {
          m_scoreFree = false;
          quest += tr("Listened sound show in the score");
          if (level->useKeySign && level->manualKey) {
            quest += tr("<br><b>in %1 key.</b>", "in key signature").arg(question.key.getName());
          }
          if (level->forceAccids)
            quest += getTextHowAccid((Tnote::Eacidentals)question.qa.note.acidental);
        } else
          if (question.answerAs == TQAtype::e_asName) {
            m_nameFree = false;
            quest += tr("Give name of listened sound");
            if (level->forceAccids)
                quest += getTextHowAccid((Tnote::Eacidentals)question.qa.note.acidental);
          } else
            if (question.answerAs == TQAtype::e_asFretPos) {
              m_guitarFree = false;
              quest += tr("Listened sound show on the guitar");
              if (level->showStrNr)
              quest += "<br><b> " + onStringTxt(question.qa.pos.str()) + "</b>";
            } else
              if (question.answerAs == TQAtype::e_asSound) {
                quest += tr("Play or sing listened sound");          
              }
      break;
    }
    
    return quest;
  
}







