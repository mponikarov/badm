#include "game.h"
#include <map>

void ReadGames(char* theFile, Game*& theFirstGame, Players& thePlayers, ofstream& tada, 
               int& theCoeff, int& theDate, std::map<char, std::map<char, std::map<char, int> > >& theMisfCoeffs,
               bool isThisTour);
void CalculateRatings(Game*& theFirstGame, Players& thePlayers, ofstream& tada, const bool theThisTour);
void SortByRating(Players& thePlayers, const bool theThisTour, const bool theFinalRating = false);
void CalculateRatingsDiscrete(Game*& theFirstGame, Players& thePlayers, ofstream& tada, const bool theThisTour);
