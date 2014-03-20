/***************************************************************************
 *   Copyright (C) 2013 -2014 by Tomasz Bojczuk                            *
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


#ifndef TDROPSHADOWEFFECT_H
#define TDROPSHADOWEFFECT_H

#include <nootkacoreglobal.h>
#include <QGraphicsEffect>

/** 
 * Shadow effect used for all tips 
*/
class NOOTKACORE_EXPORT TdropShadowEffect : public QGraphicsDropShadowEffect
{

public:
    TdropShadowEffect(QColor color = -1);
};

#endif // TDROPSHADOWEFFECT_H
