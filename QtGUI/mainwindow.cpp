#include "mainwindow.h"
#include <QtWidgets\QLabel>
#include <QtWidgets\QGridLayout>
#include <QtWidgets\QVBoxLayout>
#include <QtWidgets\QTableWidget>
#include <QtWidgets\QPushButton>
#include <QtWidgets\QCheckBox>
#include <QtWidgets\QSpinBox>
#include <QtWidgets\QLineEdit>
#include <QtWebKitWidgets\QWebView>
#include <QtWidgets\QGraphicsScene.h>
#include <QtWidgets\QGraphicsView.h>
#include <QtCore\QTextCodec>
#include <QtWidgets\QGraphicsRectItem>
#include <fstream>
#include <BadmCore/common.h>

// report for magnetic pairs only
#define MAGNETIC_REPORT

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  setWindowTitle("Badminton 2015");

  setCentralWidget(new QWidget);
  
  QGridLayout* aMainLayout1 = new QGridLayout(centralWidget());

  myCore = new Common;
  myCore->ReadAll();

  statisticsLayout(aMainLayout1);

  resize(800, 600);
}

void MainWindow::statisticsLayout(QGridLayout* theLayout)
{
  // some statistics
  theLayout->addWidget(new QLabel("Initial data is processed", this), 0, 0, 1, 2);
  theLayout->addWidget(new QLabel(QString("Games")), 1, 0);
  theLayout->addWidget(new QLabel(QString("%1").arg(myCore->GamesRead())), 1, 1);
  int aPlNum = myCore->PlayersRead();
  theLayout->addWidget(new QLabel(QString("Players")), 2, 0);
  theLayout->addWidget(new QLabel(QString("%1").arg(aPlNum)), 2, 1);
  // table with raitings
  QTableWidget* aRaitingsTable = new QTableWidget(aPlNum, 2, this);
  QStringList aColumsLabs, aRowsLabs;
  aColumsLabs<<"Raiting"<<"Tour Raiting";
  aRaitingsTable->setHorizontalHeaderLabels(aColumsLabs);
  for(int a = 0; a < aPlNum; a++) {
    QString aName(QTextCodec::codecForName("CP1251")->toUnicode(myCore->Name(a).c_str()));
    aRowsLabs<<aName;
    QString aRating = QString("%1").arg(myCore->Raiting(a, false));
    aRaitingsTable->setItem(a, 0, new QTableWidgetItem(aRating, 0));
    aRating = QString("%1").arg(myCore->Raiting(a, true));
    aRaitingsTable->setItem(a, 1, new QTableWidgetItem(aRating, 0));
  }
  aRaitingsTable->setVerticalHeaderLabels(aRowsLabs);
  theLayout->addWidget(aRaitingsTable, 0, 2, 5, 1);
  theLayout->setRowStretch(3, 5);
  // buttons to continue
  QWidget* aVWidget = new QWidget(this);
  QVBoxLayout* aVLayout = new QVBoxLayout(aVWidget);
  aVWidget->setLayout(aVLayout);
  QPushButton* aSortings = new QPushButton("Sortings", this);
  connect(aSortings, SIGNAL(released()), this, SLOT(onSortings()));
  aVLayout->addWidget(aSortings);
  QPushButton* aReport = new QPushButton("Report", this);
  connect(aReport, SIGNAL(released()), this, SLOT(onReport()));
  aVLayout->addWidget(aReport);
#ifdef MAGNETIC_REPORT
  QPushButton* aReportM = new QPushButton("Magnetic rep", this);
  connect(aReportM, SIGNAL(released()), this, SLOT(onReportM()));
  aVLayout->addWidget(aReportM);
#endif
  QPushButton* anExit = new QPushButton("Exit", this);
  connect(anExit, SIGNAL(released()), this, SLOT(onExit()));
  aVLayout->addWidget(anExit);
  theLayout->addWidget(aVWidget, 4, 0, 1, 2);
}

void MainWindow::onExit() {
  exit(0);
}
void MainWindow::onSortings() {
  myPls = 0;
  int aPSize = myCore->PreparePlay(myPls);

  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());
  myPlsChecks = new QCheckBox*[aPSize + 10];
  int a;
  for(a = 0; a < aPSize; a++) {
    // two columns
    const int aColCol = 3; // number of columns in each column
    int aCol = a % 2;
    int aRow = a / 2;
    QString aName(QTextCodec::codecForName("CP1251")->toUnicode(
      myPls[a].myFullName.empty() ? myPls[a].myNick : myPls[a].myFullName.c_str()));
    QLabel* aNameLab = new QLabel(aName, this);
    aLayout->addWidget(aNameLab, aRow, aColCol * aCol + 0);
    QString aMisfStr = QString("%1").arg(myPls[a].myMisfortune);
    QLabel* aMisfLab = new QLabel(aMisfStr, this);
    aLayout->addWidget(aMisfLab, aRow, aColCol * aCol + 1);
    myPlsChecks[a] = new QCheckBox(this);
    myPlsChecks[a]->setChecked(myPls[a].myIsToday);
    aLayout->addWidget(myPlsChecks[a], aRow, aColCol * aCol + 2);
  }
  myPlsChecks[a] = 0; // last is null
  QPushButton* aSortNext = new QPushButton("Next", this);
  connect(aSortNext, SIGNAL(released()), this, SLOT(onSortNext()));
  aLayout->addWidget(aSortNext, aPSize / 2 + 1, 0);
}

void MainWindow::onSortNext()
{
  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());
  aLayout->addWidget(new QLabel("Selected players "), 0, 0);
  int aNumSelected = 0;
  for(int a = 0; myPlsChecks[a]; a++) {
    if (myPlsChecks[a]->isChecked()) aNumSelected++;
    myPls[a].myIsToday = myPlsChecks[a]->isChecked();
  }
  aLayout->addWidget(new QLabel(QString("%1").arg(aNumSelected)), 0, 1);
  aLayout->addWidget(new QLabel("Number of doubles games "), 1, 0);
  myNumPlaces4 = new QSpinBox(this);
  myNumPlaces4->setMinimum(0);
  myNumPlaces4->setMaximum(aNumSelected / 4);
  int aPools = aNumSelected / 4;
  if (aNumSelected % 4 == 1) aPools--;
  if (aPools > 4) aPools = 4;
  myNumPlaces4->setValue(aPools);
  aLayout->addWidget(myNumPlaces4, 1, 1);

  aLayout->addWidget(new QLabel("Number of singles games "), 2, 0);
  myNumPlaces2 = new QSpinBox(this);
  myNumPlaces2->setMinimum(0);
  myNumPlaces2->setMaximum(aNumSelected / 2);
  int aSinglesNum = aNumSelected - aPools * 4;
  aSinglesNum -= aSinglesNum % 1;
  aSinglesNum /= 2;
  if (aSinglesNum && aNumSelected - aSinglesNum * 2 - aPools * 4 == 1)
    aSinglesNum--;
  myNumPlaces2->setValue(aSinglesNum);
  aLayout->addWidget(myNumPlaces2, 2, 1);

  aLayout->addWidget(new QLabel(""), 3, 0);
  aLayout->setRowStretch(3, 5);

  QPushButton* aSortNext = new QPushButton("Next", this);
  connect(aSortNext, SIGNAL(released()), this, SLOT(onSortTotal()));
  aLayout->addWidget(aSortNext, 4, 0);
}

void MainWindow::onSortTotal()
{
  myIntNumPlaces4 = myNumPlaces4->value();
  myIntNumPlaces2 = myNumPlaces2->value();
  myCore->sort(myPls, myIntNumPlaces4, myIntNumPlaces2);
  myScores = new QLineEdit*[myIntNumPlaces4 + myIntNumPlaces2];
  mySortingPlayers = new std::string[myIntNumPlaces4 + myIntNumPlaces2];
  // show the results of sortings
  static const int aRowsPerPlace = 5;
  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());
  for(int aPlace = 0; aPlace < myIntNumPlaces4 + myIntNumPlaces2; aPlace++) {
    GameToday aGame = myCore->sorted(aPlace);

    QString aPl0 = QString("%1").arg(aGame.myNicks[0]);
    aLayout->addWidget(new QLineEdit(aPl0, this), aRowsPerPlace * aPlace, 0);
    QString aRating0 = QString("g:%1 l:%2").arg(myCore->Raiting(aGame.myNicks[0], false)).arg(myCore->Raiting(aGame.myNicks[0], true));
    aLayout->addWidget(new QLineEdit(aRating0, this), aRowsPerPlace * aPlace, 1);

    QString aPl1;
    if (!aGame.isSingle) 
      aPl1 = QString("%1").arg(aGame.myNicks[1]);
    aLayout->addWidget(new QLineEdit(aPl1, this), aRowsPerPlace * aPlace + 2, 0);
    QString aRating1;
    if (!aGame.isSingle) 
      aRating1 = QString("g:%1 l:%2").arg(myCore->Raiting(aGame.myNicks[1], false)).arg(myCore->Raiting(aGame.myNicks[1], true));
    aLayout->addWidget(new QLineEdit(aRating1, this), aRowsPerPlace * aPlace + 2, 1);

    QString aPl2;
    if (aGame.isSingle) 
      aPl2 = QString("%1").arg(aGame.myNicks[1]);
    else 
      aPl2 = QString("%1").arg(aGame.myNicks[2]);
    aLayout->addWidget(new QLineEdit(aPl2, this), aRowsPerPlace * aPlace, 5);
    QString aRating2;
    if (aGame.isSingle) 
      aRating2 = QString("g:%1 l:%2").arg(myCore->Raiting(aGame.myNicks[1], false)).arg(myCore->Raiting(aGame.myNicks[1], true));
    else
      aRating2 = QString("g:%1 l:%2").arg(myCore->Raiting(aGame.myNicks[2], false)).arg(myCore->Raiting(aGame.myNicks[2], true));
    aLayout->addWidget(new QLineEdit(aRating2, this), aRowsPerPlace * aPlace, 4);

    QString aPl3;
    if (!aGame.isSingle)
      aPl3 = QString("%1").arg(aGame.myNicks[3]);
    aLayout->addWidget(new QLineEdit(aPl3, this), aRowsPerPlace * aPlace + 2, 5);
    QString aRating3;
    if (!aGame.isSingle)
      aRating3 = QString("g:%1 l:%2").arg(myCore->Raiting(aGame.myNicks[3], false)).arg(myCore->Raiting(aGame.myNicks[3], true));
    aLayout->addWidget(new QLineEdit(aRating3, this), aRowsPerPlace * aPlace + 2, 4);

    if (aGame.isSingle)
      mySortingPlayers[aPlace] = std::string(aGame.myNicks[0]) + " " + std::string(aGame.myNicks[1]);
    else 
      mySortingPlayers[aPlace] = std::string(aGame.myNicks[0]) + " " + std::string(aGame.myNicks[1]) + " " + std::string(aGame.myNicks[2]) + " " + std::string(aGame.myNicks[3]);
    aLayout->addWidget(new QLabel("Results:"), aRowsPerPlace * aPlace + 3, 0);
    myScores[aPlace] = new QLineEdit(this);
    myScores[aPlace]->setText(aGame.myPropScore.c_str());
    aLayout->addWidget(myScores[aPlace], aRowsPerPlace * aPlace + 3, 1, 1, 5);

    aLayout->setRowStretch(aRowsPerPlace * aPlace + 4, 5);
  }

  QPushButton* aSave = new QPushButton("Save", this);
  connect(aSave, SIGNAL(released()), this, SLOT(onSortSave()));
  aLayout->addWidget(aSave, aRowsPerPlace * (myIntNumPlaces4 + myIntNumPlaces2), 0);
}

void MainWindow::onSortSave() {
  std::ofstream res("today.txt", std::ofstream::out | std::ofstream::app);
  for(int aPlace = 0; aPlace < myIntNumPlaces4 + myIntNumPlaces2; aPlace++) {
    res<<mySortingPlayers[aPlace]<<std::endl;
    res<<myScores[aPlace]->text().toStdString()<<std::endl;
  }
  res<<"x"<<myCore->unlucky()<<" x"<<std::endl;
  exit(0);
}

void MainWindow::onReport() {
  std::string aHTML = myCore->saveReport();
  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());

  QWebView* aWebView = new QWebView(this);
  QString aQHTML(QTextCodec::codecForName("CP1251")->toUnicode(aHTML.c_str()));
  aWebView->setHtml(aQHTML);
  aLayout->addWidget(aWebView, 0, 0);

  QPushButton* aGr = new QPushButton("Graphics", this);
  connect(aGr, SIGNAL(released()), this, SLOT(onReportGraphics()));
  aLayout->addWidget(aGr, 1, 0);
}
  // for each player make nice picture of size:
static int kTotalW = 800, kTotalH = 50;
// and shift from left:
static int kTotalShift = 50;

static double positionByRating(const int theRating)
{
  int aRatingX = int(double(theRating) / 1000. * kTotalW);
  return kTotalW - aRatingX + kTotalShift;
}

static void drawPlayer(QGraphicsScene* theScene, int theY, PlayerInfo& thePlayer, Qt::GlobalColor theColor, const int anOtherRating1, const int anOtherRating2) {
  QGraphicsEllipseItem* aCirc = theScene->addEllipse(positionByRating(thePlayer.myRating) - 4, theY + 1, 8, 8);
  aCirc->setBrush(* new QBrush(theColor));
  // compute before or after the nick must be positioned
  bool isBefore = false;
  if (thePlayer.myRating > anOtherRating1 && thePlayer.myRating > anOtherRating2)
    isBefore = true;
  else if (thePlayer.myRating > anOtherRating1 && thePlayer.myRating < anOtherRating2)
    isBefore = thePlayer.myRating - anOtherRating1 < anOtherRating2 - thePlayer.myRating;
  else if (thePlayer.myRating > anOtherRating2 && thePlayer.myRating < anOtherRating1)
    isBefore = thePlayer.myRating - anOtherRating2 < anOtherRating1 - thePlayer.myRating;
  if (thePlayer.myRating == anOtherRating1 || thePlayer.myRating == anOtherRating2)
    isBefore = rand() % 2;

  if (thePlayer.myRating < 100) // there is a score after "0" rating
    isBefore = true;

  theScene->addText(thePlayer.myNick, QFont("Courier", 7))->setPos(positionByRating(thePlayer.myRating) + (isBefore ? -24 : 2) , theY - 5);
}

void MainWindow::onReportGraphics() {
  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());

  int aPlNum = myCore->PlayersRead();
  int aPlTourNum = 0;
  for(int a = 0; a < aPlNum; a++) {
    if (myCore->GamesThisTour(a) == 0)  // skip not-played players this tour
      aPlTourNum++;
  }
  // compute the needed size
  int aSumHeight = 0;
  for(int a = 0; a < aPlNum; a++) {
    if (myCore->GamesThisTour(a) == 0)  // skip not-played players this tour
      continue;
    aSumHeight += 22 + myCore->GamesThisTour(a) * 10;
  }

  QGraphicsScene* aScene = new QGraphicsScene(QRectF(0, 0, kTotalW + kTotalShift * 2, aSumHeight + 10), this);

  // draw games rectangle before the vertical and other lines: background
  int a;
  int aY0 = 0; // current position of graphics by Y for the current player
  for(a = 0; a < aPlNum; a++) {
    if (myCore->GamesThisTour(a) == 0)  // skip not-played players this tour
      continue;
    aY0 += 22;
    // colored rectanges: blue are lose, green are win
    int aPrevDate = 0;
    for(int aGame = 0; aGame < myCore->GamesThisTour(a); aGame++) {
      GameInfo anInfo = myCore->getGame(a, aGame);
      int aLevelGreen = anInfo.myWinCoef > 0 ? anInfo.myWinCoef * 128 : 0;
      int aLevelBlue = anInfo.myWinCoef < 0 ? (-anInfo.myWinCoef) * 128 : 0;
      QColor aColor(255 - aLevelGreen - aLevelBlue, 255 - aLevelBlue, 255 - aLevelGreen);
      aScene->addRect(kTotalShift, aY0, kTotalW, 10, * new QPen(aColor))->setBrush(* new QBrush(aColor));
      aY0 += 10;
      // if date is changed, draw a grey separator
      if (aPrevDate != anInfo.myDate) {
        aPrevDate = anInfo.myDate;
        if (aGame != 0)  {
          aScene->addLine(kTotalShift, aY0 - 11, kTotalW + kTotalShift * 2, aY0 - 11, QPen(Qt::darkGray));
        }
      }
    }
  }

  // the grid vertical lines
  for(int aRating = 0; aRating <= 1000; aRating += 100) {
    aScene->addLine(positionByRating(aRating), 0, positionByRating(aRating), aSumHeight, QPen(Qt::darkGray));
  }
  aScene->addLine(positionByRating(0), aSumHeight, positionByRating(1000), aSumHeight, QPen(Qt::darkGray));

  aY0 = 0; // current position of graphics by Y for the current player
  int aNumDisplayed = 0;
  for(a = 0; a < aPlNum; a++) {
    if (myCore->GamesThisTour(a) == 0)  // skip not-played players this tour
      continue;
    aNumDisplayed++;
    // main rectangle with general players info
    QGraphicsRectItem* aRect = aScene->addRect(kTotalShift, aY0, kTotalW, 22); // 22 pix height
    aRect->setBrush(* new QBrush(QColor(240, 160, 160)));
    QString aTitleStr = QString("%1 ").arg(aNumDisplayed) + QTextCodec::codecForName("CP1251")->toUnicode(myCore->Name(a).c_str());
    aTitleStr += QString(" (%1)").arg(myCore->Nick(a));
    aScene->addText(aTitleStr, QFont("Times", 10))->setPos(kTotalShift + 10, aY0);
    GameInfo anInfo = myCore->getGame(a, 0);
    QString aStatisticsStr = QTextCodec::codecForName("CP1251")->toUnicode("Поб %1 Пор %2 Игр %3 Скам %4").arg(anInfo.myThis.myWins).arg(anInfo.myThis.myLoses).arg(anInfo.myThis.myGames).arg(anInfo.myThis.myUnluck);
    aScene->addText(aStatisticsStr, QFont("Times", 10))->setPos(kTotalShift + 10 + 400, aY0);
    QString aRatingStr = QString(" %1").arg(int(myCore->Raiting(a, true, true))); // the final rating anyway
    aScene->addText(aRatingStr, QFont("Times", 10))->setPos(kTotalShift + 10 + 750, aY0);


    int aThisRatingPos = positionByRating(myCore->Raiting(a, true));
    aY0 += 22;
    int aY = aY0;
    QGraphicsEllipseItem* aCirc = aScene->addEllipse(aThisRatingPos - 2, aY0 - 2, 5,  10 * myCore->GamesThisTour(a));
    int aThisFinalPos = positionByRating(myCore->Raiting(a, true, true));
    if (aThisFinalPos != aThisRatingPos) { // draw a rating with penalty by red ellipse
      QGraphicsEllipseItem* aCircRed = aScene->addEllipse(aThisFinalPos - 2, aY0 - 2, 5,  10 * myCore->GamesThisTour(a));
      aCircRed->setBrush(* new QBrush(Qt::red));
    } else { // normal color, no penalty
      aCirc->setBrush(* new QBrush(Qt::green));
    }
    // display information about games of this player
    for(int aGame = 0; aGame < myCore->GamesThisTour(a); aGame++) {
      GameInfo anInfo = myCore->getGame(a, aGame);
      // score
      QGraphicsTextItem* aScoreText = aScene->addText(anInfo.myScore.c_str(), QFont("Times", 6));
      aScoreText->setPos(kTotalShift + kTotalW, aY0 - 4);
      aScoreText->setDefaultTextColor(anInfo.myWin ? Qt::darkGreen : Qt::darkBlue);
      // other players
      if (anInfo.isSingle) {
        drawPlayer(aScene, aY0, anInfo.myContra1, Qt::magenta, anInfo.myThis.myRating, anInfo.myThis.myRating);
      } else {
        drawPlayer(aScene, aY0, anInfo.myAlly, Qt::green, anInfo.myContra1.myRating, anInfo.myContra2.myRating);
        drawPlayer(aScene, aY0, anInfo.myContra1, Qt::magenta, anInfo.myAlly.myRating, anInfo.myContra2.myRating);
        drawPlayer(aScene, aY0, anInfo.myContra2, Qt::magenta, anInfo.myContra1.myRating, anInfo.myAlly.myRating);
      }
      aY0 += 10;
    }
    // small circles
    for(int aGame = 0; aGame < myCore->GamesThisTour(a); aGame++) {
      GameInfo anInfo = myCore->getGame(a, aGame);
      int aThisPossibleRating = positionByRating(anInfo.myThis.myRating);
      aScene->addEllipse(aThisPossibleRating - 2, aY + 3, 4, 4)->setBrush(* new QBrush(Qt::green));
      aY += 10;
    }
  }

  QGraphicsView* aView = new QGraphicsView(aScene);
  aLayout->addWidget(aView, 0, 0);
  aView->show();
}

void MainWindow::onReportM() {
  std::string aHTML = myCore->magneticReport();
  centralWidget()->hide();
  setCentralWidget(new QWidget());
  QGridLayout* aLayout = new QGridLayout(centralWidget());

  QWebView* aWebView = new QWebView(this);
  QString aQHTML(QTextCodec::codecForName("CP1251")->toUnicode(aHTML.c_str()));
  aWebView->setHtml(aQHTML);
  aLayout->addWidget(aWebView, 0, 0);
}

MainWindow::~MainWindow()
{

}
