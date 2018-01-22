#include "ui.hpp"

#include <QtGui>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QObject>
#include <QLabel>
#include <QPixmap>

#include "uiElements/imageView.hpp"

UI::UI(int argc, char *argv[], AppAdaptor *appAdaptor):
    app(argc, argv),
    vatnaoManager(appAdaptor)
{}

int UI::exec(){
    QWidget window;

    ImageView topImageView;
    ImageView botImageView;

    QPushButton btnPPrev("<<");
    QPushButton btnPrev("<-");
    QPushButton btnNext("->");
    QPushButton btnNNext(">>");

    QObject::connect(&btnPPrev, SIGNAL(clicked()), &vatnaoManager, SLOT(prevTenFrames()));
    QObject::connect(&btnPrev, SIGNAL(clicked()), &vatnaoManager, SLOT(prevFrame()));
    QObject::connect(&btnNext, SIGNAL(clicked()), &vatnaoManager, SLOT(nextFrame()));
    QObject::connect(&btnNNext, SIGNAL(clicked()), &vatnaoManager, SLOT(nextTenFrames()));
    QObject::connect(&vatnaoManager, SIGNAL(topImageChanged(QPixmap)), &topImageView, SLOT(setPixmap(QPixmap)));
    QObject::connect(&vatnaoManager, SIGNAL(botImageChanged(QPixmap)), &botImageView, SLOT(setPixmap(QPixmap)));

    /*
     * Region visualiser
     */

    QGroupBox *groupBoxRegions = new QGroupBox ("Regions");
    QBoxLayout *vboxRegions = new QVBoxLayout ();

    groupBoxRegions->setLayout (vboxRegions);

    // Buttons

    QGroupBox *groupBoxRegionButtons = new QGroupBox ();
    QHBoxLayout *regionButtons = new QHBoxLayout ();

    QPushButton btnPPrevRegion("<<");
    QPushButton btnPrevRegion("<-");
    QPushButton btnNextRegion("->");
    QPushButton btnNNextRegion(">>");

    regionButtons->addWidget (&btnPPrevRegion);
    regionButtons->addWidget (&btnPrevRegion);
    regionButtons->addWidget (&btnNextRegion);
    regionButtons->addWidget (&btnNNextRegion);

    groupBoxRegionButtons->setLayout (regionButtons);

    vboxRegions->addWidget(groupBoxRegionButtons);

    QObject::connect(&btnPPrevRegion, SIGNAL(clicked()), &vatnaoManager, SLOT(prevTenRegions()));
    QObject::connect(&btnPrevRegion, SIGNAL(clicked()), &vatnaoManager, SLOT(prevRegion()));
    QObject::connect(&btnNextRegion, SIGNAL(clicked()), &vatnaoManager, SLOT(nextRegion()));
    QObject::connect(&btnNNextRegion, SIGNAL(clicked()), &vatnaoManager, SLOT(nextTenRegions()));

    // Saliency type

    QGroupBox *groupBoxSaliency = new QGroupBox ("Saliency type");
    QGridLayout *saliencyLayout = new QGridLayout();

    vatnaoManager.radioCircleCandidate = new QRadioButton ("Circle candidate points");
    vatnaoManager.radioOtsuThreshold = new QRadioButton ("Otsu thresholded image");
    vatnaoManager.radioColourSaliency = new QRadioButton ("Colour saliency");
    vatnaoManager.radioAll = new QRadioButton ("All");
    vatnaoManager.radioYHist = new QRadioButton ("Y Histogram");

    vatnaoManager.radioAll->setChecked(true);

    saliencyLayout->addWidget (vatnaoManager.radioCircleCandidate, 0, 0, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.radioOtsuThreshold, 0, 1, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.radioColourSaliency, 1, 0, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.radioAll, 1, 1, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.radioYHist, 2, 0, 1, 1);

    vatnaoManager.YHistThresh = new QLineEdit("100");
    saliencyLayout->addWidget (vatnaoManager.YHistThresh, 2, 1, 1, 1);

    vatnaoManager.checkBoxDownsampled = new QCheckBox("Downsampled?");
    vatnaoManager.checkBoxBallMetaData = new QCheckBox("Show ball metadata?");

    vatnaoManager.checkBoxDownsampled->setChecked(true);
    vatnaoManager.checkBoxBallMetaData->setChecked(false);
    vatnaoManager.resultInfo = new QLabel();
    vatnaoManager.resultInfo->setFont(QFont("Monospace"));
    vatnaoManager.resultInfo->setText("Status");

    saliencyLayout->addWidget (vatnaoManager.checkBoxDownsampled, 3, 0, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.checkBoxBallMetaData, 3, 1, 1, 1);
    saliencyLayout->addWidget (vatnaoManager.resultInfo, 4, 0, 1, 1);

    groupBoxSaliency->setLayout (saliencyLayout);

    vboxRegions->addWidget(groupBoxSaliency);

    QObject::connect(vatnaoManager.radioCircleCandidate, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));
    QObject::connect(vatnaoManager.radioOtsuThreshold, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));
    QObject::connect(vatnaoManager.radioColourSaliency, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));
    QObject::connect(vatnaoManager.radioAll, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));
    QObject::connect(vatnaoManager.radioYHist, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));

    QObject::connect(vatnaoManager.YHistThresh, SIGNAL(textChanged(const QString &)), &vatnaoManager, SLOT(saliencySelectionMade()));

    QObject::connect(vatnaoManager.checkBoxDownsampled, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencySelectionMade()));
    QObject::connect(vatnaoManager.checkBoxBallMetaData, SIGNAL(clicked()), &vatnaoManager, SLOT(showMetaData()));

    QGroupBox *groupBoxSaliencyROI = new QGroupBox ("ROI type");
    QGridLayout *saliencyROILayout = new QGridLayout();
    
    vatnaoManager.radioNaive = new QRadioButton ("Naive ROI");
    vatnaoManager.radioCircle = new QRadioButton ("Circle ROI");
    vatnaoManager.radioBlack = new QRadioButton ("Black ROI");
    vatnaoManager.radioCombo = new QRadioButton ("Combo ROI");

    vatnaoManager.radioCombo->setChecked(true);
    
    saliencyROILayout->addWidget (vatnaoManager.radioNaive, 0, 0, 1, 1);
    saliencyROILayout->addWidget (vatnaoManager.radioCircle, 0, 1, 1, 1);
    saliencyROILayout->addWidget (vatnaoManager.radioBlack, 1, 0, 1, 1);
    saliencyROILayout->addWidget (vatnaoManager.radioCombo, 1, 1, 1, 1);

    groupBoxSaliencyROI->setLayout (saliencyROILayout);

    vboxRegions->addWidget(groupBoxSaliencyROI);

    QObject::connect(vatnaoManager.radioNaive, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencyROISelectionMade()));
    QObject::connect(vatnaoManager.radioCircle, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencyROISelectionMade()));
    QObject::connect(vatnaoManager.radioBlack, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencyROISelectionMade()));
    QObject::connect(vatnaoManager.radioCombo, SIGNAL(clicked()), &vatnaoManager, SLOT(saliencyROISelectionMade()));

    QGroupBox *groupBoxROIButtons = new QGroupBox ();
    QHBoxLayout *roiButtons = new QHBoxLayout ();

    QPushButton btnPPrevROI("<<");
    QPushButton btnPrevROI("<-");
    QPushButton btnNextROI("->");
    QPushButton btnNNextROI(">>");

    roiButtons->addWidget (&btnPPrevROI);
    roiButtons->addWidget (&btnPrevROI);
    roiButtons->addWidget (&btnNextROI);
    roiButtons->addWidget (&btnNNextROI);

    groupBoxROIButtons->setLayout (roiButtons);

    vboxRegions->addWidget(groupBoxROIButtons);

    QObject::connect(&btnPPrevROI, SIGNAL(clicked()), &vatnaoManager, SLOT(prevTenROIs()));
    QObject::connect(&btnPrevROI, SIGNAL(clicked()), &vatnaoManager, SLOT(prevROI()));
    QObject::connect(&btnNextROI, SIGNAL(clicked()), &vatnaoManager, SLOT(nextROI()));
    QObject::connect(&btnNNextROI, SIGNAL(clicked()), &vatnaoManager, SLOT(nextTenROIs()));

    // Image representation

    ImageView rawImageView;
    ImageView saliencyImageView;

    vboxRegions->addWidget(&rawImageView);
    vboxRegions->addWidget(&saliencyImageView);

    QObject::connect(&vatnaoManager, SIGNAL(regionImageChanged(QPixmap)), &rawImageView, SLOT(setPixmap(QPixmap)));
    QObject::connect(&vatnaoManager, SIGNAL(regionImageSaliencyChanged(QPixmap)), &saliencyImageView, SLOT(setPixmap(QPixmap)));

    /*
     * Set up layout
     */

    QGridLayout *layout = new QGridLayout;

    layout->addWidget(&topImageView, 0, 0, 1, 8);
    layout->addWidget(&botImageView, 1, 0, 1, 8);
    layout->addWidget(&btnPPrev, 2, 0);
    layout->addWidget(&btnPrev, 2, 1, 1, 3);
    layout->addWidget(&btnNext, 2, 4, 1, 3);
    layout->addWidget(&btnNNext, 2, 7);

    layout->addWidget(groupBoxRegions, 0, 9, 2, 1, Qt::AlignTop | Qt::AlignLeft);

    window.setLayout(layout);
    window.show();

    return app.exec();
}
