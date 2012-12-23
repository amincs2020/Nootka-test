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

#include "tkeysignatureview.h"
#include "tscorewidgetsimple.h"
#include "tnoteview.h"
#include "tglobals.h"
#include "tkeysignature.h"
#include <QtGui>



extern Tglobals *gl;

/* static */
char TkeySignatureView::getPosOfAccid(int noteNr) {
    if (noteNr >= 0 && noteNr < 7)
        return m_posOfAccid[noteNr];
    else
        return 0;
}


char TkeySignatureView::m_posOfAccid[7] = {
    16, // Fes & Fis (F#)
    19, // Ces (C#)
    15, // Ges (G#)
    18, // Des (D#)
    21, // As  (A#)
    17, // ES  (E#)
    20 // B   (H#)  (Bb - B#) in west
};




TkeySignatureView::TkeySignatureView(TscoreWidgetSimple *parent, char _keySign) :
    QGraphicsView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet(("background: transparent"));
    setRenderHint(QPainter::TextAntialiasing, true);
    setMouseTracking(true);
    setStatusTip(tr("Use mouse wheel to change a key signature"));

    m_scene = new QGraphicsScene();
    setScene(m_scene);

    for (int i=0; i<7; i++) {
        m_accidentals[i] = new QGraphicsSimpleTextItem();
        m_accidentals[i]->setBrush(QBrush(palette().text().color()));
        m_scene->addItem(m_accidentals[i]);
        m_accidentals[i]->hide();
    }

    m_keyNameText = new QGraphicsSimpleTextItem();
    m_keyNameText->setBrush(QBrush(palette().text().color()));
    m_scene->addItem(m_keyNameText);

    m_flatKeyFuse = -7;
    m_sharpKeyFuse = 7;
    m_accInKeyPtr = parent->accInKeyArr;
    m_keySignature = _keySign;
    setKeySignature(_keySign);
    show();
}


void TkeySignatureView::resize(int co) {
    m_coeff = co;
    m_scene->setSceneRect(0, 0, 8*m_coeff, height());
    qreal fontFactor = 3.8;
 #if defined(Q_OS_MAC)
    fontFactor = fontFactor*1.4;
 #endif
    QFont font(QFont("nootka"));
    font.setPointSizeF(fontFactor * m_coeff);
    QFontMetricsF fMetrics(font);
    qreal fact = (qreal)(co * 1.3) / fMetrics.boundingRect(QChar(0xe11a)).width();
    font.setPointSize(qRound(font.pointSize() * fact));
    int base;
    if (m_accidentals[0]->text() == "")
        m_accidentals[0]->setText(QString(QChar(0xe11a)));
    m_accidentals[0]->setFont(font);
    m_accTextOffset = m_accidentals[0]->boundingRect().height()/2 /*- qRound(m_coeff * 0.5)*/;
    if (m_keySignature >= 0) base = 0;
    else base = 6;
    for (int i=0; i<7; i++) {
        m_accidentals[i]->setFont(font);
        m_accidentals[i]->setPos(i*m_coeff-1,
                    m_posOfAccid[qAbs(base-i)]*m_coeff-m_accTextOffset);
    }
    if (gl->SshowKeySignName) {
        m_keyNameText->setPos(1, 8*m_coeff);
    // All calculation below is to have enaught space for key name
        m_keyNameText->setFont(QFont(this->font().family(), m_coeff*4));
        m_keyNameText->hide();
        QString S = m_keyNameText->text();
        m_keyNameText->setText("Ges-" + gl->SminKeyNameSufix); //simple text to determine max width
        while (m_keyNameText->boundingRect().width() > width()) {
            QFont f = m_keyNameText->font();
            f.setPointSize(f.pointSize()-1);
            m_keyNameText->setFont(f);
        }
        m_keyNameText->setText(S);
        m_keyNameText->show();
    }
}

void TkeySignatureView::wheelEvent(QWheelEvent * event) {
    char prevKey = m_keySignature;
    int base;
    if (event->delta() > 0) {
        if (m_keySignature < m_sharpKeyFuse) m_keySignature++;
    }
    else {
        if (m_keySignature > m_flatKeyFuse) m_keySignature--;
    }
    if (m_keySignature != prevKey) {
        setKeySignature(m_keySignature);
    }

}

void TkeySignatureView::setKeySignature(char keySign) {
    for (int i = 1; i < 8; i++) {
        int base = 0;
        char sign = 1;
        if (keySign < 0) {
            base = 8;
            sign = -1;
        }
        if (i <= qAbs(keySign)) {// show accid
            m_accidentals[i-1]->setText(TnoteView::getAccid(sign));
            m_accidentals[i-1]->setPos( (i-1)*m_coeff+1,
                    qreal(m_posOfAccid[qAbs(base - i)-1]*m_coeff)-m_accTextOffset);
            *(m_accInKeyPtr+(26-m_posOfAccid[qAbs(base - i)-1])%7) = sign;
            m_accidentals[i-1]->show();
        }
        else { // hide
                m_accidentals[i-1]->hide();
                *(m_accInKeyPtr+(26-m_posOfAccid[qAbs(base - i)-1])%7) = 0;
            }
    }
    m_keySignature = keySign;
    showKeyName();
    emit keySignWasChanged();
}

void TkeySignatureView::showKeyName() {
    if (gl->SshowKeySignName) {
        m_keyNameText->setText(TkeySignature::getMajorName(m_keySignature) + "\n" +
                               TkeySignature::getMinorName(m_keySignature));
        if (m_keyNameText->boundingRect().width() > width()) {
            QFont f = m_keyNameText->font();
            f.setPointSize(f.pointSize() - 1);
            m_keyNameText->setFont(f);
        }
        m_keyNameText->show();
    }
    else m_keyNameText->hide();
}


/** @todo TkeySignatureView::setKeysFuse() 
 * Untill now it has no use...*/
