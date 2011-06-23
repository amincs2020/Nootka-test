#include "tglobals.h"
#include "tkeysignature.h"
#include <QDir>
#include <QSettings>
#include <QCoreApplication>



/*static*/
QString Tglobals::getInstPath(QString appInstPath) {
    QString p = "";
#if defined(Q_OS_LINUX)
    QDir d = QDir(appInstPath);
    d.cdUp();
    p = d.path()+"/share/nootka/"; //Linux
#endif
    return p;
}

QString Tglobals::getBGcolorText(QColor C) {
  if ( C != -1)
    return QString(
      "background-color: rgba(%1, %2, %3, %4); ").arg(C.red()).arg(C.green()).arg(C.blue()).arg(C.alpha());
  else
    return QString("background-color: transparent; ");
}

QColor Tglobals::invertColor(QColor C) {
    if (C != -1)
        C.setRgb(qRgb(255-C.red(), 255-C.green(), 255-C.blue()));
    return C;
}

/*end static*/


Tglobals::Tglobals() {

    version = "0.6 beta";
//    path ; Is declared in mainWindow constructor

    QCoreApplication::setOrganizationName("Nootka");
    QCoreApplication::setOrganizationDomain("nootka.sf.net");
    QCoreApplication::setApplicationName("Nootka");
    QSettings sett;
#if defined(Q_OS_WIN32) // I hate mess in Win registry
    sett = QSettings(QSettings::IniFormat, QSettings::UserScope, "Nootka", "Nootka");
#endif

    sett.beginGroup("common");
        hintsEnabled = sett.value("enableHints", true).toBool(); //true;
    sett.endGroup();

//score widget settings
    sett.beginGroup("score");
        SkeySignatureEnabled = sett.value("keySignature", true).toBool(); //true;
        SshowKeySignName = sett.value("keyName", true).toBool(); //true;
        SnameStyleInKeySign = Tnote::EnameStyle(sett.value("nameStyleInKey",
                                                           (int)Tnote::e_english_Bb).toInt());
        SmajKeyNameSufix = sett.value("majorKeysSufix", QObject::tr("major")).toString();
        SminKeyNameSufix = sett.value("minorKeysSufix", QObject::tr("minor")).toString();
	if (sett.contains("pointerColor"))
	    SpointerColor = sett.value("pointerColor").value<QColor>(); //-1;
	else 
	    SpointerColor = -1;
    sett.endGroup();

    TkeySignature::setNameStyle(SnameStyleInKeySign, SmajKeyNameSufix, SminKeyNameSufix);

//common for score widget and note name
    sett.beginGroup("common");
        doubleAccidentalsEnabled = sett.value("doubleAccidentals", true).toBool(); //true;
	showEnharmNotes = sett.value("showEnaharmonicNotes", true).toBool(); //true;
	if (sett.contains("enharmonicNotesColor"))
	    enharmNotesColor = sett.value("enharmonicNotesColor").value<QColor>(); //-1;
	else
	    enharmNotesColor = -1;
	seventhIs_B = sett.value("is7thNote_B", true).toBool(); //true;
    sett.endGroup();
    
//note name settings    
    sett.beginGroup("noteName");
	NnameStyleInNoteName = Tnote::EnameStyle(sett.value("nameStyle", (int)Tnote::e_english_Bb).toInt());
	NoctaveInNoteNameFormat = sett.value("octaveInName", true).toBool();
	//    NoctaveNameInNoteName = true;
    sett.endGroup();

// guitar settings
    sett.beginGroup("guitar");
	GfretsNumber = sett.value("fretNumber", 19).toInt();
	GisRightHanded = sett.value("rightHanded", true).toBool(); //true;
	GshowOtherPos = sett.value("showOtherPos", true).toBool(); //true;
	if (sett.contains("fingerColor"))
	    GfingerColor = sett.value("fingerColor").value<QColor>();
	else
	    GfingerColor = -1;
	if (sett.contains("selectedColor"))
	    GselectedColor = sett.value("selectedColor").value<QColor>();
	else
	    GselectedColor = -1;
//    Q_DECLARE_METATYPE(Ttune)
    qRegisterMetaType<Ttune>("Ttune");
    qRegisterMetaTypeStreamOperators<Ttune>("Ttune");
   setTune(Ttune::stdTune);
      GpreferFlats = sett.value("flatsPrefered", false).toBool(); //false;	
    sett.endGroup();

   
// Exam settings
    EquestionColor = QColor("red");
    EquestionColor.setAlpha(40);
    EanswerColor = QColor("green");
    EanswerColor.setAlpha(40);
    EautoNextQuest = false;
    ErepeatIncorrect = true;

}

Tglobals::~Tglobals() {
    storeSettings();
}

void Tglobals::setTune(Ttune t) {
    m_tune = t;
// creating array with guitar strings in order of their height
    char openStr[6];
    for (int i=0; i<6; i++) {
        m_order[i] = i;
        openStr[i] = m_tune[i+1].getChromaticNrOfNote();
    }
      int i = 4;
      while (i > -1) {
          for (int j=i; j < 5 && openStr[m_order[j]] < openStr[m_order[j+1]]; j++) {
              char tmp = m_order[j];
              m_order[j] = m_order[j+1];
              m_order[j+1] = tmp;
          }
          i--;
      }
}

void Tglobals::storeSettings() {
    QSettings sett;
#if defined(Q_OS_WIN32) // I hate mess in Win registry
    sett = QSettings(QSettings::IniFormat, QSettings::UserScope, "Nootka", "Nootka");
#endif
    sett.beginGroup("common");
        sett.setValue("enableHints", hintsEnabled);
        sett.setValue("doubleAccidentals", doubleAccidentalsEnabled);
        sett.setValue("showEnaharmonicNotes", showEnharmNotes);
        sett.setValue("enharmonicNotesColor", enharmNotesColor);
        sett.setValue("is7thNote_B", seventhIs_B);
    sett.endGroup();

    sett.beginGroup("score");
        sett.setValue("keySignature", SkeySignatureEnabled);
        sett.setValue("keyName", SshowKeySignName);
        sett.setValue("nameStyleInKey", (int)SnameStyleInKeySign);
        sett.setValue("majorKeysSufix", SmajKeyNameSufix);
        sett.setValue("minorKeysSufix", SminKeyNameSufix);
        sett.setValue("pointerColor", SpointerColor);
    sett.endGroup();
  
    sett.beginGroup("noteName");
	sett.setValue("nameStyle", (int)NnameStyleInNoteName);
	sett.setValue("octaveInName", NoctaveInNoteNameFormat);
    sett.endGroup();
    
    sett.beginGroup("guitar");
	sett.setValue("fretNumber", (int)GfretsNumber);
	sett.setValue("rightHanded", GisRightHanded);
	sett.setValue("showOtherPos", GshowOtherPos);
	sett.setValue("fingerColor", GfingerColor);
	sett.setValue("selectedColor", GselectedColor);
// 	sett.setValue(""); //tune
//    QVariant var = settings.value("Ttune");
//    if (var.isValid()) {
//        Ttune tt = var.value<Ttune>();

//    } else {
        sett.setValue("tune", qVariantFromValue(Gtune()));
//    }
	sett.setValue("flatsPrefered", GpreferFlats);
    sett.endGroup();
}
