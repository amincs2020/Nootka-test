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
 *  You should have received a copy of the GNU General Public License	   *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef QAPPMAC_H
#define QAPPMAC_H


#include <QApplication>
#include <QFileOpenEvent>
#include <QEvent>
//#include <QtGui>


/** This class overrides standard QApplication to handle opening file.
 * MacOs doesn't support invokeing nootka with commandline args. */
class QAppMac : public QApplication
{

    Q_OBJECT

public:
    QAppMac(int argc, char *argv[]) : QApplication(argc, argv){}
// 	virtual ~QAppMac();
	
signals:
	void fileToOpen(QString fileName);
	
protected:
  bool event(QEvent *event);

};
#endif 
