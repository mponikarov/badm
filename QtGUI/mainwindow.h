#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets\QMainWindow>
#include <string>

class QGridLayout;
class Common;
class QCheckBox;
struct PlayerToday;
class QSpinBox;
class QLineEdit;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  Common* myCore; // algorithms
  QCheckBox** myPlsChecks; // checkboxes for players
  PlayerToday* myPls; // information about the selected players
  QSpinBox* myNumPlaces4; // number of places to game, GUI control
  QSpinBox* myNumPlaces2; // number of places to game, GUI control
  int myIntNumPlaces4; // store the number of 4-players places in int
  int myIntNumPlaces2; // store the number of 2-players places in int
  std::string* mySortingPlayers; // lines that will be stored for players (number = myNumPlaces value)
  QLineEdit** myScores; // final scores texts (number = myNumPlaces value)

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void onExit();
  void onSortings();
  void onSortNext();
  void onSortTotal();
  void onSortSave();
  void onReport();
  void onReportM();
  void onReportGraphics();

private:
  // creates initial layout presentation after all data reading: it contains the readed statistics information
  // and the buttons for the later actions
  void statisticsLayout(QGridLayout* theLayout);
};

#endif // MAINWINDOW_H
