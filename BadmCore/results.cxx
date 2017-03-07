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
