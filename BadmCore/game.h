#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "players.h"

#include <fstream>
#include <map>

class Results;
using namespace std;

// class for game emulation
// p1 and p2 vs p3 and p4
// also game stores the result scores
class Game {
  bool mySingle; /// is tru if only two players presented
  Player* pls[4];
  int plays[3];
  int scores[6];
  Game* next;
  std::map<int, double> prb;
  int myDate; // date of a game in format DDMMYY
  bool myLastInFrame; // is the game last in frame
  bool myLastInDay; // is the dame last in day
  bool myThisTour; // if it is true, this game is checked as it is in this tour
  bool myIsDisabled; // used for intermediate computations of ratings, if it is "disabled", it is out of the iteration
public:
  bool IsSingle() {return mySingle;}

  // reads nick from file and stores if to buffer
  // if something is bad, sets zero length buffer
  static void ReadNick(char* theBuffer, FILE* file);

  Game(Player* p1, Player* p2);
  Game(Player* p1, Player* p2, Player* p3, Player* p4);
  Game(FILE* file, Players& thePlayers, ofstream&);// constructor for game results reading from the file
  // storess difference between scores of p1+p2 and p3+p4, play3 may be 0
  void SetScores(int thePlay1, int thePlay2, int thePlay3);
  // sets date
  void SetDate(const int theDate) {myDate = theDate;}
  void SetLastFrame(const bool theFrame) {myLastInFrame = theFrame;}
  bool IsLastInFrame() {return myLastInFrame;}
  void SetLastInDay(const bool theDay) {myLastInDay = theDay;}
  bool IsLastInDay() {return myLastInDay;}
  const int Date() const {return myDate;}

  Game* Next();
  void SetNext(Game* theNext) {next = theNext;}
  long double GetProbability(ofstream& tada, const Results* theResults); // returns the probability of the current game
  Player* GetPlayer(int anIndex);
  
  double GetWinDelta2(); // returns the delta to increase raiting in 2 sets
  double GetWinDelta3(); // returns the delta to increase raiting in 3 sets
  bool FirstWin(); // returns true if first team wins
  int NumWin(bool theFirst); // returns number of winned plays
  bool AreTogether(Player* theP1, Player* theP2); // returns true if players are played in one team
  bool AreContra(Player* theP1, Player* theP2); // returns true if players are played one vs another
  bool IsInGame(Player* theP); // returns true if player is in this game
  Player* Together(Player* theP); // returns the player that plays togother with the given
  Player* Contra1(Player* theP); // returns the first contra player of the given
  Player* Contra2(Player* theP); // returns the first contra player of the given
  bool IsThreeSets(); // returns true if there was 3 sets, not two
  bool IsInFirstTeam(Player* theP); // returns true if the player is in the first team
  bool IsInSecondTeam(Player* theP); // returns true if the player is in the second team
  
  // for scores computation like: points of team / summ points
  double GetSummScores(bool theFirstTeam);
  // returns the current raiting of first or second team (0-1000)
  double GetTeamRaiting(bool theFirstTeam, const Results* theResults);
  // real scores
  int Score(const int theNum) {return scores[theNum];}
  void SetThisTour(bool theThisTour) {myThisTour = theThisTour;}
  bool ThisTour() {return myThisTour;}

  // makes this (and if theAllNext is true, then all next games) as disabled=theFlag
  void setDisabled(const bool theFlag, const bool theAllNext = false);
  bool isDisabled();

  // computes the most possible rating of the given player if only this game is played and ratings of other players (in this tours) are like this
  int MostPossibleRating(Player* thePl, Results* theResults);
};

#endif
