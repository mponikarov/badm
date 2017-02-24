// General high-level API for simple accessing to the Badm core functionality

#include <string>

#ifdef EXPORTS_BADMCORE
#  define EXPORT_BADMCORE __declspec(dllexport)
#else
#  define EXPORT_BADMCORE __declspec(dllimport)
#endif

class Game;
class Players;
class Sample;

// storage for player that is today
struct PlayerToday {
  const char* myNick;
  std::string myFullName;
  bool myIsToday; // is this player plays today
  int myMisfortune; // misfortune coeff (the higher, the higher chance to play)
};

// game sortings
struct GameToday {
  bool isSingle; // true if only 2 players are in this game: p0 and p1
  const char* myNicks[4]; // p0 p1 vs p2 p3
  std::string myPropScore; // propable score using the current ratings
};

// information about the player in the game
struct PlayerInfo {
  int myRating;
  const char* myNick;
  int myWins, myLoses, myGames, myUnluck;
};

struct GameInfo {
  bool isSingle; // true if only 2 players are in this game: myThis and myContra1
  PlayerInfo myThis; ///< this player
  PlayerInfo myAlly; ///< in team of this player
  PlayerInfo myContra1; ///< first contra
  PlayerInfo myContra2; ///< first contra
  std::string myScore; ///< score information
  double myWinCoef; ///< win coefficient of this player: lose [-1; 0], win [0; 1]
  bool myWin; ///< true if myThis wins in this game
  int myDate; ///< date of this game
};

class EXPORT_BADMCORE Common {
  Game* myFirstGame; ///< pointer to the first game in the list
  Players* myPlayers; ///< players presented in all games 
  Sample* mySample; ///< results of sortings
  int myCurrentDate; ///< the date of the latest game

public:
  /// General constructor
  Common();

  /// Reads all data from different files
  void ReadAll();
  int GamesRead();
  int PlayersRead();
  /// Returns the long name of a player by the index
  std::string& Name(const int theIndex);
  /// Returns the long name of a player by the index
  const char* Nick(const int theIndex);
  /// Returns the raiting of the player by the index (base is 500)
  /// theFinal means with penalty
  double Raiting(const int theIndex, const bool theThisTour, const bool theFinal = false);
  /// Returns the raiting of the player by nick
  double Raiting(const char* theNick, const bool theThisTour);
  /// Returns the number of games played by the player in this tour
  int GamesThisTour(const int theIndex);
  /// Prepares the "today" players info: all players, on/off by the "today" file.
  /// The result array size will be equal to PlayersRead() + 10 (for new players), no-players have empty nick.
  int PreparePlay(PlayerToday*& thePlayers);
  /// Does the sorting. thePlayers contains information who is really checked
  /// and theNumPools4 is number of games of 4-players
  /// and theNumPools2 is number of games of 2-players
  void sort(PlayerToday* thePlayers, const int theNumPools4, const int theNumPools2);
  /// One of the result of sorted [0;theNumPools-1]
  GameToday sorted(int theIndex);
  /// Returns the string of nicks who is out of sortings
  std::string unlucky();
  /// Stores the reports in files
  std::string saveReport();
  /// Returns the information about the game of the player
  GameInfo getGame(const int thePlayerIndex, const int theGameNum);
};
