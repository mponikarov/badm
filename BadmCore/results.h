/// results of player at some time and using some rating calculation method

#ifndef RESULTS_H
#define RESULTS_H

#include "player.h"
#include <map>
#include <list>

class Results {
  std::map<Player*, double> ratings; ///< player to rating 
  std::list<Player*> sorted; ///< sorted players by the rating
  double initRating; ///< initial rating if nothing is defined for the player
public:
  static const int NO_POSITION = -1000; // means that this player is not in this results table

  void SetRating(Player* thePlayer, const double theRating);
  double Rating(Player* thePlayer) const;
  // sets rating for all players which has no defined rating yet
  void SetInitialRating(const double theRating);
  // sorts players by rating
  void SortByRating();
  // position in the positions table (result is started from 1)
  int Position(const Player* thePlayer) const;
  // Returns players in the sorted order
  const std::list<Player*>& Sorted() {return sorted;} ///< sorted players by the rating
};

#endif
