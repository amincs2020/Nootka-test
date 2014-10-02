/***************************************************************************
 *   Copyright (C) 2014 by Tomasz Bojczuk                                  *
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

#ifndef TPENALTY_H
#define TPENALTY_H


#include <QObject>

class TprogressWidget;
class TexamView;
class TexecutorSupply;
class Texam;


/** 
 * This class manages penalties during exam execution.
 * @p m_penalStep is determines how many 'normal' questions is asked between penalties
 * When it is 0 - penalty is asked every time.
 * @p m_penalCount counts questions and is increased by @p nextQuestion()
 * @p m_blackQuestNr keeps number of question in a black list or it is -1 when 'normal' question.
 * 
 * @p nextQuestion() is invoked fromTexam::askQuestion() to increase counter
 * then @p ask() prepares the penalty if its time was come.
 * @p releaseBlackList() is invoked from Texam::checkAnswer() 
 * and when penalty was asked it removes it from black list.
 * @p setBlackQuestion() is called from Texam::repeatQuestion() 
 * to set @p m_blackQuestNr to the last question in the black list and when answer on it is correct
 * this repeated question is removed from the list.
 */
class Tpenalty : public QObject
{
	
	Q_OBJECT
	
public:
	Tpenalty(Texam* exam, TexecutorSupply* supply, TexamView* examView, TprogressWidget* progress);
	
	bool isNot() { return m_blackQuestNr == -1 && m_blackNumber == -1; } /** @p TRUE when a question is not penalty */
	
			/** Check state of counters and if it is time for penalty
			 * prepares last question in @p Texam list by copying someone from black list
			 * and returns @p TRUE. Otherwise just returns @p FALSE */
	bool ask();
	
	void updatePenalStep();
	void nextQuestion(); /** Increases counter of question, resets m_blackQuestNr. Starts timer */
	void checkAnswer(); /** Checks last answer and when not valid adds penalty(s) to black list. */
	void setMelodyPenalties();
	void releaseBlackList(); /** If asked question was penalty and answer was correct it removes penalty from black list. */
	void checkForCert(); /** Checks could be exam finished and emits @p certificate() when it can. */
	
	void pauseTime(); /** Pauses exam view timers. */
	void continueTime(); /** Continues exam view timers. */
	void updateExamTimes(); /** Updates exam variables with already elapsed times */
	void stopTimeView(); /** Stops refreshing elapsing time on the exam view labels */
	
			/** Sets @p m_blackQuestNr to the last question from the black list
			* because previous answer was wrong or not so bad and it was added at the end of blacList
			* When an answer will be correct the list will be decreased. */
	void setBlackQuestion();
	
signals:
	void certificate(); /** Emitted when last mandatory question was correct. */
	
private:
	TexamView								*m_examView;
	TprogressWidget					*m_progress;
	Texam										*m_exam;
	TexecutorSupply					*m_supply;
	int 										 m_blackQuestNr; /** -1 if no black, otherwise points question in blackList list. */
	int											 m_blackNumber; /** Points number is black numbers list or -1 for none */
	int 										 m_penalStep; /** Interval of questions, after it penalty question is asked */
	int 										 m_penalCount; /** Counts questions to ask penalties one. */
};

#endif // TPENALTY_H
