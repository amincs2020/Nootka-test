/***************************************************************************
 *   Copyright (C) 2011 by Tomasz Bojczuk                                  *
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


#include "audioinsettings.h"
#include "taudioin.h"
#include "tpitchview.h"
#include "tpitchfinder.h"
#include <QtGui>


AudioInSettings::AudioInSettings(QWidget* parent) :
  QWidget(parent),
  m_audioIn(0)
{
  QVBoxLayout *lay = new QVBoxLayout();
  
  enableInBox = new QGroupBox(tr("enable audio input (pitch detection)"), this);
  enableInBox->setCheckable(true);
  enableInBox->setChecked(true);
  
  QVBoxLayout *inLay = new QVBoxLayout();
  
  QHBoxLayout *upLay = new QHBoxLayout();  
  QVBoxLayout *devDetLay = new QVBoxLayout(); // device & detection method layout
  
  QLabel *devLab = new QLabel(tr("input device"), this);
  devDetLay->addWidget(devLab);
  inDeviceCombo = new QComboBox(this);
  devDetLay->addWidget(inDeviceCombo);
  QLabel *detectLab = new QLabel(tr("detection method"), this);
  devDetLay->addWidget(detectLab);
  detectMethodCombo = new QComboBox(this);
  devDetLay->addWidget(detectMethodCombo);
  detectMethodCombo->addItem("MPM");
  detectMethodCombo->addItem(tr("Autocorrelation"));
  detectMethodCombo->addItem(tr("MPM & modified cepstrum"));
  
  devDetLay->addStretch(1);
  loudChB = new QCheckBox(tr("low-pass filter"), this);
  loudChB->setChecked(true);
  devDetLay->addWidget(loudChB);
  voiceChB = new QCheckBox(tr("human voice"), this);
  voiceChB->setStatusTip(tr("Check this for singing."));
  devDetLay->addWidget(voiceChB);
//   noiseChB = new QCheckBox(tr("noise floor"), this);
//   noiseChB->setChecked(true);
//   devDetLay->addWidget(noiseChB);
  devDetLay->addStretch(1);
  
  upLay->addLayout(devDetLay);
  QVBoxLayout *tunLay = new QVBoxLayout(); //middle A & threshold layout
  
  midABox = new QGroupBox(tr("middle A")+" (a1)", this);
  QVBoxLayout *midLay = new QVBoxLayout();
  QLabel *frLab = new QLabel(tr("frequency:"), this);
  midLay->addWidget(frLab);
  freqSpin = new QSpinBox(this);
  midLay->addWidget(freqSpin);
  freqSpin->setMinimum(400);
  freqSpin->setMaximum(480);
  freqSpin->setValue(440);
  freqSpin->setSuffix(" Hz");
  
  QLabel *intLab = new QLabel(tr("interval:"), this);
  midLay->addWidget(intLab);
  intervalCombo = new QComboBox(this);
  midLay->addWidget(intervalCombo);
  intervalCombo->addItem(tr("semitone up"));
  intervalCombo->addItem(tr("none"));
  intervalCombo->addItem(tr("semitone down"));
  intervalCombo->setCurrentIndex(1);
  midABox->setLayout(midLay);
  tunLay->addWidget(midABox);
  
  noisGr = new QGroupBox(this);
  QVBoxLayout *noisLay = new QVBoxLayout();
  QLabel *threLab = new QLabel(tr("noise level:"), this);
  noisLay->addWidget(threLab, 1, Qt::AlignCenter);
  noiseSpin = new QDoubleSpinBox(this);
  noiseSpin->setMinimum(0.2);
  noiseSpin->setMaximum(98.0);
  noiseSpin->setDecimals(1);
  noiseSpin->setSingleStep(0.2);
  noiseSpin->setSuffix(" %");
  noisLay->addWidget(noiseSpin);
  noiseSpin->setStatusTip(tr("This value determines level of signal above witch sounds are detected."));
  calcButt = new QPushButton(tr("Calculate"), this);
  noisLay->addWidget(calcButt, 1, Qt::AlignCenter);
  calcButt->setStatusTip(tr("Click to automatically detect noise level.<br>Keep silence during 2 seconds to determine it properly."));
  noisGr->setLayout(noisLay);
  tunLay->addWidget(noisGr);
  
  upLay->addLayout(tunLay);
  
  inLay->addLayout(upLay);
  
  testTxt = tr("Test");
  stopTxt = tr("Stop");
  
  QGroupBox *testGr = new QGroupBox(this);
  QHBoxLayout *testLay = new QHBoxLayout();
  testButt = new QPushButton(testTxt, this);
  testButt->setStatusTip(tr("Check, Are audio input settings appropirate for You,<br>and did pitch detection work ?"));
  testLay->addWidget(testButt);
  testLay->addStretch(1);
  volMeter = new TpitchView(m_audioIn, this);
  testLay->addWidget(volMeter);
  volMeter->setStatusTip(tr("Level of a volume"));
  testLay->addStretch(1);
  QVBoxLayout *freqLay = new QVBoxLayout();
  freqLay->setAlignment(Qt::AlignCenter);
  pitchLab = new QLabel("--", this);
  pitchLab->setFixedWidth(70);
  pitchLab->setStatusTip(tr("Detected pitch"));
  pitchLab->setAlignment(Qt::AlignCenter);
  freqLay->addWidget(pitchLab);
  
  freqLab = new QLabel("--", this);
  freqLab->setFixedWidth(70);
  freqLab->setAlignment(Qt::AlignCenter);
  freqLab->setStatusTip(tr("Frequency of detected note.") + 
	  "<br><span style=\"font-family: nootka;\">6</span>E = 82,5Hz, " +
	  "<span style=\"font-family: nootka;\">5</span>A = 110Hz, " +
	  "<span style=\"font-family: nootka;\">4</span>d = 146Hz, " +
	  "<span style=\"font-family: nootka;\">3</span>g = 195Hz, " +
	  "<span style=\"font-family: nootka;\">2</span>h = 245Hz, " +
	  "<span style=\"font-family: nootka;\">1</span>e<sup>1</sup> = 330Hz");
  freqLay->addWidget(freqLab);
  testLay->addLayout(freqLay);
  testLay->addStretch(1);  
  
  testGr->setLayout(testLay);
  inLay->addWidget(testGr);
  
  enableInBox->setLayout(inLay);  
  lay->addWidget(enableInBox);
  setLayout(lay);
  
  inDeviceCombo->addItems(TaudioIN::getAudioDevicesList());
  setTestDisabled(true);
  
  connect(testButt, SIGNAL(clicked()), this, SLOT(testSlot()));
  connect(calcButt, SIGNAL(clicked()), this, SLOT(calcSlot()));
  connect(intervalCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(intervalChanged(int)));
  connect(freqSpin, SIGNAL(valueChanged(int)), this, SLOT(baseFreqChanged(int)));
  
}

AudioInSettings::~AudioInSettings()
{
}

//------------------------------------------------------------------------------------
//------------          methods       ------------------------------------------------
//------------------------------------------------------------------------------------
void AudioInSettings::setTestDisabled(bool disabled) {
  m_testDisabled = disabled;
  if (disabled) {
	volMeter->setDisabled(true);
	pitchLab->setText("--");
	freqLab->setText("--");
	pitchLab->setDisabled(true);
	freqLab->setDisabled(true);
  // enable the rest of widget
	inDeviceCombo->setDisabled(false);
	detectMethodCombo->setDisabled(false);
	voiceChB->setDisabled(false);
	midABox->setDisabled(false);
	noisGr->setDisabled(false);	
	loudChB->setDisabled(false);
  } else {
	volMeter->setDisabled(false);
	pitchLab->setDisabled(false);
	freqLab->setDisabled(false);
	// disable the rest of widget
	inDeviceCombo->setDisabled(true);
	detectMethodCombo->setDisabled(true);
	voiceChB->setDisabled(true);
	midABox->setDisabled(true);
	noisGr->setDisabled(true);
	loudChB->setDisabled(true);
  }
}

void AudioInSettings::grabParams() {
  m_aInParams.doingAutoNoiseFloor = true;
  m_aInParams.equalLoudness = true;
  m_aInParams.a440diff = freq2pitch(440.0) - freq2pitch((float)freqSpin->value());
  m_aInParams.analysisType = (EanalysisModes)detectMethodCombo->currentIndex();
  m_aInParams.devName = inDeviceCombo->currentText();
//   m_aInParams.doingAutoNoiseFloor = noiseChB->isChecked();
  m_aInParams.equalLoudness = loudChB->isChecked();
  m_aInParams.isVoice = voiceChB->isChecked();
  m_aInParams.noiseLevel = qRound((noiseSpin->value()/100) * 32768.0);
}



//------------------------------------------------------------------------------------
//------------          slots       --------------------------------------------------
//------------------------------------------------------------------------------------

void AudioInSettings::calcSlot() {
  if (!m_audioIn)
	m_audioIn = new TaudioIN(this);
  if (inDeviceCombo->currentText() != m_audioIn->deviceName())
	m_audioIn->setAudioDevice(inDeviceCombo->currentText());
  connect(m_audioIn, SIGNAL(noiseLevel(qint16)), this, SLOT(noiseDetected(qint16)));
  m_audioIn->calculateNoiseLevel();
}

void AudioInSettings::testSlot() {
  setTestDisabled(!m_testDisabled);
  if (!m_testDisabled) { // start a test
	if (!m_audioIn)
	  m_audioIn = new TaudioIN(this);
	if (inDeviceCombo->currentText() != m_audioIn->deviceName())
	  if(!m_audioIn->setAudioDevice(inDeviceCombo->currentText())) {
		setTestDisabled(true);
		return;
	  }
	grabParams();
	m_audioIn->setParameters(m_aInParams);
	testButt->setText(stopTxt);
	volMeter->setAudioInput(m_audioIn);
	volMeter->startVolume();
	connect(m_audioIn, SIGNAL(noteDetected(Tnote)), this, SLOT(noteSlot(Tnote)));
	connect(m_audioIn, SIGNAL(fundamentalFreq(float)), this, SLOT(freqSlot(float)));
	m_audioIn->startListening();
  } 
  else { // stop a test
	volMeter->stopVolume();
	m_audioIn->stopListening();
	testButt->setText(testTxt);
	setTestDisabled(true);
  }
}

void AudioInSettings::noiseDetected(qint16 noise) {
  if (noise < 10) {
	QMessageBox::warning(this, "", 
			tr("There isn't any noise !?!<br>It seems, Your audio input<br>is not configured properly."));
	m_noiseLevel = 70;
  } else
	m_noiseLevel = noise;
  double nVal = (noise/32768.0f)*100;
  disconnect(m_audioIn, SIGNAL(noiseLevel(qint16)), this, SLOT(noiseDetected(qint16)));
  noiseSpin->setValue(nVal);
}

void AudioInSettings::noteSlot(Tnote note) {
  pitchLab->setText("<b>"+QString::fromStdString(note.getName())+"</b>");
}

void AudioInSettings::freqSlot(float freq) {
	freqLab->setText(QString("%1 Hz").arg(freq, 0, 'f', 1, '0'));
}

void AudioInSettings::intervalChanged(int index) {
  if (intervalCombo->hasFocus()) {
		switch (index) {
			case 0 : freqSpin->setValue(465); break;
			case 1 : freqSpin->setValue(440); break;
			case 2 : freqSpin->setValue(415); break;
		}
  }
}

void AudioInSettings::baseFreqChanged(int bFreq) {
	if (freqSpin->hasFocus()) {
		if (freqSpin->value() <= 415)
			intervalCombo->setCurrentIndex(2);
		else if (freqSpin->value() >= 465)
			intervalCombo->setCurrentIndex(0);
		else
			intervalCombo->setCurrentIndex(1);
	}		
}





