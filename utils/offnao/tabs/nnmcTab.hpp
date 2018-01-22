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

#pragma once

#include <QTabWidget>

#include <QMenuBar>
#include <QWidget>
#include <QObject>
#include <QCheckBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QPixmap>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPainter>
#include <QPushButton>
#include <QProcess>
#include <QColor>
#include <QSignalMapper>
#include <QTreeWidgetItem>
#include <QTreeWidget>

#include <cstdio>
#include <deque>

#include "tabs/tab.hpp"
#include "perception/vision/other/YUV.hpp"
#include "perception/vision/colour/GreenYUVClassifier.hpp"
#include "perception/vision/VisionDefinitions.hpp"
#include "utils/Logger.hpp"
#include "mediaPanel.hpp"
#include "classifier.hpp"

#include "types/VisionInfoOut.hpp"

/*
   0 - raw
   1, 2 - loaded
   3 - combined
 */

#define NUM_CAMS 4

class Vision;

class Blackboard;

/*
 * This is the default/initial tab that shows on entering Off-Nao.
 * As the name suggests it gives an overview of most of the important
 * aspects of the nao. Good for general gameplay watching etc.
 *
 * Also will be used for localization debugging unless the localization
 * people need more info.
 */
class NNMCTab : public Tab {
   Q_OBJECT
   QProcess scpNNMC;
   public:
      NNMCTab(QTabWidget *parent, QMenuBar *menuBar, Vision *vision
                  );
      QPixmap *renderPixmap;
      QLabel *renderWidget;
   private:
      void init();
      void initMenu(QMenuBar *menuBar);

      /*  Draw the image on top of a pixmap */
      void drawImage(QImage *topImage, QImage *botImage);
      void drawOverlays(QPixmap *topImage, QPixmap *botImage);

      QRgb getEdge(unsigned int col,
         unsigned int row,
         const uint8_t *yuv,
         int num_cols,
         Gradient_Operator g,
         uint8_t (*getYUV)(const uint8_t*, int, int, int),
         int threshold);

      void displayScanLineResults(QLabel *qlabel,
         QPixmap qpixmap, 
         const uint8_t *image, 
         bool top, 
         int n_rows, 
         int n_cols, 
         int stepsize);

      void noiseReduce(const uint8_t *frame, uint8_t *noiseReducedFrame, int n_rows, int n_cols);

      bool haveData;
      int threshold;
      QString naoName;
      GreenYUVClassifier *green_yuv_classifier_top_;
      GreenYUVClassifier *green_yuv_classifier_bot_;

      QGridLayout *layout;

      QSignalMapper *signalMapper;

      QPixmap topImageRawPixmap;
      QLabel *topCamRawLabel;
      QPixmap botImageRawPixmap;
      QLabel *botCamRawLabel;

      QPixmap topImageFoveaPixmap;
      QLabel *topCamFoveaLabel;
      QPixmap botImageFoveaPixmap;
      QLabel *botCamFoveaLabel;

      QPixmap topImageEdgePixmap;
      QLabel *topCamEdgeLabel;
      QPixmap botImageEdgePixmap;
      QLabel *botCamEdgeLabel;

      bool nnmcLoaded;

      // Data
      Colour topSaliency[TOP_SALIENCY_ROWS][TOP_SALIENCY_COLS];
      Colour botSaliency[BOT_SALIENCY_ROWS][BOT_SALIENCY_COLS];
      Blackboard *blackboard;

      QPushButton *loadTopColour;
      QPushButton *loadBotColour;
      QPushButton *resetColour;
      QPushButton *saveTopColour;
      QPushButton *saveBotColour;
      QPushButton *runColourTop;
      QPushButton *runColourBot;
      QPushButton *runFillPointsGreen;
      QPushButton *runFillPointsWhite;

      QRadioButton *radioNAIVE;
      QRadioButton *radioSOBEL;
      QRadioButton *radioROBERTS;
      QRadioButton *radioISOTROPIC;

      QRadioButton *radioY;
      QRadioButton *radioU;
      QRadioButton *radioV;

      QRadioButton *radioColour;
      QRadioButton *radioDetectedEdges;

      QLineEdit *stepSizeNum;
      
      QRadioButton *thresholdAutoTop;
      QRadioButton *thresholdAutoBot;
      QRadioButton *thresholdNoAuto;

      QCheckBox *thresholdCheckbox;

      QCheckBox *showDetectedEdgesCheckbox;

      QLineEdit *thresholdNum;
      QLabel *resultInfo;

   public slots:
      void newNaoData(NaoData *naoData);
      void redraw();
      void safe_redraw();
      void loadTopNnmc();
      void loadBotNnmc();
      void saveTopNnmc();
      void saveBotNnmc();
      void runNnmcTop();
      void runNnmcBot();
      void resetNnmc();
      void runNnmcFillPointsGreen();
      void runNnmcFillPointsWhite();
};

