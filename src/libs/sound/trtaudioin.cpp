/***************************************************************************
 *   Copyright (C) 2011-2015 by Tomasz Bojczuk                             *
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

#include "trtaudioin.h"
#include "tpitchfinder.h"
#include "taudioobject.h"
#include "taudioparams.h"
#include <QDebug>


/*static */
QStringList TaudioIN::getAudioDevicesList() {
    QStringList devList;
    createRtAudio();
    int devCnt = rtDevice()->getDeviceCount();
    if (devCnt < 1)
        return devList;
    for (int i = 0; i < devCnt; i++) {
        RtAudio::DeviceInfo devInfo;
        try {
          devInfo = rtDevice()->getDeviceInfo(i);
        }
        catch (RtAudioError& e) {
          qDebug() << "error when probing input device" << i;
          continue;
        }
        if (devInfo.probed && devInfo.inputChannels > 0)
          devList << QString::fromLocal8Bit(devInfo.name.data());
    }
    if (getCurrentApi() == RtAudio::LINUX_ALSA && !devList.isEmpty())
				devList.prepend("ALSA default");
    return devList;
}


bool TaudioIN::inCallBack(void* inBuff, unsigned int nBufferFrames, const RtAudioStreamStatus& status) {
		if (m_goingDelete || instance()->isStoped())
				return true;
    if (status)
        qDebug() << "Stream over detected!";
    qint16 *in = (qint16*)inBuff;
		qint16 value;
    for (int i = 0; i < nBufferFrames; i++) {
					value = *(in + i);
					instance()->m_pitch->fillBuffer(float(value) / 32768.0f);
    }
    return false;
}

TaudioIN*        			TaudioIN::m_instance = 0;
bool                  TaudioIN::m_goingDelete = false;

//------------------------------------------------------------------------------------
//------------          constructor     ----------------------------------------------
//------------------------------------------------------------------------------------
TaudioIN::TaudioIN(TaudioParams* params, QObject* parent) :
    QObject(parent),
    TrtAudio(params, e_input, inCallBack),
    m_pitch(0),
    m_volume(0.0),
    m_state(e_stopped),
    m_stoppedByUser(false),
    m_loPitch(15), m_hiPitch(140),
    m_noteWasStarted(false),
    m_currentRange(1)
{
  if (m_instance) {
    qDebug() << "Nothing of this kind... TaudioIN already exist!";
    return;
  }
  m_instance = this;
  m_pitch = new TpitchFinder();
  setAudioInParams();
	m_goingDelete = false;
  
	connect(m_pitch, &TpitchFinder::noteStarted, this, &TaudioIN::noteStartedSlot);
	connect(m_pitch, &TpitchFinder::noteFinished, this, &TaudioIN::noteFinishedSlot);
	connect(m_pitch, &TpitchFinder::pitchInChunk, this, &TaudioIN::pitchInChunkSlot);
	connect(m_pitch, &TpitchFinder::volume, this, &TaudioIN::volumeSlot);
	connect(ao(), &TaudioObject::paramsUpdated, this, &TaudioIN::updateSlot);
}

TaudioIN::~TaudioIN()
{
	m_goingDelete = true;
  closeStream();
	m_pitch->blockSignals(true);
	m_pitch->deleteLater();
  m_instance = 0;
  deleteInParams();
  resetCallBack();
}

//------------------------------------------------------------------------------------
//------------          methods         ----------------------------------------------
//------------------------------------------------------------------------------------

void TaudioIN::setAudioInParams() {
  setDetectionMethod(audioParams()->detectMethod);
	setMinimalVolume(audioParams()->minimalVol);
	m_pitch->setMinimalDuration(audioParams()->minDuration);

	m_pitch->setSampleRate(sampleRate(), m_currentRange); // framesPerChunk is determined here
	m_volume = 0.0;
//   qDebug() << "setAudioInParams" << sampleRate() << audioParams()->detectMethod << audioParams()->minDuration << audioParams()->minimalVol;
}


void TaudioIN::setMinimalVolume(float minVol) {
	m_pitch->setMinimalVolume(minVol);
	audioParams()->minimalVol = minVol;
}


/** Range of notes is increased one note down and up.
 * This 46 and 48 are its sign. 
 * Normally 47 is offset of midi note to Nootka Tnote. */
void TaudioIN::setAmbitus(Tnote loNote, Tnote hiNote) {
	m_loPitch = loNote.chromatic() + 46;
	m_hiPitch = hiNote.chromatic() + 48;
	m_loNote = loNote;
	m_hiNote = hiNote;
	TpitchFinder::Erange range = TpitchFinder::e_middle;
	if (loNote.chromatic() > Tnote(6, 0, 0).chromatic())
		range = TpitchFinder::e_high;
	else if (loNote.chromatic() > Tnote(5, -2, 0).chromatic())
		range = TpitchFinder::e_middle;
	else
		range = TpitchFinder::e_low;
	if ((int)range != m_currentRange) {
		m_currentRange = (int)range;
		bool isStop = isStoped();
		stopListening();
		m_pitch->setSampleRate(m_pitch->aGl()->rate, m_currentRange);
		if (!isStop)
			startListening();
	}
// 	qDebug() << "Ambitus set to:" << loNote.toText() << "--" << hiNote.toText();
}


void TaudioIN::setDetectionMethod(int method) {
	method = qBound<int>(0, method, 2);
	m_pitch->aGl()->analysisType = EanalysisModes(method);
	audioParams()->detectMethod = method;
}


//------------------------------------------------------------------------------------
//------------          slots       --------------------------------------------------
//------------------------------------------------------------------------------------
void TaudioIN::startListening() {
	if (!streamParams()) {
			qDebug() << "Can not start listening due to uninitialized input";
			return;
	} else
// 		qDebug() << "startListening";
	m_volume = 0.0;
	if (!m_stoppedByUser && startStream())
    setState(e_listening);
}


void TaudioIN::stopListening() {
	m_volume = 0.0;
	m_LastChunkPitch = 0.0;
  setState(e_stopped);
	m_pitch->resetFinder();
}



void TaudioIN::pitchInChunkSlot(float pitch) {
	if (isPaused())
			return;
  if (pitch == 0.0)
			m_LastChunkPitch = 0.0;
  else
			m_LastChunkPitch = pitch - audioParams()->a440diff;
}


void TaudioIN::noteStartedSlot(qreal pitch, qreal freq, qreal duration) {
	if (!isPaused()) {
			m_lastNote.set(pitch - audioParams()->a440diff, freq, duration);
			if (inRange(m_lastNote.pitchF)) {
				m_noteWasStarted = true;
				emit noteStarted(m_lastNote);
			}
  } else
			m_lastNote.set(); // reset last detected note structure
}


void TaudioIN::noteFinishedSlot(TnoteStruct* lastNote) {
	m_noteWasStarted = false;
	if (!isPaused()) {
			m_lastNote.set(lastNote->pitchF - audioParams()->a440diff, lastNote->freq, lastNote->duration);
			if (inRange(m_lastNote.pitchF)) {
				emit noteFinished(m_lastNote);
			}
  } else 
			m_lastNote.set(); // reset last detected note structure
}








