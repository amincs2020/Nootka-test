/***************************************************************************
 *   Copyright (C) 2016 by Tomasz Bojczuk                                  *
 *   seelook@gmail.com                                                     *
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

#ifndef TNOTEITEM_H
#define TNOTEITEM_H


#include <nootkacoreglobal.h>
#include "tscoreitem.h"


class Trhythm;


/**
 * Paints note head on the @class TscoreNote
 * and rhythm when it is enabled.
 * It paints steam and flag of whole and quarter notes
 * but eights and sixteenths have only heads - their stems are painted outside
 * It is controlled through @p setFlagsPaint() - @p FALSE forces painting flag always
 */
class NOOTKACORE_EXPORT TnoteItem : public TscoreItem
{

public:
  TnoteItem(TscoreScene* scene, const Trhythm& r);
  virtual ~TnoteItem();

  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
  virtual QRectF boundingRect() const;

  void setColor(const QColor& c) { m_color = c; }
  QColor color() { return m_color; }

      /** When sets to @p TRUE beaming of Trhythm is ignored and flags of 8 and 16 are painted.
       * This is for cursor note.
       * Otherwise, 8 and 16 have only heads */
  void setFlagsPaint(bool flagPaint) { m_paintFlags = flagPaint; }


  void setRhythm(const Trhythm& r);
  Trhythm* rhythm() const { return m_rhythm; } /**< Actual rhythm of a note */

private:
  void obtainNoteLetter(); /**< Common routine to get letter of actual rhythm value */
  bool upsideDown(const Trhythm& r);

private:
  QColor        m_color;
  Trhythm      *m_rhythm; /**< This is pointer of @class Trhythm - Note head and flags are determined by it */
  bool          m_paintFlags, m_upsideDown;
  QString       m_noteLetter; /**< single letter representing a note symbol in Nootka font */
};

#endif // TNOTEITEM_H
