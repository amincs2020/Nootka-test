/***************************************************************************
 *   Copyright (C) 2013-2014 by Tomasz Bojczuk                             *
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

#include "tscorestaff.h"
#include "tscorescene.h"
#include "tscoreclef.h"
#include "tscorenote.h"
#include "tscorekeysignature.h"
#include "tscorecontrol.h"
#include "tscorescordature.h"
#include <music/tnote.h>
#include <animations/tcombinedanim.h>
#include <tnoofont.h>
#include <QApplication>
#include <QGraphicsView>
#include <QPalette>

#include <QDebug>


TnoteOffset::TnoteOffset(int noteOff, int octaveOff) :
  note(noteOff),
  octave(octaveOff)
{}



TscoreStaff::TscoreStaff(TscoreScene* scene, int notesNr) :
  TscoreItem(scene),
  m_offset(TnoteOffset(3, 2)),
  m_externWidth(0.0),
	m_enableScord(false), m_scordature(0),
	m_accidAnim(0), m_flyAccid(0),
	m_index(0), m_selectableNotes(false),
	m_lowerStaffPos(0.0),
	m_isPianoStaff(false),
	m_upperLinePos(16.0),
	m_height(40.0),
	m_keySignature(0)
{
	m_lines[0] = 0;
	m_lowLines[0] = 0; // first array item points are all items exist or not
	setZValue(10);
  setAcceptHoverEvents(true);
// Clef
  Tclef cl = Tclef();
  m_clef = new TscoreClef(scene, this, cl);
  connect(m_clef, SIGNAL(clefChanged(Tclef)), this, SLOT(onClefChanged(Tclef)));
	m_clef->setZValue(55);
// Notes
  for (int i = 0; i < notesNr; i++) {
			m_notes << new Tnote(0, 0, 0);
      m_scoreNotes << new TscoreNote(scene, this, i);
      m_scoreNotes[i]->setPos(7.0 + i * m_scoreNotes[i]->boundingRect().width(), 0);
			m_scoreNotes[i]->setZValue(50);
      connect(m_scoreNotes[i], SIGNAL(noteWasClicked(int)), this, SLOT(onNoteClicked(int)));
  }
  
// Staff lines, it also sets m_width of staff
	prepareStaffLines();
	
  for (int i = 0; i < 7; i++)
    accidInKeyArray[i] = 0;
}


TscoreStaff::~TscoreStaff() {
	for (int i = 0; i < m_notes.size(); i++)
		delete m_notes[i];
	m_notes.clear();
}

//####################################################################################################
//########################################## PUBLIC ##################################################
//####################################################################################################

void TscoreStaff::setScoreControler(TscoreControl* scoreControl) {
	if (scoreControl) {
		m_scoreControl = scoreControl;
		connect(scoreControl, SIGNAL(accidButtonPressed(int)), this, SLOT(onAccidButtonPressed(int)));
	}
}


int TscoreStaff::noteToPos(const Tnote& note)	{
	int nPos = m_offset.octave * 7 + m_offset.note + upperLinePos() - 1 - (note.octave * 7 + (note.note - 1));
	if (isPianoStaff() && nPos > lowerLinePos() - 5)
		return nPos + 2;
	else
		return nPos;
}

		/** Calculation of note position works As folow:
		 * 1) expr: m_offset.octave * 7 + m_offset.note + upperLinePos() - 1 returns y position of note C in offset octave
		 * 2) (note.octave * 7 + (note.note - 1)) is number of note to be set.
		 * 3) Subtraction of them gives position of the note on staff with current clef and it is displayed 
		 * when this value is in staff scale. */
void TscoreStaff::setNote(int index, const Tnote& note) {
	if (index >= 0 && index < m_scoreNotes.size()) {
		if (note.note)
				m_scoreNotes[index]->setNote(noteToPos(note), (int)note.acidental);
		else
				m_scoreNotes[index]->setNote(0, 0);
		if (m_scoreNotes[index]->notePos()) // store note in the list
				*(m_notes[index]) = note;
		else
				*(m_notes[index]) = Tnote(0, 0, 0);
    if (note.note)
      setCurrentIndex(index);
	}
}


void TscoreStaff::insertNote(int index, const Tnote& note, bool disabled) {
	index = qBound(0, index, m_scoreNotes.size()); // 0 - adds at the begin, size() - adds at the end
	insert(index);
	if (index < m_scoreNotes.size())
			updateIndex();
	m_notes.insert(index, new Tnote());
	updateWidth();
// 	setNote(index, note);
	*(m_notes[index]) = note; // Do not set note 
	setNoteDisabled(index, disabled);
	updateSceneRect();
}


void TscoreStaff::insertNote(int index, bool disabled) {
	insertNote(index, Tnote(0, 0, 0), disabled);
}


void TscoreStaff::addNote(Tnote& note, bool disabled) {
	insertNote(m_scoreNotes.size(), note, disabled);
}


void TscoreStaff::removeNote(int index) {
	if (index >= 0 && index < m_scoreNotes.size()) {
		delete m_scoreNotes[index];
		m_scoreNotes.removeAt(index);
		delete m_notes[index];
		m_notes.removeAt(index);
		updateIndex();
		updateWidth();
		updateSceneRect();
	}
}


void TscoreStaff::updateSceneRect() {
	QRectF scRec = mapToScene(boundingRect()).boundingRect();
	scene()->setSceneRect(0.0, 0.0, scRec.width() + (isPianoStaff() ? 2.0 : 1.0), scRec.height());
}


void TscoreStaff::setNoteDisabled(int index, bool isDisabled) {
	if (index >=0 && index < m_scoreNotes.size())
		m_scoreNotes[index]->setReadOnly(isDisabled);
}


void TscoreStaff::setEnableKeySign(bool isEnabled) {
	if (isEnabled != (bool)m_keySignature) {
		if (isEnabled) {
			m_keySignature = new TscoreKeySignature(scoreScene(), this);
			m_keySignature->setPos(7.0, 0.0);
			m_keySignature->setClef(m_clef->clef());
			m_keySignature->setZValue(30);
			connect(m_keySignature, SIGNAL(keySignatureChanged()), this, SLOT(onKeyChanged()));
			m_flyAccid = new QGraphicsSimpleTextItem;
			registryItem(m_flyAccid);
			m_flyAccid->setFont(TnooFont(5));
			m_flyAccid->setScale(TscoreNote::accidScale());
			m_flyAccid->hide();
			if (m_scoreNotes.size())				
					m_flyAccid->setBrush(m_scoreNotes[0]->mainNote()->brush());
			m_accidAnim = new TcombinedAnim(m_flyAccid, this);
			connect(m_accidAnim, SIGNAL(finished()), this, SLOT(accidAnimFinished()));
			m_accidAnim->setDuration(300);
			m_accidAnim->setScaling(m_flyAccid->scale(), m_flyAccid->scale() * 3.0);
// 			m_accidAnim->scaling()->setEasingCurveType(QEasingCurve::OutQuint);
			m_accidAnim->setMoving(QPointF(), QPointF()); // initialize moving
			m_accidAnim->moving()->setEasingCurveType(QEasingCurve::OutBack);
			for (int i = 0; i < m_scoreNotes.size(); i++) {
				connect(m_scoreNotes[i], SIGNAL(fromKeyAnim(QString,QPointF,int)), this, SLOT(fromKeyAnimSlot(QString,QPointF,int)));
				connect(m_scoreNotes[i], SIGNAL(toKeyAnim(QString,QPointF,int)), this, SLOT(toKeyAnimSlot(QString,QPointF,int)));
				connect(m_accidAnim, SIGNAL(finished()), m_scoreNotes[i], SLOT(keyAnimFinished()));
			}
			if (m_scoreControl && !m_scoreControl->isEnabled()) {
					/** This is in case when score/staff is disabled and key signature is added.
					 * TscoreControl::isEnabled() determines availableness state. */
					m_keySignature->setReadOnly(true);
					m_keySignature->setAcceptHoverEvents(false);
			}
		} else {
					delete m_keySignature;
					m_keySignature = 0;
					m_accidAnim->deleteLater();
					m_accidAnim = 0;
					delete m_flyAccid;
					m_flyAccid = 0;
		}
		updateWidth();
	}
}


void TscoreStaff::setScordature(Ttune& tune) {
	if (!hasScordature()) {
		m_scordature = new TscoreScordature(scoreScene(), this);
		m_scordature->setParentItem(this);
		m_scordature->setZValue(35); // above key signature
	}
	m_scordature->setTune(tune);
	if (m_scordature->isScordatured())	{ 
			m_enableScord = true;
	} else { // nothing to show - standard tune
			delete m_scordature;
			m_scordature = 0;
			m_enableScord = false;
	}
	updateWidth();
}


void TscoreStaff::removeScordatute() {
	delete m_scordature; 
	m_scordature = 0; 
	m_enableScord = false;
	updateWidth();	
}


void TscoreStaff::setDisabled(bool disabled) {
	scoreClef()->setReadOnly(disabled);
	if (scoreKey()) {
		scoreKey()->setAcceptHoverEvents(!disabled); // stops displaying status tip
		scoreKey()->setReadOnly(disabled);
	}
	for (int i = 0; i < m_scoreNotes.size(); i++) {
		m_scoreNotes[i]->setReadOnly(disabled);
		m_scoreNotes[i]->hideWorkNote();
	}
}


QRectF TscoreStaff::boundingRect() const {
  return QRectF(0, 0, m_width, m_height);
}


int TscoreStaff::accidNrInKey(int noteNr, char key) {
	int accidNr;
	switch ((56 + notePosRelatedToClef(noteNr, m_offset)) % 7 + 1) {
		case 1: accidNr = 1; break;
		case 2: accidNr = 3; break;
		case 3: accidNr = 5; break;
		case 4: accidNr = 0; break;
		case 5: accidNr = 2; break;
		case 6: accidNr = 4; break;
		case 7: accidNr = 6; break;
	}
	if (key < 0)
		accidNr = 6 - accidNr;
	return accidNr;
}


void TscoreStaff::setPianoStaff(bool isPiano) {
	if (isPiano != m_isPianoStaff) {
		m_isPianoStaff = isPiano;
		if (isPiano) {
				m_upperLinePos = 14.0;
				m_lowerStaffPos = 28.0;
				m_height = 46.0;
		} else {
				m_upperLinePos = 16.0;
				m_lowerStaffPos = 0.0;
				m_height = 40.0;
		}
		prepareStaffLines();
		for (int i = 0; i < count(); i++)
			noteSegment(i)->adjustSize();
		TscoreNote::adjustCursor();
		emit pianoStaffSwitched();
	}
}


//##########################################################################################################
//########################################## PROTECTED   ###################################################
//##########################################################################################################

void TscoreStaff::prepareStaffLines() {
	if (!m_lines[0]) // create main staff lines
			for (int i = 0; i < 5; i++) {
				m_lines[i] = new QGraphicsLineItem();
				registryItem(m_lines[i]);
				m_lines[i]->setPen(QPen(qApp->palette().text().color(), 0.15));
				m_lines[i]->setZValue(5);
			}
	if (isPianoStaff()) {
		if (!m_lowLines[0]) { // create lower staff lines
			for (int i = 0; i < 5; i++) {
				m_lowLines[i] = new QGraphicsLineItem();
				registryItem(m_lowLines[i]);
				m_lowLines[i]->setPen(QPen(qApp->palette().text().color(), 0.15));
				m_lowLines[i]->setZValue(5);
			}
			createBrace();
		}
	} else {
		if (m_lowLines[0]) { // delete lower staff lines
			for (int i = 0; i < 5; i++)
				delete m_lowLines[i];
			m_lowLines[0] = 0;
			delete m_brace;
		}
	}
	updateWidth();
}


void TscoreStaff::insert(int index) {
	TscoreNote *newNote = new TscoreNote(scoreScene(), this, index);
	newNote->setZValue(50);
	connect(newNote, SIGNAL(noteWasClicked(int)), this, SLOT(onNoteClicked(int)));
	m_scoreNotes.insert(index, newNote);
}


void TscoreStaff::setEnableScordtature(bool enable) {
	if (enable != m_enableScord) {
		m_enableScord = enable;
		updateWidth();
	}
}


int TscoreStaff::fixNotePos(int pianoPos) {
	if (isPianoStaff() && pianoPos > lowerLinePos() - 4)
		return pianoPos - 2; // piano staves gap
	else
		return pianoPos;
}


//##########################################################################################################
//####################################### PUBLIC SLOTS     #################################################
//##########################################################################################################

void TscoreStaff::setCurrentIndex(int index) {
	if (m_selectableNotes) {
			if (currentIndex() != -1) { // unset the previous
				m_scoreNotes[currentIndex()]->setBackgroundColor(-1);
				m_scoreNotes[currentIndex()]->selectNote(false);
			}
			m_index = index;
			if (currentIndex() != -1) {
				m_scoreNotes[currentIndex()]->setBackgroundColor(qApp->palette().highlight().color());
				m_scoreNotes[currentIndex()]->selectNote(true);
			}
	}
}


void TscoreStaff::onClefChanged(Tclef clef) {
	setPianoStaff(clef.type() == Tclef::e_pianoStaff);
	switch(clef.type()) {
    case Tclef::e_treble_G:
      m_offset = TnoteOffset(3, 2); break;
    case Tclef::e_treble_G_8down:
      m_offset = TnoteOffset(3, 1); break;
    case Tclef::e_bass_F:
      m_offset = TnoteOffset(5, 0); break;
    case Tclef::e_bass_F_8down:
      m_offset = TnoteOffset(5, -1); break;
    case Tclef::e_alto_C:
      m_offset = TnoteOffset(4, 1); break;
    case Tclef::e_tenor_C:
      m_offset = TnoteOffset(2, 1); break;
		case Tclef::e_pianoStaff:
      m_offset = TnoteOffset(3, 2); break;
		default: break;
  }
  scoreClef()->setClef(clef);
  if (m_keySignature)
      m_keySignature->setClef(m_clef->clef());
	if (m_scoreNotes.size()) {
			for (int i = 0; i < m_scoreNotes.size(); i++) {
				if (m_scoreNotes[i]->notePos()) {
						setNote(i, *(m_notes[i]));
				} 
			}
	}
	emit clefChanged(scoreClef()->clef());
}


void TscoreStaff::noteChangedAccid(int accid) {
	if (m_scoreControl) {
			m_scoreControl->setAccidental(accid);
	}
}

//##########################################################################################################
//####################################### PROTECTED SLOTS  #################################################
//##########################################################################################################

void TscoreStaff::onPianoStaffChanged(Tclef clef) {
	setPianoStaff(clef.type() == Tclef::e_pianoStaff);
	scoreClef()->setClef(clef);
}


void TscoreStaff::onKeyChanged() {
  for (int i = 0; i < m_scoreNotes.size(); i++) {
    if (m_scoreNotes[i]->notePos())
        m_scoreNotes[i]->moveNote(m_scoreNotes[i]->notePos());
  }
}


void TscoreStaff::onNoteClicked(int noteIndex) {
  int globalNr = notePosRelatedToClef(fixNotePos(m_scoreNotes[noteIndex]->notePos())
				+ m_scoreNotes[noteIndex]->ottava() * 7, m_offset);
	m_notes[noteIndex]->note = (char)(56 + globalNr) % 7 + 1;
	m_notes[noteIndex]->octave = (char)(56 + globalNr) / 7 - 8;
	m_notes[noteIndex]->acidental = (char)m_scoreNotes[noteIndex]->accidental();
	setCurrentIndex(noteIndex);
	qDebug() << m_notes[noteIndex]->toText();
	emit noteChanged(noteIndex);
}


void TscoreStaff::onAccidButtonPressed(int accid) {
	scoreScene()->setCurrentAccid(accid);
	/** It is enough to do this as long as every TscoreNote handles mouseHoverEvent
	 * which checks value set above and changes accidental symbol if necessary. */
}


void TscoreStaff::fromKeyAnimSlot(QString accidText, QPointF accidPos, int notePos) {
	m_flyAccid->setText(accidText);
	m_accidAnim->setMoving(mapFromScene(m_keySignature->accidTextPos(accidNrInKey(notePos, scoreKey()->keySignature()))),
												 mapFromScene(accidPos));
	m_accidAnim->startAnimations();
	m_flyAccid->show();
}


void TscoreStaff::toKeyAnimSlot(QString accidText, QPointF accidPos, int notePos) {
	m_flyAccid->setText(accidText);
	m_accidAnim->setMoving(mapFromScene(accidPos),
												 mapFromScene(m_keySignature->accidTextPos(accidNrInKey(notePos, scoreKey()->keySignature()))));
	m_accidAnim->startAnimations();
	m_flyAccid->show();
}


void TscoreStaff::accidAnimFinished() {
	m_flyAccid->hide();
}

//##########################################################################################################
//########################################## PRIVATE     ###################################################
//##########################################################################################################

void TscoreStaff::updateIndex() {
	m_index = -1;
	for (int i = 0; i < m_scoreNotes.size(); i++) {
		m_scoreNotes[i]->changeIndex(i); // Update index of next notes in the list
		if (m_scoreNotes[i]->isSelected())
			m_index = i;
	}
}



void TscoreStaff::updateWidth() {
	qreal off = 0.0;
	if (m_keySignature)
			off = KEY_WIDTH + 1.5;
	else if (m_enableScord)
			off = KEY_WIDTH / 2;
	if (m_scoreNotes.size())
			m_width = 10.0 + off + m_scoreNotes.size() * m_scoreNotes[0]->boundingRect().width() + 2.0;
	else
			m_width = 10.0 + off + 2.0;
	if (m_externWidth > m_width)
		m_width = m_externWidth;
	
	for (int i = 0; i < m_scoreNotes.size(); i++) // update positions of the notes
				m_scoreNotes[i]->setPos(7.0 + off + i * m_scoreNotes[0]->boundingRect().width(), 0);
	for (int i = 0; i < 5; i++) { // adjust staff lines length
			m_lines[i]->setLine(1, upperLinePos() + i * 2, width() - 2, upperLinePos() + i * 2);
			if (isPianoStaff())
				m_lowLines[i]->setLine(1, lowerLinePos() + i * 2, width() - 2, lowerLinePos() + i * 2);
	}
// 	if (!scene()->views().isEmpty())
// 		scoreScene()->setSceneRect(0.0, 0.0, 
// 									width() * scene()->views()[0]->transform().m11(), height() * scene()->views()[0]->transform().m11());
	emit staffSizeChanged();
// 	scoreScene()->update();
}


void TscoreStaff::createBrace() {
	m_brace = new QGraphicsSimpleTextItem();
	registryItem(m_brace);
	QFont ff = QFont("nootka");
#if defined (Q_OS_MAC)
    ff.setPointSizeF(27.5);
#else
  ff.setPointSizeF(25.5);
#endif
  QFontMetrics fm(ff);
  ff.setPointSizeF(ff.pointSizeF() * (ff.pointSizeF() / fm.boundingRect(QChar(0xe16c)).height()));
  m_brace->setFont(ff);
	m_brace->setText(QString(QChar(0xe16c)));
#if defined (Q_OS_MAC)
    qreal distance = lowerLinePos() + 8.3 - upperLinePos();
    qreal fact = (distance + 1.3) / brace->boundingRect().height();
#elif defined (Q_OS_WIN)
    qreal distance = lowerLinePos() + 7 - upperLinePos();
    qreal fact = (distance + 1.8) / brace->boundingRect().height();
#else
	qreal distance = lowerLinePos() + 8 - upperLinePos();
	qreal fact = (distance + 0.2) / m_brace->boundingRect().height();
#endif
	m_brace->setScale(fact);
	m_brace->setBrush(qApp->palette().text().color());
	m_brace->setPos(-2.0, upperLinePos() + distance / 2 - (m_brace->boundingRect().height() * m_brace->scale()) / 2 + 0.4);
}







