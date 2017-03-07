#include "game.h"
#include <map>

class Results;

void ReadGames(char* theFile, Game*& theFirstGame, Players& thePlayers, ofstream& tada, 
               int& theCoeff, int& theDate, std::map<char, std::map<char, std::map<char, int> > >& theMisfCoeffs,
               bool isThisTour);
void CalculateRatings(Game*& theFirstGame, Players& thePlayers, ofstream& tada, Results* theResults, const bool theThisTour);
void CalculateRatingsDiscrete(Game*& theFirstGame, Players& thePlayers, ofstream& tada, Results* theResults, const bool theThisTour);
