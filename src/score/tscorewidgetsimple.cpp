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

#include "tscorewidgetsimple.h"
#include <QPainter>
#include "tclefview.h"
#include "tnoteview.h"
#include "tkeysignatureview.h"
#include "tkeysignature.h"
#include <QtGui>
//#include <QDebug>



/** This is count of notes witch can be shown on the whole widget*/
const char _C = 36;

TscoreWidgetSimple::TscoreWidgetSimple(unsigned char _notesCount, QWidget *parent) :
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);
    setMinimumHeight(180);
    setBGcolor(palette().base().color());

    m_clef = new TclefView(this);

    for (int i=0; i<_notesCount; i++) {
        noteViews << new TnoteView(i,this);
        m_notes.push_back(Tnote());
    }
    keySignView = 0;

    m_dblSharpBut = new TpushButton("x", this);
    setButtons(m_dblSharpBut);
    m_sharpBut = new TpushButton("#", this);
    setButtons(m_sharpBut);
    m_flatBut = new TpushButton("b", this);
    setButtons(m_flatBut);
    m_dblFlatBut = new TpushButton("B", this);
    setButtons(m_dblFlatBut);
    QVBoxLayout *butLay = new QVBoxLayout;
    butLay->addStretch(1);
    butLay->addWidget(m_dblSharpBut);
    butLay->addWidget(m_sharpBut);
    butLay->addSpacing(5);
    butLay->addWidget(m_flatBut);
    butLay->addWidget(m_dblFlatBut);
    butLay->addStretch(1);
    QHBoxLayout *mainLay = new QHBoxLayout();
    mainLay->addLayout(butLay);
    mainLay->setAlignment(Qt::AlignRight);
    setLayout(mainLay);

    for (int i=0; i<7; i++) accInKeyArr[i]=0;
    setEnabledDblAccid(false);
    setHasScord(false);
    resize();

    connect(m_dblSharpBut,SIGNAL(clicked()),this,SLOT(onAcidButtonPressed()));
    connect(m_sharpBut,SIGNAL(clicked()),this,SLOT(onAcidButtonPressed()));
    connect(m_flatBut,SIGNAL(clicked()),this,SLOT(onAcidButtonPressed()));
    connect(m_dblFlatBut,SIGNAL(clicked()),this,SLOT(onAcidButtonPressed()));
    for (int i=0; i<noteViews.size(); i++) {
        connect(noteViews[i],SIGNAL(accidWasChanged(int)),this,SLOT(changeAccidButtonsState(int)));
        connect(noteViews[i],SIGNAL(noteWasClicked(int)),this,SLOT(noteWasClicked(int)));
    }

}

void TscoreWidgetSimple::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setWindow(0, 0, width(), height());

    painter.setPen(QPen(palette().color(palette().currentColorGroup(), QPalette::Text)));
//     QColor bg = palette().color(palette().currentColorGroup(), QPalette::Base); //.base().color();
//     bg.setAlpha(220);
//     painter.setBrush(QBrush(palette().color(palette().currentColorGroup(), QPalette::Base), Qt::SolidPattern));
    painter.setBrush(QBrush(m_bgColor));
    painter.drawRoundedRect(1, 1, width() - 55, height() - 2, coeff, coeff);

    for (int i = 16; i < 26; i += 2)
        painter.drawLine(5, (i * coeff), width() - 55, (i * coeff));
//    painter.setFont(QFont("nootka",coeff*12.5,QFont::Normal));
//    painter.drawText(QRect(1, qRound(12.2*coeff), coeff*6,coeff*18), Qt::AlignLeft, QString(QChar(0xe1a7)));
}

void TscoreWidgetSimple::resizeEvent(QResizeEvent *) {
    resize();
}

void TscoreWidgetSimple::resize() {
    coeff = geometry().height() / _C;
    m_clef->setGeometry(1, 0, 5.5 * coeff, height());
    m_clef->resize(coeff);
    int shift = 6 * coeff;
    if (m_hasScord)
//         if (shift < 85) shift = 85;
        shift = 12 * coeff;
    if (keySignView) {
        keySignView->setGeometry(5*coeff, 0, 8*coeff, height());
        keySignView->resize(coeff);
        shift = 14 * coeff;
    }
    for (int i = 0; i < noteViews.size(); i++) {
        noteViews[i]->setGeometry(shift + (i * 6) * coeff, 0, 6 * coeff, height());
        noteViews[i]->resize(coeff);
    }
}


void TscoreWidgetSimple::setButtons(QPushButton *button) {
    button->setFixedSize(40, 45);
#if defined(Q_OS_MAC)
    button->setFont(QFont("nootka", 25, QFont::Normal));
#else
    button->setFont(QFont("nootka", 20, QFont::Normal));
#endif
//     button->setCheckable(true);
}

void TscoreWidgetSimple::onAcidButtonPressed() {
	if (sender() != m_sharpBut)
		m_sharpBut->setChecked(false);
	if (sender() != m_dblSharpBut)
		m_dblSharpBut->setChecked(false);
	if (sender() != m_dblFlatBut)
		m_dblFlatBut->setChecked(false);
	if (sender() != m_flatBut)
		m_flatBut->setChecked(false);
    int ac;
	TpushButton *button = static_cast<TpushButton *>(sender());
	button->setChecked(!button->isChecked());
    if (sender() == m_flatBut) {
        if (m_flatBut->isChecked()) ac = -1;
        else ac = 0;
    } else {
        if (sender() == m_sharpBut) {
           if (m_sharpBut->isChecked()) ac = 1;
           else ac = 0;
       } else {
           if (sender() == m_dblFlatBut)   {
                if (m_dblFlatBut->isChecked()) ac = -2;
                else ac = 0;
            } else {
//                 
                if (m_dblSharpBut->isChecked()) ac = 2;
                else ac = 0;
            }
       }
      }
    for (int i=0; i<noteViews.size(); i++)
        noteViews[i]->setAccidText(ac);
}

void TscoreWidgetSimple::changeAccidButtonsState(int accNr) {
    m_dblSharpBut->setChecked(false);
    m_sharpBut->setChecked(false);
    m_flatBut->setChecked(false);
    m_dblFlatBut->setChecked(false);
    for (int i=0; i<noteViews.size(); i++)
        noteViews[i]->setAccidText(accNr);
    switch (accNr) {
    case -2: m_dblFlatBut->setChecked(true); break;
    case -1: m_flatBut->setChecked(true); break;
    case 1: m_sharpBut->setChecked(true); break;
    case 2: m_dblSharpBut->setChecked(true); break;
    }
}

void TscoreWidgetSimple::noteWasClicked(int index) {
    m_notes[index] = Tnote((39 - noteViews[index]->notePos())%7+1,(39 - noteViews[index]->notePos())/7 - 2,noteViews[index]->accidental());
    emit noteHasChanged(index, m_notes[index]);
}

void TscoreWidgetSimple::onKeySignatureChanged() {
    for (int i=0; i<noteViews.size(); i++) {
        if (noteViews[i]->notePos())
            noteViews[i]->moveNote(noteViews[i]->notePos());
    }
}

void TscoreWidgetSimple::setEnabledDblAccid(bool isEnabled) {
    if (isEnabled) {
        m_dblSharpBut->show();
        m_dblFlatBut->show();
        dblAccidFuse = 2;
    }
    else {
        m_dblSharpBut->hide();
        m_dblFlatBut->hide();
        dblAccidFuse = 1;
        changeAccidButtonsState(0);// set no accidental for all nottes & unset buttons
    }
}

void TscoreWidgetSimple::setEnableKeySign(bool isEnabled) {
    if (bool(keySignView) != isEnabled) {
        if (isEnabled) {
            keySignView = new TkeySignatureView(this);
            connect(keySignView, SIGNAL(keySignWasChanged()),
                    this,SLOT(onKeySignatureChanged()));
        }
        else {
            delete keySignView;
            keySignView = 0;
        }
        for (int i=0; i<7; i++) accInKeyArr[i]=0;
    }
    resize();
}

TkeySignature TscoreWidgetSimple::keySignature() {
    if (keySignView) return TkeySignature(keySignView->keySignature());
    else return TkeySignature(); // default is C-major
}

void TscoreWidgetSimple::refreshKeySignNameStyle() {
    if (keySignView) keySignView->showKeyName();
}

void TscoreWidgetSimple::setNote(int index, Tnote note) {
    m_notes[index] = note;
    if (note.note) {
        noteViews[index]->setNote(getNotePos(note), note.acidental);
    } else
        clearNote(index);
}

void TscoreWidgetSimple::clearNote(int index) {
    noteViews[index]->markNote(-1);
    noteViews[index]->hideNote();
    m_notes[index] = Tnote(0,0,0);
}

void TscoreWidgetSimple::setAmbitus(Tnote lo, Tnote hi, int index) {
    noteViews[index]->setAmbitus(getNotePos(lo)+1,getNotePos(hi));
}

void TscoreWidgetSimple::setAmbitus(Tnote lo, Tnote hi) {
    for (int i=0; i<noteViews.size(); i++)
        setAmbitus(lo,hi,i);
}

int TscoreWidgetSimple::getNotePos(Tnote note) {
    int np = 26 - (note.octave*7 + note.note);
    return np;
}

void TscoreWidgetSimple::setKeySignature(TkeySignature keySign) {
    if (keySignView)
        keySignView->setKeySignature(keySign.value());
}

void TscoreWidgetSimple::setScoreDisabled(bool disabled) {
    if (disabled) {
        m_sharpBut->setDisabled(true);
        m_flatBut->setDisabled(true);
        m_dblSharpBut->setDisabled(true);
        m_dblFlatBut->setDisabled(true);
        for (int i=0; i<noteViews.size(); i++)
            noteViews[i]->setDisabled(true);
        if (keySignView)
            keySignView->setDisabled(true);
        noteViews[0]->hideWorkNote();
    } else {
        m_sharpBut->setDisabled(false);
        m_flatBut->setDisabled(false);
        m_dblSharpBut->setDisabled(false);
        m_dblFlatBut->setDisabled(false);
        for (int i=0; i<noteViews.size(); i++)
            noteViews[i]->setDisabled(false);
        if (keySignView)
            keySignView->setDisabled(false);
    }
}


void TscoreWidgetSimple::setBGcolor(QColor bgColor) {
  m_bgColor = bgColor;
  m_bgColor.setAlpha(220);
  update();
}
