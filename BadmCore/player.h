// class definition of player
// rating: integer [1;1000]
// nick is a name of player: usually 3 characters

#ifndef PLAYER_H
#define PLAYER_H

// to disable warnings about safety of printfs and others
#pragma warning(disable : 4996)

class Player {
  //double rating;
  //double finalRating; // the final rating with penalty for missed days
  //double thisTourRating;
  char* nick;
  Player* next;
  bool isHere; // is player here today or not?
  int misfortune; // scores of misfortune: on unluck: +=100, on end of frame and played -15, end of day -35
  bool justPassed; // true if the player is just passed game and can not pass one more
  int numgames; // number of played games in the tournament by this player before
  int numUnlucky; // number of unlucky not played games in the tournament by this player before
  bool playedInFrame;
  int playedInSingle; // participated in the single-game in this tour (number of times)
  bool myIsMagnetic; // means that players with this property must more likely play in pair trogether
public:
  Player(const char* theNick, const int theRating = 500);
  ~Player();

  char* Nick() const;

  void SetNext(Player* theNext) {next = theNext;}
  Player* Next() {return next;}
  Player* Last() {if (next) return next->Last(); return this;}
  
  void IsHere(bool theHere) {isHere = theHere;}
  bool IsHere() {return isHere;}

  const int Misfortune() const {return misfortune;}
  void Unlucky(const int theCoeff);
  void DayEnd();
  void FrameEnd();
  void PlayedInFrame() {playedInFrame = true;}
  bool isJustPassed() {return justPassed;}

  const int GamesNum() const {return numgames;}
  void IncGamesNum() {numgames++;}
  void ResetGamesNum() {numgames = 0;}
  const int UnluckyThisTour() const {return numUnlucky;}
  void IncUnluckyThisTour() {numUnlucky++;}

  void SetMagnetic(bool theMagnetic);
  bool IsMagnetic();

  void SetMagnetic(bool theMagnetic);
  bool IsMagnetic();

};
#endif
