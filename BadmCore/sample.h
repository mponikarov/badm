#ifndef SAMPLE_H
#define SAMPLE_H

#include "game.h"

// class for games organization: distribution players among the places
// p1 and p2 vs p3 and p4
class Sample {
  Players myPlayers; // array of result players: p1 + p2 vs p3 + p4 ; p5 + p6 vs p7 + p8 ; p9 vs p10 etc.: always first pairs, then singles
  int myPlaces4; // number of 4-memebers games
  int myPlaces2; // number of 2-memebers games
  map<int, double> inTeamBad, contraBad, contraSinglesBad, singlesBad;
  map<Player*, int> indexes; // cash index of player in aPlayers by pointer
public:
  Sample(Players& thePlayers, Game* theFirstGame, const int thePlaces4, const int thePlaces2, ofstream& tada);
  void Init(Players& thePlayers, Game* theFirstGame, const int thePlaces4, const int thePlaces2, ofstream& tada);
  int GetPlaces4(); // return the result number of places with 4 players
  int GetPlaces2(); // return the result number of places with singles
  Game* GetGame(const int thePlace); // all counted from zero
private:
  int NumInTeam(const int theNumInVariant); // returns a number of the player in the team by the simple index of player in the whole variant
  bool IsSingle(const int theNumInVariant); // returns a kind of game by the simple index of player in the whole variant
  bool IsLast(const int theNumInVariant); // returns true if the player is the last in game-variant by the simple index of player in the whole variant
  // returns the badness of one team that possible may play
  const double TeamBadness(Players& theVariant, const int theStartIndex, bool isSingle, ofstream& tada, bool dump = false);

};

#endif