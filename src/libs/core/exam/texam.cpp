/***************************************************************************
 *   Copyright (C) 2011-2014 by Tomasz Bojczuk                             *
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


#include "texam.h"
#include "tlevel.h"
#include "tinitcorelib.h"
#include <tscoreparams.h>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>



/*static*/
/** Versions history:
 * 1. 0x95121702; 
 * 
 * 2. 0x95121704; (2012.07)
 * 		- exam stores penalties in the list
 *
 * 3. 0x95121706 (2013.12.02)
 * 		- new level version
 */

const qint32 Texam::examVersion = 0x95121702;
const qint32 Texam::examVersion2 = 0x95121704;
const qint32 Texam::currentVersion = 0x95121706;

const quint16 Texam::maxAnswerTime = 65500;

int Texam::examVersionNr(qint32 ver) {
	if ((ver - examVersion) % 2)
			return -1; // invalid when rest of division is 1
	return ((ver - examVersion) / 2) + 1 ;
}


bool Texam::couldBeExam(qint32 ver) {
	int givenVersion = examVersionNr(ver);
	if (givenVersion >= 1 && givenVersion <= 127)
		return true;
	else
		return false;
}


bool Texam::isExamVersion(qint32 ver) {
	if (examVersionNr(ver) <= examVersionNr(currentVersion))
		return true;
	else
		return false;
}


qint32 Texam::examVersionToLevel(qint32 examVer) {
	if (examVersionNr(examVer) <= 2)
		return Tlevel::getVersionId(1); // level version 1 for exam versions 1 and 2
	else
		return Tlevel::getVersionId(2); // level version 2 for exam versions 3 and so
}


bool Texam::areQuestTheSame(TQAunit& q1, TQAunit& q2) {
  if (q1.questionAs == q2.questionAs && // the same questions
      q1.answerAs == q2.answerAs && // the same answers
      q1.qa.note == q2.qa.note && // the same notes
      q1.qa.pos == q2.qa.pos // the same frets
    )
        return true;
  else
        return false;
}

qreal Texam::effectiveness(int questNumber, int mistakes, int notBad) {
    return (((qreal)questNumber - (qreal)(mistakes + ((qreal)notBad / 2))) / (qreal)questNumber) * 100.0;   
}


QString Texam::formatReactTime(quint16 timeX10, bool withUnit) {
		QString hh = "", mm = "", ss = "";
    int dig = 0;
    if (timeX10 / 36000) {
        hh = QString("%1").arg(timeX10 / 36000);
        dig = 2;
    }
    int dig2 = 0;
    if ((timeX10 % 36000) / 600) {
        mm = QString("%1").arg((timeX10 % 36000) / 600, dig, 'i', 0, '0');
        dig2 = 2;
    }
    ss = QString("%1").arg(((timeX10 % 36000) % 600) / 10, dig2, 'i', 0, '0' );
    QString res = "";
    if (hh != "")
        res = hh + ":";
    if (mm != "")
        res += mm + ":";
    QString unitS = "";
    if (withUnit && timeX10 < 600)
        unitS = " s";
    return res + ss + QString(".%1").arg(timeX10 % 10) + unitS;
}

/*end of static*/


Texam::Texam(Tlevel* l, QString userName):
  m_level(l),
  m_userName(userName),
  m_fileName(""),
  m_mistNr(0),
  m_workTime(0),
  m_penaltysNr(0),
  m_isFinished(false),
  m_halfMistNr(0),
  m_blackCount(0)
{

}


Texam::~Texam()
{
  m_answList.clear();
  m_blackList.clear();  
}


Texam::EerrorType Texam::loadFromFile(QString& fileName) {
    m_fileName = fileName;
    QFile file(fileName);
    quint16 questNr;
    m_workTime = 0;
    m_mistNr = 0;
    m_blackCount = 0;
    m_blackList.clear();
    m_answList.clear();
    EerrorType result = e_file_OK;
    quint32 ev; //exam template version
    if (file.open(QIODevice::ReadOnly)) {
      QDataStream in(&file);
      in.setVersion(QDataStream::Qt_4_7);
      in >> ev;
//       if (ev != examVersion && ev != examVersion2)
			if (couldBeExam(ev)) {
				if (!isExamVersion(ev))
						return e_newerVersion;
			}	else
					return e_file_not_valid;

      in >> m_userName;
//       getLevelFromStream(in, *(m_level));
			getLevelFromStream(in, *(m_level), examVersionToLevel(ev));
      in >> m_tune;
      in >> m_totalTime;
      in >> questNr >> m_averReactTime >> m_mistNr;
//       if (ev == examVersion2) {
			if (examVersionNr(ev) >= 2) {
        in >> m_halfMistNr >> m_penaltysNr >> m_isFinished;
      } else { // exam version 1
        m_halfMistNr = 0;
        m_penaltysNr = 0;
        m_isFinished = false;
      }
      bool isExamFileOk = true;
      int tmpMist = 0;
      int tmpHalf = 0;
      int fixedNr = 0;
      int okTime = 0; // time of correct and notBad answers to calculate average
      while (!in.atEnd()) {
          TQAunit qaUnit;
          if (!getTQAunitFromStream(in, qaUnit))
              isExamFileOk = false;
          if ((qaUnit.questionAs == TQAtype::e_asName || qaUnit.answerAs == TQAtype::e_asName) 
                && qaUnit.styleOfQuestion() < 0) {
                  qaUnit.setStyle(Tglob::glob()->S->nameStyleInNoteName, qaUnit.styleOfAnswer());
                  fixedNr++;
              } /** In old versions, style was set to 0 so now it gives styleOfQuestion = -1
                * Also in transition Nootka versions it was left unchanged.
                * Unfixed it invokes stupid names in charts.
                * We are fixing it by insert user preferred style of naming */
          if (qaUnit.time <= maxAnswerTime || ev == examVersion) { // add to m_answList
              m_answList << qaUnit;
              m_workTime += qaUnit.time;
              if ( !qaUnit.isCorrect() ) {
                if (qaUnit.isWrong())
                  tmpMist++;
                else
                  tmpHalf++; // not so bad answer
              }
              if (!qaUnit.isWrong())
                  okTime += qaUnit.time;
          } else { // add to m_blackList
              m_blackList << qaUnit;
          }
      }
      if (questNr != m_answList.size()) {
        isExamFileOk = false;        
      }
//       if (ev == examVersion2 && (tmpMist != m_mistNr || tmpHalf != m_halfMistNr)) {
			if (examVersionNr(ev) >= 2 && (tmpMist != m_mistNr || tmpHalf != m_halfMistNr)) {
        m_mistNr = tmpMist; //we try to fix exam file to give proper number of mistakes
        m_halfMistNr = tmpHalf;
        isExamFileOk = false;
      } else {
        m_mistNr = tmpMist; // transition to exam version 2
      }
      if (ev == examVersion) {
          convertToVersion2();
          m_halfMistNr = tmpHalf;
      }
      if (fixedNr)
          qDebug() << "fixed style in questions:" << fixedNr;
//       m_averReactTime = m_workTime / count(); // OBSOLETE
      if ((count() - mistakes()))
        m_averReactTime = okTime / (count() - mistakes());
      else 
        m_averReactTime = 0.0;
      if (!isExamFileOk)
          result = e_file_corrupted;
      file.close();
     } else {
					Tlevel::fileIOerrorMsg(file, 0);
          result = e_cant_open;
     }
  updateBlackCount();
  return result;
}


Texam::EerrorType Texam::saveToFile(QString fileName) {
	if (fileName != "")
		setFileName(fileName); // m_fileName becomes fileName
	if (m_fileName == "")
		return e_noFileName;
	QFile file(m_fileName);
	if (file.open(QIODevice::WriteOnly)) {
		QDataStream out(&file);
		out.setVersion(QDataStream::Qt_4_7);
		out << currentVersion;
// 		out << m_userName << *m_level << m_tune; TODO !!!!!!!!!!!!!!!!! XML !!!!!!!!!!!!!!!!!!!!!!!!!!!
		out << m_totalTime; // elapsed exam time (quint32)
			// data for file preview
		out << (quint16)m_answList.size(); // number of questions
		out << m_averReactTime; // average time of answer (quint16)
			// that's all
		out << m_mistNr; // number of mistakes (quint16)
      /** Those were added in version 2 */
		out << m_halfMistNr << m_penaltysNr << m_isFinished;
		for (int i = 0; i < m_answList.size(); i++)
				out << m_answList[i]; // and obviously answers
	  if (m_blackList.size()) {
      for (int i = 0; i < m_blackList.size(); i++)
        out << m_blackList[i]; // and black list
    }
	} else {
		QMessageBox::critical(0, "",
           QObject::tr("Cannot save exam file:\n%1").arg(QString::fromLocal8Bit(qPrintable(file.errorString()))));
		return e_cant_open;
	}
	qDebug() << "Exam saved to:" << m_fileName;
	return e_file_OK;
}


void Texam::setAnswer(TQAunit& answer) {
    answer.time = qMin(maxAnswerTime, answer.time); // when user think too much
//     m_answList.last() = answer; // it is not necessary as long as Texam::curQ() returns a reference to the last question in the answers list (m_answList) so checkAnswer() of TexamExecutor did all changes directly on last element of TQAunit
    if (!answer.isCorrect() && !curQ().melody()) {
      if (!isFinished()) // finished exam has got no black list
          m_blackList << answer;
      if (answer.isNotSoBad()) {
        if (!isFinished()) {
            m_blackList.last().time = 65501;
            m_penaltysNr++;
        }
        m_halfMistNr++;
      } else {
        if (!isFinished()) {
            m_blackList.last().time = 65502;
            m_penaltysNr += 2;
        }
        m_mistNr++;
      }
    }
    m_workTime += answer.time;
    if (!isFinished() && !curQ().melody())
        updateBlackCount();
}


void Texam::removeLastQuestion() {
	m_workTime -= curQ().time;
	m_answList.removeLast();
}



//############################### PROTECTED ########################################

void Texam::updateBlackCount() {
  m_blackCount = 0;
  if (m_blackList.size()) {
    for (int i = 0; i < m_blackList.size(); i++)
    m_blackCount += (m_blackList[i].time - maxAnswerTime);
  }
}


void Texam::convertToVersion2() {
  bool hasStyle = false;
  Tnote::EnameStyle randStyles[3];
  if (m_level->canBeName()) {
    // version 1 didn't put proper Tnote::EnameStyle to file - we fixing it
    hasStyle = true;
    qDebug("Fixing styles of note names in file");
    qsrand(QDateTime::currentDateTime().toTime_t());
   if (m_level->requireStyle) { // prepare styles array to imitate switching
      randStyles[0] = Tnote::e_italiano_Si;
      if (Tglob::glob()->S->seventhIs_B) {
        randStyles[1] = Tnote::e_english_Bb;
        randStyles[2] = Tnote::e_nederl_Bis;
      } else {
        randStyles[1] = Tnote::e_norsk_Hb;
        randStyles[2] = Tnote::e_deutsch_His;
      }
   }
  }
  
  for (int i = 0; i < m_answList.size(); i++) {
    if (m_answList[i].time > maxAnswerTime) // fix too long times from version 1 if any
        m_answList[i].time = maxAnswerTime;
  // version 1 didn't put proper Tnote::EnameStyle to file - we fixing it
    if (hasStyle) {
      if (m_level->requireStyle) {
        if (m_answList[i].questionAs == TQAtype::e_asName && m_answList[i].answerAs == TQAtype::e_asName) {
          Tnote::EnameStyle qSt = randStyles[qrand() % 3];
          Tnote::EnameStyle aSt;
          if (qSt == Tnote::e_italiano_Si)
            aSt = randStyles[(qrand() % 2) +1];
          else
            aSt = Tnote::e_italiano_Si;
          m_answList[i].setStyle(qSt, aSt);
        } else
          if (m_answList[i].questionAs == TQAtype::e_asName) {
            m_answList[i].setStyle(randStyles[qrand() % 3], Tglob::glob()->S->nameStyleInNoteName);
          } else
            if (m_answList[i].questionAs == TQAtype::e_asName) {
              m_answList[i].setStyle(Tglob::glob()->S->nameStyleInNoteName, randStyles[qrand() % 3]);
            }
      } else // fixed style - we changing to user preferred
          m_answList[i].setStyle(Tglob::glob()->S->nameStyleInNoteName, Tglob::glob()->S->nameStyleInNoteName);
    }
      
    if (!m_answList[i].isCorrect()) {
      quint16 penCnt = 0; // counts of penalties
      if (m_answList[i].isWrong()) {
        if (i < (m_answList.size() -1) && areQuestTheSame(m_answList[i], m_answList[i+1])) {
          // there was next question repeated
          if (m_answList[i+1].isCorrect()) // and was correct
            penCnt = 65501; // so add one penalty
          else // when again wrong
            penCnt = 65502; // add two
          // The next loop will add next two penalties !!
        } else // question was not repeated
            penCnt = 65502;
      } else { // not so bad
        if (i < (m_answList.size() -1) && areQuestTheSame(m_answList[i], m_answList[i+1])) {
          // there was next question repeated
          if (m_answList[i+1].isCorrect()) // and was correct
//             m_blackList.removeLast(); // remove it from black list - corrected
            penCnt = 0;
          else
            penCnt = 65501;
        }
      }
      if (penCnt) {
        m_blackList << m_answList[i];
        m_blackList.last().time = penCnt;
        m_penaltysNr += (penCnt - 65500);
      }
    }
  }
  qDebug() << "Converted to exam version 2!!!  black list:" << m_blackList.size() << "penaltys:" << m_penaltysNr;
}



