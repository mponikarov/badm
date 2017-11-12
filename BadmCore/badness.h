#ifndef BADNESS_H
#define BADNESS_H

#include <map>

using namespace std;

// class for storage of badness of different combinations of players (in team, contra, participation in singles, etc)
class Badness {
public:
  enum BadnessKind {
    BAD_INTEAM, // in the same team
    BAD_CONTRADOUBLE, // in contra double-teams
    BAD_CONTRASINGLE, // in contra single-teams
    BAD_SINGLES, // participation in singles
  };

private:
  // number of players to compute the combination of indices of player effectively
  int myNbPlayers;
  // type of badness + index of player(s) -> current badness
  map<int, double> myBad;
  // type of badness + index of player(s) -> number of occurences and time period of the last occurence
  map<int, pair<int, double> > myOccur;

public:
  Badness();

  void inTeam(const int thePl1, const int thePl2);
  void contraDouble(const int thePl1, const int thePl2);
  // this also registers the singles badness for players separately
  void contraSingle(const int thePl1, const int thePl2);

  // increases the time past
  void framePassed();
  // increases the time past
  void dayPassed();

  // returns the badness
  double badness(BadnessKind theKind, const int thePl1, const int thePl2 = -1);

private:
  // computes the map unique key from 3 values
  int GenerateID(BadnessKind theKind, const int thePl1, const int thePl2);
  // computes the map unique key from 2 values
  int GenerateID(BadnessKind theKind, const int thePl1);
  // registers the new occurence
  void registerOccur(const int theID);
};

#endif
