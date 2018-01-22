/*
Copyright 2010 The University of New South Wales (UNSW).

This file is part of the 2010 team rUNSWift RoboCup entry. You may
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version as
modified below. As the original licensors, we add the following
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be
interpreted to include entry into a competition, and hence the source
of any derived work entered into a competition must be made available
to all parties involved in that competition under the terms of this
license.

In addition, if the authors of a derived work publish any conference
proceedings, journal articles or other academic papers describing that
derived work, then appropriate academic citations to the original work
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this source code; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <boost/shared_ptr.hpp>
#include <QButtonGroup>
#include <QDebug>
#include <QString>
#include <QDockWidget>
#include <QInputDialog>
#include <string>
#include <iostream>
#include <fstream>
#include "visualiser.hpp"
#include "visualiser.tcc"
#include "ui_visualiser.h"
#include "readers/recordReader.hpp"
#include "readers/bbdReader.hpp"
#include <utils/Logger.hpp>
#include <thread/Thread.hpp>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "progopts.hpp"

using namespace boost;
using std::string;
using namespace std;

Visualiser::Visualiser(QWidget *parent)
   : QMainWindow(parent), ui(new Ui::Visualiser),
   reader(0), naoData(0), cb() {
      // call initLogs so that vision et al don't segfault when they llog
      Thread::name = "Visualiser";
      Logger::init(config["debug.logpath"].as<string>(), config["debug.log"].as<string>(), true);

      lastTabIndex = -1;

      srand(time(0));
      extern bool offNao;
      offNao = true;

      vision = new Vision(false, false, false);

      ui->setupUi(this);
      ui->centralWidget->setMinimumSize(1250, 600);

      initMenu();

      // add the media panel to the bottom of the window
      mediaPanel = new MediaPanel(ui->centralWidget);

      // widgets for connecting to the naos
      connectionBar = new QDockWidget;
      cb.setupUi(connectionBar);
      connect(cb.cbHost, SIGNAL(activated(const QString &)),
              this, SLOT(connectToNao(const QString &)));
      connect(cb.bgMasks, SIGNAL(buttonClicked(QAbstractButton *)),
              this, SLOT(writeToNao(QAbstractButton *)));

      // set up tab holder
      tabs = new QTabWidget(ui->centralWidget);
      tabs->setMinimumSize(600, 450);

      overviewTab =  new OverviewTab(tabs, ui->menuBar, vision);
      nnmcTab =  new NNMCTab(tabs, ui->menuBar, vision);
      calibrationTab =  new CalibrationTab(tabs, ui->menuBar, vision);
      visionTab = new VisionTab(tabs, ui->menuBar, vision);
      localisationTab = new LocalisationTab();
      cameraTab = new CameraTab(tabs, ui->menuBar, vision);
      sensorTab = new SensorTab(tabs, ui->menuBar, vision);
      cameraPoseTab = new CameraPoseTab(tabs, ui->menuBar, vision);
      graphTab = new GraphTab(tabs, ui->menuBar, vision);
      walkTab = new WalkTab(tabs, ui->menuBar, vision);
      zmpTab = new ZMPTab(tabs, ui->menuBar, vision);
      controlTab = new ControlTab(tabs, ui->menuBar, vision);
      //surfTab =  new SurfTab(tabs, ui->menuBar, vision);
      icpTab = new ICPTab(tabs, ui->menuBar, vision);
      teamTab =  new TeamTab(tabs, ui->menuBar);
      teamBallTab = new TeamBallTab(tabs, ui->menuBar);

      tabVector.push_back(overviewTab);
      tabVector.push_back(nnmcTab);
      tabVector.push_back(calibrationTab);
      tabVector.push_back(teamTab);
      tabVector.push_back(visionTab);
      //tabVector.push_back(surfTab);
      tabVector.push_back(icpTab);
      tabVector.push_back(localisationTab);
      tabVector.push_back(cameraTab);
      tabVector.push_back(sensorTab);
      tabVector.push_back(cameraPoseTab);
      tabVector.push_back(graphTab);
      tabVector.push_back(walkTab);
      tabVector.push_back(zmpTab);
      tabVector.push_back(controlTab);
      tabVector.push_back(logsTab = new LogsTab(tabs, cb.cbHost));
      tabVector.push_back(teamBallTab);




      /* Set up the tabs */
      foreach (Tab *t, tabVector) {
         tabs->addTab(t, QString(t->metaObject()->className()).
                           remove(QRegExp("Tab$", Qt::CaseInsensitive)));
         connect(t, SIGNAL(showMessage(QString, int)),
                 ui->statusBar, SLOT(showMessage(QString, int)));
      }

      /* Used to redraw tabs when they are in focus */
      connect(tabs, SIGNAL(currentChanged(int)), this,
              SLOT(currentTabChanged(int)));

      ui->rootLayout->addWidget(mediaPanel, 1, 0, 1, 1);
      ui->rootLayout->addWidget(tabs, 0, 0, 1, 1);
   }

Visualiser::~Visualiser() {
   std::cout << "---- about to exit ----" << std::endl << std::flush;
   /*
   delete connectionBar;
   delete ui;
   for (unsigned int i = 0; i > tabVector.size(); i++) {
      delete tabVector[i];
   }
   */
}

void Visualiser::newNaoData(NaoData *naoData) {
   this->naoData = naoData;
}

// sets up the generic menus
void Visualiser::initMenu() {
   fileMenu = new QMenu("&File");
   ui->menuBar->addMenu(fileMenu);

   // set up file menu
   loadAct = new QAction(tr("&Load"), fileMenu);
   saveAsAct = new QAction(tr("&Save As"), fileMenu);

   exitAct = new QAction(tr("&Quit"), fileMenu);


   fileMenu->addAction(tr("Connect to &Nao"), this, SLOT(connectToNao()),
         QKeySequence(tr("Ctrl+N", "File|New")));
   fileMenu->addAction(tr("&Open File"), this, SLOT(openFileDialogue()),
         QKeySequence(tr("Ctrl+O", "File|Open")));
   // This is networkReader specific, but I'm not sure where to put it
   fileMenu->addAction(tr("Send Co&mmand Line String"), this,
                       SLOT(commandLineString()),
                            QKeySequence(tr("F5", "Refresh")));
   fileMenu->addAction(tr("&Save File"), this, SLOT(saveFileDialogue()),
                       QKeySequence(tr("Ctrl+S", "File|Save")));
   fileMenu->addAction(tr("&Save Whiteboard File"), this, SLOT(saveWhiteboardFileDialogue()),
                       QKeySequence(tr("Ctrl+W", "File|SaveWhiteboard")));
   fileMenu->addAction(tr("&Extract Raw Images"), this, SLOT(extractRawImagesDialogue()),
                       QKeySequence(tr("Ctrl+E", "File|Extract")));
   fileMenu->addAction(tr("&Extract Regions of Interest"), this, SLOT(extractROIsDialogue()),
                       QKeySequence(tr("Ctrl+R", "File|ExtractROI")));
   fileMenu->addAction(tr("&Close Current Reader"), this,
                       SLOT(disconnectFromNao()),
         QKeySequence(tr("Ctrl+W", "File|Close")));

   fileMenu->addAction(tr("&Quit"), this,
         SLOT(close()),
         QKeySequence(tr("Ctrl+Q", "Close")));

}


void Visualiser::close() {
   delete connectionBar;
   for (unsigned int i = 0; i > tabVector.size(); i++) {
      delete tabVector[i];
   }
   //delete tabs;
   //delete vision;
   delete ui;
   exit(0);
}

void Visualiser::openFileDialogue() {
   QString fileName =
     QFileDialog::getOpenFileName(this, "Open File",
                                  getenv("RUNSWIFT_CHECKOUT_DIR"),
                                  #if BOOST_HAS_COMPRESSION
                                  "Known file types (*.bbd *.yuv *.ofn *.ofn.gz *.ofn.bz2);;"
                                  #else
                                  "Known file types (*.bbd *.yuv *.ofn);;"
                                  #endif
                                  "Dump files (*.yuv);;"
                                  #if BOOST_HAS_COMPRESSION
                                  "Record files (*.ofn *.ofn.gz *.ofn.bz2)"
                                  #else
                                  "Record files (*.ofn);;"
                                  #endif
                                  "Blackboard Dump files (*.bbd)"
                                  );
   openFile(fileName);

}

void Visualiser::openFile (const QString &path)
{
   if (!path.isEmpty()) {
      if(path.endsWith(".bbd")) {
         reconnect<BBDReader, const QString &>(path);
      } else if(path.endsWith(".yuv"))
         reconnect<DumpReader, const QString &>(path);
      else if(path.endsWith(".ofn")
         #if BOOST_HAS_COMPRESSION
         || path.endsWith(".ofn.gz")  || path.endsWith(".ofn.bz2")
         #endif
         )
         reconnect<RecordReader, const QString &>(path);
      else
         ui->statusBar->showMessage(
           QString("Unknown file extension, modify %1:%2:%3").
                   arg(__FILE__, __PRETTY_FUNCTION__,
                       QString::number(__LINE__)));
   }
}

void Visualiser::saveWhiteboardFileDialogue() {
   QString fileName =
   QFileDialog::getSaveFileName(this, "Save File",
                                getenv("RUNSWIFT_CHECKOUT_DIR"),
                                       "Known file types (*.wb);;"
                                       "Record files (*.wb)"
                                       );
   std::cout << "SAVING WB FILE" << std::endl;
   saveFile(fileName);
}

void Visualiser::saveFileDialogue() {
   QString fileName =
   QFileDialog::getSaveFileName(this, "Save File",
                                getenv("RUNSWIFT_CHECKOUT_DIR"),
                                       #if BOOST_HAS_COMPRESSION
                                       "Known file types ("/**.yuv */"*.ofn *.ofn.gz *.ofn.bz2);;"
                                       #else
                                       "Known file types ("/**.yuv */"*.ofn);;"
                                       #endif
                                       // "Dump files (*.yuv);;"
                                       #if BOOST_HAS_COMPRESSION
                                       "Record files (*.ofn *.ofn.gz *.ofn.bz2)"
                                       #else
                                       "Record files (*.ofn)"
                                       #endif
                                       );

   saveFile(fileName);
}

void Visualiser::saveFile (const QString &path)
{
   if (!path.isEmpty()) {
      if(path.endsWith(".ofn")
            #if BOOST_HAS_COMPRESSION
            || path.endsWith(".ofn.gz")  || path.endsWith(".ofn.bz2")
            #endif
            )
      {
         RecordReader::write(path, *naoData);
      // else if(path.endsWith(".yuv"))
      //   saveDump(path);
      } else if (path.endsWith(".wb"))
      {
          RecordReader::writeWhiteboard(path, *naoData);
      } else 
      {
         ui->statusBar->showMessage(
         QString("Unknown file extension, saving as %1.ofn").
         arg(path));
         RecordReader::write(path + ".ofn", *naoData);
      }
   }
}

void Visualiser::extractRawImagesDialogue() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Save images to folder"),
                                               getenv("RUNSWIFT_CHECKOUT_DIR"),
                                               QFileDialog::ShowDirsOnly
                                               | QFileDialog::DontResolveSymlinks);
   extractRawImages(dir);
}

void Visualiser::extractRawImages(const QString &path)
{
   if (!path.isEmpty() && naoData) {
      RecordReader::dumpImages(path, *naoData, false);
   }
}

void Visualiser::extractROIsDialogue() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Save images to folder"),
                                               getenv("RUNSWIFT_CHECKOUT_DIR"),
                                               QFileDialog::ShowDirsOnly
                                               | QFileDialog::DontResolveSymlinks);
   extractROIs(dir);
}

void Visualiser::extractROIs(const QString &path)
{
   if (!path.isEmpty() && naoData) {
      RecordReader::dumpImages(path, *naoData, true);
   }
}

void Visualiser::keyPressEvent(QKeyEvent *event)
{
   //((QMainWindow*)this)->keyPressEvent(event);
   if (!event->isAutoRepeat())
   {
      controlTab->keyPressEvent(event);
      //std::cout << "_" << event->key() << "\n";
   }
}

void Visualiser::keyReleaseEvent(QKeyEvent *event)
{
   //((QMainWindow*)this)->keyReleaseEvent(event);
   if (!event->isAutoRepeat())
   {
      controlTab->keyReleaseEvent(event);
      //std::cout << "^" << event->key() << "\n";
   }
}

/*
 * Not yet used. Probably will be for the network reader.
 */
void Visualiser::startRecording(QAction *action) {
}

void Visualiser::currentTabChanged(int tabIndex) {
   if (lastTabIndex != -1) {
      tabVector[lastTabIndex]->tabDeselected();
   }
   tabVector[tabIndex]->tabSelected();

   emit refreshNaoData();
}

OffNaoMask_t Visualiser::transmissionMask(const QAbstractButton *) {
   OffNaoMask_t mask = 0;
   // TODO(jayen): make more efficient by just twiddling qab in the prior mask
   QList<QAbstractButton *> buttons = cb.bgMasks->buttons();
   while (!buttons.isEmpty()) {
      QAbstractButton *button = buttons.takeFirst();
      if (button->isChecked()) {
         if (button == cb.cbBlackboard)
            mask |= BLACKBOARD_MASK;
         else if (button == cb.cbSaliency)
            mask |= SALIENCY_MASK;
         else if (button == cb.cbRaw)
            mask |= RAW_IMAGE_MASK;
         else if (button == cb.cbBatch)
            mask |= USE_BATCHED_MASK;
         else if (button == cb.cbParticles)
            mask |= PARTICLE_FILTER_MASK;
         else if (button == cb.cbRobotFilter)
            mask |= ROBOT_FILTER_MASK;
         else if (button == cb.cbLandmarks)
            mask |= LANDMARKS_MASK;
         else
            ui->statusBar->showMessage("Error in parsing mask checkboxes!");
      }
   }
   return mask;
}

void Visualiser::connectToNao(const QString &naoName) {
   QString naoIP;
   string strNao = naoName.toStdString(); 
	 if (strNao.find('.') != string::npos){
      // If the ip address of the robot has been entered
      naoIP = naoName;
   } else {
      // If the name of the robot has been entered
      FILE *fpipe;
      const char *command = ("getent hosts " + strNao + ".local | awk '{ print $1 }'").c_str();
      char c = 0;
      string ipStr = "";
      fpipe = (FILE*)popen(command, "r");
      while (fread(&c, sizeof c, 1, fpipe)) {
         ipStr += c;
      }
      pclose(fpipe);
      naoIP = QString::fromStdString(ipStr);
   }
   if(reconnect<NetworkReader, pair<pair<const QString &, int>, OffNaoMask_t> >
      (make_pair(make_pair(naoIP, cb.sbPort->value()), transmissionMask(NULL)))) {
      NetworkReader *reader = dynamic_cast<NetworkReader *>(this->reader);
      Q_ASSERT(reader);
      connect(this, SIGNAL(sendCommandLineString(const QString&)), reader,
              SLOT(sendCommandLineString(QString)));
      connect(cameraPoseTab, SIGNAL(sendCommandToRobot(QString)),
              reader, SLOT(sendCommandLineString(QString)));
      connect(cameraTab, SIGNAL(sendCommandToRobot(QString)),
              reader, SLOT(sendCommandLineString(QString)));
      connect(logsTab, SIGNAL(sendCommandToRobot(QString)),
              reader, SLOT(sendCommandLineString(QString)));

      this->reader = reader;
      setUpReaderSignals(this->reader);
      this->reader->start();
      controlTab->setNao(naoIP);
      overviewTab->setNao(naoName);
   }
}

void Visualiser::writeToNao(QAbstractButton *qab) {
   if (reader)
      // TODO(jayen): check reader is a networkreader
      ((NetworkReader*)reader)->write(transmissionMask(qab));
}

void Visualiser::connectToNao() {
   addDockWidget(Qt::TopDockWidgetArea, connectionBar);
   connectionBar->show();
   cb.cbHost->setFocus();
   cb.cbHost->lineEdit()->selectAll();
}

// This is networkReader specific, but I'm not sure where to put it
void Visualiser::commandLineString() {
   QString item = QInputDialog::getText(NULL, "Send String",
                                           tr("Command FieldEdge String:"));
   emit sendCommandLineString(item);
}

void Visualiser::disconnectFromNao() {
   if (reader && reader->isFinished() == false) {
      connect(reader, SIGNAL(finished()), this, SLOT(disconnectFromNao()));
      reader->finishUp();
      qDebug("Try to destroy reader. Wait for thread to exit.");
   } else if (reader) {
      emit readerClosed();
      delete reader;
      reader = 0;
      qDebug("Finished destroying reader");
      ui->statusBar->showMessage(QString("Reader destroyed."));
   }
}

void Visualiser::setUpReaderSignals(Reader *reader) {

   for (unsigned int i = 0; i < tabVector.size(); i++) {
      connect(reader, SIGNAL(newNaoData(NaoData*)), tabVector[i],
            SLOT(newNaoData(NaoData *)));
      connect(this, SIGNAL(readerClosed()), tabVector[i],
            SLOT(readerClosed()));
   }

   connect(reader, SIGNAL(newNaoData(NaoData*)), mediaPanel,
         SLOT(newNaoData(NaoData *)));
   connect(reader, SIGNAL(newNaoData(NaoData*)), this,
           SLOT(newNaoData(NaoData *)));

   connect(mediaPanel->forwardAct, SIGNAL(triggered()), reader,
         SLOT(forwardMediaTrigger()));
   connect(mediaPanel->backAct, SIGNAL(triggered()), reader,
         SLOT(backwardMediaTrigger()));
   connect(mediaPanel->playAct, SIGNAL(triggered()), reader,
         SLOT(playMediaTrigger()));
   connect(mediaPanel->pauseAct, SIGNAL(triggered()), reader,
         SLOT(pauseMediaTrigger()));
   connect(mediaPanel->stopAct, SIGNAL(triggered()), reader,
         SLOT(stopMediaTrigger()));
   connect(mediaPanel->recordAct, SIGNAL(triggered()), reader,
         SLOT(recordMediaTrigger()));
   connect(mediaPanel->frameSlider, SIGNAL(valueChanged(int)),
         reader, SLOT(sliderMoved(int)));

   connect(this, SIGNAL(refreshNaoData()), reader, SLOT(refreshNaoData()));
   connect(mediaPanel->frameSlider, SIGNAL(sliderReleased()), reader,
         SLOT(refreshNaoData()));
   connect(reader, SIGNAL(showMessage(QString, int)), ui->statusBar,
           SLOT(showMessage(QString, int)));
   connect(reader, SIGNAL(openFile()), this, SLOT(openFileDialogue()));
   connect(reader, SIGNAL(disconnectFromNao()), this,
           SLOT(disconnectFromNao()));
   mediaPanel->recordButton->setFocus();
}
