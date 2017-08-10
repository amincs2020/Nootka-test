/***************************************************************************
 *   Copyright (C) 2011-2017 by Tomasz Bojczuk                             *
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

#include "tsound.h"
#if defined (Q_OS_ANDROID)
  #include "tqtaudioin.h"
  #include "tqtaudioout.h"
#else
  #include "tmidiout.h"
  #include "trtaudioout.h"
  #include "trtaudioin.h"
#endif
#include "ttickcolors.h"
#include <tprecisetimer.h>
#include <tinitcorelib.h>
#include <taudioparams.h>
#include "music/tmelody.h"
#include "music/tchunk.h"

#include <QtQml/qqmlengine.h>
#include <QtCore/qdebug.h>


/* static */
Tsound* Tsound::m_instance = nullptr;


#define INT_FACTOR (1.2)

Tsound::Tsound(QObject* parent) :
  QObject(parent),
  player(0),
  sniffer(0),
  m_examMode(false),
  m_melodyNoteIndex(-1),
  m_tempo(60),
  m_quantVal(6)
{
  if (m_instance) {
    qDebug() << "Tsound instance already exists!";
    return;
  }

  m_instance = this;
  qRegisterMetaType<Tchunk>("Tchunk");
  qRegisterMetaType<TnoteStruct>("TnoteStruct");
  qmlRegisterType<TtickColors>("Nootka", 1, 0, "TtickColors");
#if !defined (Q_OS_ANDROID) && (defined (Q_OS_LINUX) || defined (Q_OS_WIN))
  TrtAudio::initJACKorASIO(GLOB->A->JACKorASIO);
#endif
  if (GLOB->A->OUTenabled)
      createPlayer();
  else
      player = 0;
  if (GLOB->A->INenabled) {
      createSniffer();
  } else {
      sniffer = 0;
  }

  QTimer::singleShot(1000, [=]{ sniffer->startListening(); });
}

Tsound::~Tsound()
{ //They have not a parent
  deleteSniffer();
  deletePlayer();
  m_instance = nullptr;
}

//#################################################################################################
//###################                PUBLIC            ############################################
//#################################################################################################

void Tsound::play(const Tnote& note) {
  bool playing = false;
  if (player && note.note)
      playing = player->play(note.chromatic());
#if defined (Q_OS_ANDROID)
  if (playing) {
    if (sniffer) { // stop sniffer if midi output was started
      if (!m_stopSniffOnce) { // stop listening just once
        sniffer->stopListening();
        m_stopSniffOnce = true;
      }
    }
  }
#else
  if (playing && !GLOB->A->playDetected && player->type() == TabstractPlayer::e_midi) {
    if (sniffer) { // stop sniffer if midi output was started
      if (!m_stopSniffOnce) { // stop listening just once
        sniffer->stopListening();
        m_stopSniffOnce = true;
      }
    }
  }
#endif
}


void Tsound::playMelody(Tmelody* mel) {
  if (m_melodyNoteIndex > -1)
    m_melodyNoteIndex = m_playedMelody->length();
  else {
    m_melodyNoteIndex = 0;
    m_playedMelody = mel;
  }
  playMelodySlot();
}


qreal Tsound::inputVol() {
  return sniffer ? sniffer->volume() : 0.0;
}


qreal Tsound::pitchDeviation() {
  if (sniffer)
    return static_cast<qreal>(qBound(-0.49, (sniffer->lastChunkPitch() - static_cast<float>(qRound(sniffer->lastChunkPitch()))) * INT_FACTOR, 0.49));
  else
    return 0.0;
}


void Tsound::acceptSettings() {
  bool doParamsUpdated = false;
  // for output
  if (GLOB->A->OUTenabled) {
    if (!player)
        createPlayer();
    else {
      #if !defined (Q_OS_ANDROID)
        if (GLOB->A->midiEnabled) {
          deletePlayer(); // it is safe to delete midi
          createPlayer(); // and create it again
        } else
      #endif
        { // avoids deleting TaudioOUT instance and loading ogg file every acceptSettings call
          if (player->type() == TabstractPlayer::e_midi) {
              deletePlayer(); // player was midi so delete
              createPlayer();
          } else { // just set new params to TaudioOUT
              doParamsUpdated = true;
          }
        }
        if (player) {
          if (!player->isPlayable())
            deletePlayer();
        }
    }
  } else {
      deletePlayer();
  }
  // for input
  if (GLOB->A->INenabled) {
    if (!sniffer) {
      createSniffer();
//       m_pitchView->setAudioInput(sniffer);
    } else {
//       m_userState = sniffer->stoppedByUser();
      setDefaultAmbitus();
      doParamsUpdated = true;
    }
//     m_pitchView->setMinimalVolume(GLOB->A->minimalVol);
//     m_pitchView->setIntonationAccuracy(GLOB->A->intonation);
  } else {
    if (sniffer)
      deleteSniffer();
  }
#if defined (Q_OS_ANDROID)
  if (player)
    static_cast<TaudioOUT*>(player)->setAudioOutParams();
  if (sniffer)
    sniffer->updateAudioParams();
#else
  if (doParamsUpdated) {
      if (player && player->type() == TabstractPlayer::e_audio) {
          static_cast<TaudioOUT*>(player)->updateAudioParams();
      } else if (sniffer)
          sniffer->updateAudioParams();
  }
#endif
  if (sniffer) {
    restoreSniffer();
  }
}


void Tsound::prepareToConf() {
  if (player) {
    player->stop();
#if !defined (Q_OS_ANDROID)
    player->deleteMidi();
#endif
  }
  if (sniffer) {
    m_userState = sniffer->stoppedByUser(); // m_pitchView->isPaused();
    sniffer->stopListening();
//     m_pitchView->setDisabled(true);
    blockSignals(true);
    sniffer->setStoppedByUser(false);
  }
}


void Tsound::restoreAfterConf() {
#if !defined (Q_OS_ANDROID)
  if (GLOB->A->midiEnabled) {
    if (player)
      player->setMidiParams();
  }
#endif
  if (sniffer)
    restoreSniffer();
}


float Tsound::pitch() {
  if (sniffer)
    return sniffer->lastNotePitch();
  else
    return 0.0f;
}


void Tsound::setTempo(int t) {
  if (t != m_tempo && t > 39 && t < 181) {
    m_tempo = t;
    emit tempoChanged();
  }
}


/**
 * @p m_quantVal is expressed in @p Trhythm duration of: Sixteenth triplet -> 4 or just Sixteenth -> 6 or Eighth -> 12
 */
void Tsound::setQuantization(int q) {
  if ((q == 4 || q == 6 || q == 12) != m_quantVal) {
    m_quantVal = q;
  }
}


bool Tsound::stoppedByUser() const {
  return sniffer ? sniffer->stoppedByUser() : false;
}


void Tsound::setStoppedByUser(bool sbu) {
  if (sniffer && sniffer->stoppedByUser() != sbu) {
    sniffer->setStoppedByUser(sbu);
    if (sbu)
      stopListen();
    else
      startListen();
    emit stoppedByUserChanged();
  }
}


bool Tsound::listening() const {
  return sniffer ? sniffer->detectingState() == TcommonListener::e_detecting : false;
}


void Tsound::stopListen() {
  if (sniffer)
    sniffer->stopListening();
}


void Tsound::startListen() {
  if (sniffer)
    sniffer->startListening();
}


void Tsound::prepareAnswer() {
//   m_pitchView->setBgColor(GLOB->EanswerColor);
//   m_pitchView->setDisabled(false);
}


void Tsound::pauseSinffing() {
  if (sniffer)
      sniffer->pause();
}


void Tsound::unPauseSniffing() {
  if (sniffer)
      sniffer->unPause();
}

bool Tsound::isSnifferPaused() {
  if (sniffer)
      return sniffer->isPaused();
  else
      return false;
}


bool Tsound::isSniferStopped() {
  return sniffer ? sniffer->isStoped() : true;
}


void Tsound::restoreAfterAnswer() {
//   m_pitchView->setBgColor(Qt::transparent);
//   m_pitchView->setDisabled(true);
}


void Tsound::prepareToExam(Tnote loNote, Tnote hiNote) {
  m_examMode = true;
  if (sniffer) {
//      m_pitchView->setDisabled(true);
     m_prevLoNote = sniffer->loNote();
     m_prevHiNote = sniffer->hiNote();
     sniffer->setAmbitus(loNote, hiNote);
  }
}


void Tsound::restoreAfterExam() {
  m_examMode = false;
  if (sniffer) {
//     sniffer->setAmbitus(m_prevLoNote, m_prevHiNote); // acceptSettings() has already invoked setDefaultAmbitus()
//     m_pitchView->setDisabled(false);
    unPauseSniffing();
    startListen();
  }
}


void Tsound::stopPlaying() {
  if (player)
    player->stop();
  m_melodyNoteIndex = -1;
}


bool Tsound::isPlayable() {
  if (player)
   return true;
  else
    return false;
}


void Tsound::setDefaultAmbitus() {
  if (sniffer)
    sniffer->setAmbitus(Tnote(GLOB->loString().chromatic() - 5), // range extended about 4th up and down
                  Tnote(GLOB->hiString().chromatic() + GLOB->GfretsNumber + 5));
}


#if !defined (Q_OS_ANDROID)
void Tsound::setDumpFileName(const QString& fName) {
  if (sniffer && !GLOB->A->dumpPath.isEmpty())
    sniffer->setDumpFileName(fName);
}
#endif

//#################################################################################################
//###################                PRIVATE           ############################################
//#################################################################################################



void Tsound::createPlayer() {
#if defined (Q_OS_ANDROID)
  player = new TaudioOUT(GLOB->A);
  connect(player, SIGNAL(noteFinished()), this, SLOT(playingFinishedSlot()));
#else
  if (GLOB->A->midiEnabled) {
      player = new TmidiOut(GLOB->A);
      connect(player, SIGNAL(noteFinished()), this, SLOT(playingFinishedSlot()));
  } else
      player = new TaudioOUT(GLOB->A);
#endif
  m_stopSniffOnce = false;
}


void Tsound::createSniffer() {
#if !defined (Q_OS_ANDROID)
  if (TaudioIN::instance())
    sniffer = TaudioIN::instance();
  else
#endif
  sniffer = new TaudioIN(GLOB->A);
  setDefaultAmbitus();
//   sniffer->setAmbitus(Tnote(-31), Tnote(82)); // fixed ambitus bounded Tartini capacities
  connect(sniffer, &TaudioIN::noteStarted, this, &Tsound::noteStartedSlot);
  connect(sniffer, &TaudioIN::noteFinished, this, &Tsound::noteFinishedSlot);
  connect(sniffer, &TaudioIN::stateChanged, [=]{ emit listeningChanged(); });
  m_userState = false; // user didn't stop sniffing yet
}


void Tsound::deletePlayer() {
  if (player) {
    player->stop();
    delete player;
    player = 0;
  }
}


void Tsound::deleteSniffer() {
  delete sniffer;
  sniffer = 0;
}


void Tsound::restoreSniffer() {
  sniffer->setStoppedByUser(m_userState);
//   m_pitchView->setDisabled(false);
  blockSignals(false);
  sniffer->startListening();
}



//#################################################################################################
//###################            PRIVATE SLOTS         ############################################
//#################################################################################################


void Tsound::playingFinishedSlot() {
//   qDebug("playingFinished");
  if (!m_examMode && sniffer) {
    if (m_stopSniffOnce) {
      sniffer->startListening();
    }
    m_stopSniffOnce = false;
  }
  emit plaingFinished();
}


void Tsound::playMelodySlot() {
  if (m_melodyNoteIndex > -1 && m_melodyNoteIndex < m_playedMelody->length()) {
    play(m_playedMelody->note(m_melodyNoteIndex)->p());
    TpreciseTimer::singleShot(60000 / m_playedMelody->tempo(), this, SLOT(playMelodySlot()));
    m_melodyNoteIndex++;
  } else {
    m_melodyNoteIndex = -1;
    playingFinishedSlot();
  }
}


void Tsound::noteStartedSlot(const TnoteStruct& note) {
  m_detectedNote = note.pitch;
  emit noteStarted(m_detectedNote);
  emit noteStartedEntire(note);
  if (player && GLOB->instrument().type() != Tinstrument::NoInstrument && GLOB->A->playDetected)
    play(m_detectedNote);
}


void Tsound::noteFinishedSlot(const TnoteStruct& note) {
  m_detectedNote = note.pitch;
  qreal rFactor = 2500.0 / m_tempo;
  qreal dur = note.duration * 1000.0 / rFactor;
  int normDur = qRound(dur /  static_cast<qreal>(m_quantVal)) * m_quantVal;
  Trhythm r(normDur);
  qDebug() << "noteFinishedSlot" << note.duration * 1000 << dur << normDur;
  if (r.isValid()) {
      m_detectedNote.setRhythm(r);
      qDebug() << "Detected" << note.duration << normDur << m_detectedNote.toText() << m_detectedNote.rtm.string();
      emit noteFinished();
  } else {
      TrhythmList notes;
      Trhythm::resolve(normDur, notes);
      for (int n = 0; n < notes.count(); ++n) {
        Trhythm& rr = notes[n];
        if (n == 0)
            rr.setTie(Trhythm::e_tieStart);
        else if (n == notes.count() - 1)
            rr.setTie(Trhythm::e_tieEnd);
        else
          rr.setTie(Trhythm::e_tieCont);
        m_detectedNote.setRhythm(rr);
        qDebug() << "Detected" << note.duration << normDur << n << m_detectedNote.toText() << m_detectedNote.rtm.string();
        emit noteFinished();
      }
  }
  emit noteFinishedEntire(note);
  if (player && GLOB->instrument().type() == Tinstrument::NoInstrument && GLOB->A->playDetected)
    play(m_detectedNote);
}

