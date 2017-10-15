#include "results.h"

void Results::SetRating(Player* thePlayer, const double theRating)
{
  ratings[thePlayer] = theRating;
}

double Results::Rating(Player* thePlayer) const
{
  std::map<Player*, double>::const_iterator aFind = ratings.find(thePlayer);
  if (aFind == ratings.end())
    return initRating;
  return aFind->second;
}

void Results::SetInitialRating(const double theRating)
{
  initRating = theRating;
  ratings.clear();
}

void Results::SortByRating()
{
  // just iterate map one by one to see maximum rating, then some smaller, etc and fill a list
  sorted.clear();
  double aMaxPrev = 1.e+100; // previous maximum
  for(bool aFound = true; aFound;) {
    aFound = false;
    double aMaxCurrent = -1.e+100;
    std::map<Player*, double>::iterator aPlIter;
    for(aPlIter = ratings.begin(); aPlIter != ratings.end(); aPlIter++) {
      if (aPlIter->second < aMaxPrev && aPlIter->second > aMaxCurrent) {
        aMaxCurrent = aPlIter->second;
        aFound = true;
      }
    }
    for(aPlIter = ratings.begin(); aPlIter != ratings.end(); aPlIter++) {
      if (aPlIter->second == aMaxCurrent) {
        sorted.push_back(aPlIter->first);
      }
    }
    aMaxPrev = aMaxCurrent;
  }
}

int Results::Position(const Player* thePlayer) const
{
  std::list<Player*>::const_iterator aSortIter = sorted.begin();
  for(int aPos = 1; aSortIter != sorted.end(); aPos++, aSortIter++) {
    if (*aSortIter == thePlayer) return aPos;
  }
  return NO_POSITION;
}

void Results::ApplyPenalties(std::map<Player*, int>& theGamesNum)
{
  // maximum number of games
  int aMax = 0;
  std::map<Player*, int>::iterator aGNumIter = theGamesNum.begin();
  for(; aGNumIter != theGamesNum.end(); aGNumIter++) {
    int aNum = aGNumIter->second + aGNumIter->first->UnluckyThisTour();
    if (aNum > aMax)
      aMax = aNum;
  }
  std::map<Player*, double>::iterator aPlIter = ratings.begin();
  for(; aPlIter != ratings.end(); aPlIter++) {
    Player* aP = aPlIter->first;
    int aGamesNum = aP->UnluckyThisTour();
    if (theGamesNum.find(aP) != theGamesNum.end())
      aGamesNum += theGamesNum[aP];
    gamesNum[aP] = aGamesNum;
    if (aGamesNum * 2 < aMax) {
      double aPercent = aGamesNum * 2. / aMax;
      aPlIter->second *= aPercent;
    }
  }
  maxGames = aMax;
}

double Results::AppliedPenalty(Player* thePlayer)
{
  int aPlGam = gamesNum[thePlayer];
  if (aPlGam * 2 < maxGames) {
    double aPercent = aPlGam * 2. / maxGames;
    return ratings[thePlayer] * (1. / aPercent - 1.);
  }
  return 0;
}

int Results::PenaltyGames(Player* thePlayer)
{
  int aPlGam = gamesNum[thePlayer];
  if (aPlGam * 2 < maxGames) {
    return maxGames / 2 - aPlGam;
  }
  return 0;
}
