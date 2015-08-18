/***************************************************************************
 *   Copyright (C) 2011-2015 by Tomasz Bojczuk                  				   *
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


#include "tsettingsdialogbase.h"
#include <widgets/troundedlabel.h>
#include <touch/ttouchproxy.h>
#include <touch/ttouchmenu.h>
#include <tmtr.h>
#include <QtWidgets>

/* static */
bool TsettingsDialogBase::touchEnabled() { return TtouchProxy::touchEnabled(); }


TsettingsDialogBase::TsettingsDialogBase(QWidget *parent) :
  QDialog(parent),
  m_menuTap(false)
{
    QVBoxLayout *mainLay = new QVBoxLayout;
    QHBoxLayout *contLay = new QHBoxLayout;
    navList = new QListWidget(this);
    navList->setIconSize(QSize(80, 80));
    navList->setFixedWidth(100);
    navList->setViewMode(QListView::IconMode);
		navList->setMovement(QListView::Static);
    navList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    contLay->addWidget(navList);

    m_aLay = new QVBoxLayout;
    stackLayout = new QStackedLayout;
    
    m_widget = new QWidget(this);
    m_scrollArea = new QScrollArea(this);
		m_scrollArea->hide();
    
    m_widget->setLayout(stackLayout);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_aLay->addWidget(m_widget);
    hint = new TroundedLabel(this);
    m_aLay->addWidget(hint);
    hint->setFixedHeight(fontMetrics().boundingRect("A").height() * 4);
		hint->setMinimumWidth(fontMetrics().boundingRect("w").width() * 70);
    hint->setWordWrap(true);
    contLay->addLayout(m_aLay);

    mainLay->addLayout(contLay);

		buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
		mainLay->addWidget(buttonBox);
    setLayout(mainLay);
		
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    if (touchEnabled()) {
      useScrollArea();
      setAttribute(Qt::WA_AcceptTouchEvents);
      buttonBox->hide();
      navList->hide();
    }
#if defined (Q_OS_ANDROID)
//     setContentsMargins(0, 0, 0, 0);
    mainLay->setContentsMargins(0, 0, 0, 0);
    m_aLay->setContentsMargins(0, 0, 0, 0);
    stackLayout->setContentsMargins(0, 0, 0, 0);
#endif
}


//#################################################################################################
//###################              PROTECTED           ############################################
//#################################################################################################

void TsettingsDialogBase::useScrollArea() {
  showMaximized();
  hint->hide();
  m_aLay->removeWidget(m_widget);
  m_scrollArea->setWidget(m_widget);
  m_aLay->insertWidget(0, m_scrollArea);
  m_scrollArea->show();
}


void TsettingsDialogBase::fitSize() {
  if (qApp->desktop()->availableGeometry().height() <= 600) {
#if defined (Q_OS_WIN)
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setMinimumSize(qApp->desktop()->availableGeometry().width() - 100, qApp->desktop()->availableGeometry().height() - 70);
#endif
    useScrollArea();
    convertStatusTips();
    connect(stackLayout, SIGNAL(currentChanged(int)), this, SLOT(convertStatusTips()));
  }
  navList->setFixedWidth(navList->sizeHintForColumn(0) + 2 * navList->frameWidth() +
          (navList->verticalScrollBar()->isVisible() ? navList->verticalScrollBar()->width() : 0));
}


void TsettingsDialogBase::convertStatusTips() {
	QList<QWidget*> allWidgets = findChildren<QWidget*>();
	foreach(QWidget *w, allWidgets) {
		if (w->statusTip() != "") {
			w->setToolTip(w->statusTip());
			w->setStatusTip("");
		}
	}
}


void TsettingsDialogBase::tapMenu() {
  TtouchMenu *menu = new TtouchMenu(this);
  for (int i = 0; i < navList->count(); ++i) {
    QAction *navAction = new QAction(navList->item(i)->icon(), navList->item(i)->text(), menu);
    navAction->setData(i * 2); // 0, 2, 4....
    menu->addAction(navAction);
  }
  for (int i = 0; i < buttonBox->buttons().size(); ++i) {
    QAction *buttonAction = new QAction(buttonBox->buttons()[i]->icon(), buttonBox->buttons()[i]->text(), menu);
    buttonAction->setData((i * 2) + 1); // 1, 3, 5...
    menu->addAction(buttonAction);
  }
  QAction *menuAction = menu->exec(QPoint(2, 2), QPoint(-menu->sizeHint().width(), 2));
  int actionNumber = menuAction ? menuAction->data().toInt() : -1;
  delete menu; // delete menu before performing its action
  if (actionNumber != -1) {
    if (actionNumber % 2) // 1, 3, 5....
      buttonBox->buttons()[actionNumber / 2]->click();
    else // 0, 2, 4....
      stackLayout->setCurrentIndex(actionNumber / 2);
  }
  m_menuTap = false;
}



void TsettingsDialogBase::openHelpLink(const QString& hash) {
  QDesktopServices::openUrl(QUrl(QString("http://nootka.sourceforge.net/index.php?L=%1&C=doc#" + hash).
    arg(QString(std::getenv("LANG")).left(2).toLower()), QUrl::TolerantMode));
}


bool TsettingsDialogBase::event(QEvent *event) {
  if (touchEnabled()) {
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd) {
      QTouchEvent *te = static_cast<QTouchEvent*>(event);
      if (m_menuTap || te->touchPoints().first().pos().x() < Tmtr::fingerPixels() / 3) {
        if (event->type() == QEvent::TouchBegin) {
          event->accept();
          m_menuTap = true;
        } else if (event->type() == QEvent::TouchEnd) {
            if (m_menuTap && te->touchPoints().first().pos().x() > width() * 0.15)
              tapMenu();
            else
              m_menuTap = false;
        }
        return true;
      } else {
        if (event->type() == QEvent::TouchBegin) {
          event->accept();
          return true;
        } else if (event->type() == QEvent::TouchUpdate) {
            if (QLineF(te->touchPoints().first().pos(), te->touchPoints().first().startPos()).length() > 20) // TODO use screen factor instead 10
              m_scrollArea->verticalScrollBar()->setValue(
                m_scrollArea->verticalScrollBar()->value() + (te->touchPoints()[0].lastPos().y() - te->touchPoints()[0].pos().y()));
        }
      }
    }
  } else {
      if (event->type() == QEvent::StatusTip) {
          QStatusTipEvent *se = static_cast<QStatusTipEvent *>(event);
          hint->setText("<center>"+se->tip()+"</center>");
      } else if (event->type() == QEvent::Resize)
        QTimer::singleShot(20, this, SLOT(fitSize()));
  }
  return QDialog::event(event);
}






