// class definition of set of players (instead of the iteration)

#ifndef PLAYERS_H
#define PLAYERS_H

class Player;

#define MAX_PLAYERS 100

class Players {
  // array of players
  Player** myP;
  // number of already registered players
  int myNum;
  // stores the player is used in variant or not
  bool* myUsed;
  // summ of misfortunes of all players (updated in Add and Set methods)
  int myMinMisfortune;
  // number of players with min misfortune
  int myPlMinMisf;
public:
  // constructor by the name of players
  // if theNum == 0, it stores array with MAX_PLAYERS palyers
  Players(const int theNum = 0);
  ~Players();
  // Adds player
  void Add(Player* thePlayer);
  // Reset user at the position (sets to 0)
  void Reset(const int thePos);
  // Returns the number of players
  int Num() {return myNum;}
  // Returns the player by number
  Player* Get(const int theNum) {return myP[theNum];}
  // Returns number by player (slow function)
  const int GetID(const Player* thePlayer) const;
  // Returns player with this nick or null if not found
  Player* GetByNick(const char* theNick);
  // Returns the player which has number theNum between "here" players, unused players
  Player* GetUnusedPlayer(const int theNum);
  // Removes all players
  void Clear() {myNum = 0;}
  // CLears all "used" flags
  void ClearUsed();
  // Copyes all data from source
  void TakeCopy(Players& theSource);
  // Exchanges a pair randomly
  void ExchangePair();
  // Removes player from the array (with shift of others, changing my num, etc.)
  void Delete(const int theNum);
  // Sets player as unused
  void SetUnused(const int theNum) {myUsed[theNum] = false;}
  // Sets player as used
  void SetUsed(const int theNum) {myUsed[theNum] = true;}
  // Returns the used player status
  const bool IsUsed(const int theNum) const {return myUsed[theNum];}
  // Sets the player
  void Set(const int thePosition, Player* thePlayer);
  // sets the minimal misfortune (to count players with this misfortune)
  void SetMinMisfortune(const int theMin) {myMinMisfortune = theMin;}
  // returns the current number of players with minimal misfortune
  const int PlMinMisf() const {return myPlMinMisf;}
  // call frame end and day end for all players
  void DayEnd();
  void FrameEnd();
};

#endif
