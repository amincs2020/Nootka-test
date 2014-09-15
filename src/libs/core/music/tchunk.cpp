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

#include "tchunk.h"
#include "trhythm.h"
#include "tnote.h"
#include <QXmlStreamWriter>

Tchunk::Tchunk(const Tnote& pitch, const Trhythm& rhythm, const TfingerPos& fretPos) :
	m_pitch(pitch),
	m_rhythm(rhythm),
	m_fretPos(fretPos)
{
}


Tchunk::~Tchunk()
{}


void Tchunk::toXml(QXmlStreamWriter& xml) {
	xml.writeStartElement("note");
		if (m_rhythm.isRest() || !m_pitch.isValid())
			xml.writeEmptyElement("rest");
		else 
			m_pitch.toXml(xml);
		if (m_rhythm.rhythm() == Trhythm::e_none) {
			xml.writeTextElement("stem", "none");
		} else {
			xml.writeTextElement("type", m_rhythm.xmlType());
			if (m_rhythm.hasDot())
				xml.writeEmptyElement("dot");
		}
		xml.writeTextElement("duration", "1");
		if (validPos()) {
			xml.writeStartElement("notations");
				g().toXml(xml);
			xml.writeEndElement();
		}
	xml.writeEndElement(); // note
}


bool Tchunk::fromXml(QXmlStreamReader& xml) {
	bool ok = true;
		while (xml.readNextStartElement()) {
			if (xml.name() == "pitch")
				m_pitch.fromXml(xml);
			else if (xml.name() == "rest")
				m_rhythm.setRest(true);
			else if (xml.name() == "type")
				m_rhythm.setNoteValue(xml.readElementText().toStdString());
			else if (xml.name() == "notations")
				m_fretPos.fromXml(xml);
			else
				xml.skipCurrentElement();
		}
	return ok;
}






