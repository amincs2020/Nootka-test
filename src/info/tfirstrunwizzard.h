/***************************************************************************
 *   Copyright (C) 2011-2013 by Tomasz Bojczuk                  				   *
 *   tomaszbojczuk@gmail.com   						                                 *
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
 *  You should have received a copy of the GNU General Public License	     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/


#ifndef TFIRSTRUNWIZZARD_H
#define TFIRSTRUNWIZZARD_H

#include <QDialog>


class QStackedLayout;
class QRadioButton;
class QCheckBox;
class Tpage_2;
class Tpage_3;
class Tpage_4;
class Select7note;

class TfirstRunWizzard : public QDialog
{
    Q_OBJECT
public:
    explicit TfirstRunWizzard(QWidget *parent = 0);
    static QString nextText() { return tr("next"); }

signals:

public slots:

private:
    QPushButton 		*m_skipButt, *m_nextButt, *m_prevButt;
    QStackedLayout *m_pagesLay;
    Tpage_3 *page3;
    Tpage_4 *page4;
		Tpage_2 *page2;

private slots:
    void nextSlot();
    void prevSlot();


};

// page 1 is about dialog
// page 2 is a QLabel

class Tpage_3 : public QWidget
{
    Q_OBJECT
public:
    explicit Tpage_3(QWidget *parent = 0);
    
    static QString note7txt() { return tr("b", "Give here a name of 7-th note prefered in Your country. But only 'b' or 'h' not 'si' or something worst..."); } // returns b or h dependa on translation
    
    QString keyNameStyle; // default style grab from translation

    Select7note *select7;
    QCheckBox *dblAccChB, *enharmChB, *useKeyChB;

};

class Tpage_4 : public QWidget
{
    Q_OBJECT
public:
  explicit Tpage_4(QWidget *parent = 0);
  
  
};

#endif // TFIRSTRUNWIZZARD_H
