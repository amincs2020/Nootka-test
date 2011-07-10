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


#include "tfirstrunwizzard.h"
#include "taboutnootka.h"
#include "tglobals.h"
#include <QtGui>

extern Tglobals *gl;

/*static*/
QString TfirstRunWizzard::nextText = QObject::tr("next");

TfirstRunWizzard::TfirstRunWizzard(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    pagesLay = new QStackedLayout;
    lay->addLayout(pagesLay);

    QHBoxLayout *buttLay = new QHBoxLayout;
    buttLay->addStretch(1);
    skipButt = new QPushButton(tr("skip wizzard"), this);
    buttLay->addWidget(skipButt);
    buttLay->addStretch(2);
    prevButt = new QPushButton(tr("prev"), this);
    prevButt->setDisabled(true);
    buttLay->addWidget(prevButt);
    nextButt = new QPushButton(nextText, this);
    buttLay->addWidget(nextButt);
    buttLay->addStretch(1);

    lay->addLayout(buttLay);
    setLayout(lay);

    Tabout *aboutNoot = new Tabout();
    QLabel *notationLab = new QLabel(tr("Guitar notation uses ...<br><br><p style=\"vertical-align::::middle;\"> note <img src=\"%1\"> is the same as <img src=\"%2\"></p><br><br>").arg(gl->path+"picts/c1-trebe.png").arg(gl->path+"picts/c1-treble_8.png"));

    Tpage_3 *page3 = new Tpage_3();

    pagesLay->addWidget(aboutNoot);
    pagesLay->addWidget(notationLab);
    pagesLay->addWidget(page3);

    connect(skipButt, SIGNAL(clicked()), this, SLOT(close()));
    connect(prevButt, SIGNAL(clicked()), this, SLOT(prevSlot()));
    connect(nextButt, SIGNAL(clicked()), this, SLOT(nextSlot()));
}

void TfirstRunWizzard::prevSlot() {
    switch (pagesLay->currentIndex()) {
    case 0 :
        break;
    case 1 :
        prevButt->setDisabled(true);
        pagesLay->setCurrentIndex(0);
        break;
    case 2 :
        nextButt->setText(nextText);
        pagesLay->setCurrentIndex(1);
        break;
    }
}

void TfirstRunWizzard::nextSlot() {
    switch (pagesLay->currentIndex()) {
    case 0 :
        prevButt->setDisabled(false);
        pagesLay->setCurrentIndex(1);
        break;
    case 1 :
        nextButt->setText(tr("Finish"));
        pagesLay->setCurrentIndex(2);
        break;
    case 2 :
        close();
        break;
    }
}

//###############################################  Tpage_3 ###############################################

Tpage_3::Tpage_3(QWidget *parent) :
        QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *seventhLab = new QLabel(tr("7-th note can be ...<br><br>"), this);
    lay->addWidget(seventhLab, 1, Qt::AlignTop);
    lay->addStretch(1);

    QHBoxLayout *radioLay = new QHBoxLayout;
    radioLay->addStretch(1);
    isBradio = new QRadioButton("B", this);
    radioLay->addWidget(isBradio);
    isHradio = new QRadioButton("H", this);
    radioLay->addWidget(isHradio);
    radioLay->addStretch(1);
    QButtonGroup *gr7 = new QButtonGroup(this);
    gr7->addButton(isBradio);
    gr7->addButton(isHradio);
    isBradio->setChecked(true);
    lay->addLayout(radioLay);
    lay->addStretch(1);

    dblAccChB = new QCheckBox(tr("I know about double sharps (x) and double flats (bb)"), this);
    lay->addWidget(dblAccChB);
    lay->addStretch(1);

    enharmChB = new QCheckBox(tr("I know that e# is the same as f"), this);
    lay->addWidget(enharmChB);
    lay->addStretch(1);

    setLayout(lay);

}


