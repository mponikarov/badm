#include <math.h>

#include "readgames.h"
#include "results.h"

#include <iostream>

using namespace std;

long double CalculateBadness(Game* theFirst, ofstream& tada, Results* theResults, const bool theThisTour) {
  Game* aCurrentGame;
  long double aResult = 1.;
  long double aPow = 2.; // increased significensy of the games -> reduce the power
  //tada<<"Calculate badness"<<endl;
  for(aCurrentGame = theFirst; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    if (!aCurrentGame->ThisTour() && theThisTour)
      continue;
    if (!theThisTour) {
      aPow = pow(aPow, (long double)0.99);
	  //tada<<"pow = "<<aPow<<endl;
      aResult *= pow(aCurrentGame->GetProbability(tada, theResults), aPow) * 10000.;
    } else {
      aResult *= aCurrentGame->GetProbability(tada, theResults) * 10000.;
    }
	  //tada<<"aResult="<<aResult<<endl;
  }
  return aResult;
}

void ReadGames(char* theFile, Game*& theFirstGame, Players& thePlayers, ofstream& tada, 
               int& theCoeff, int& theDate, std::map<char, std::map<char, std::map<char, int> > >& theMisfCoeffs,
               bool isThisTour) {
  tada<<"Reading file "<<theFile<<endl;
  Game *aCurrentGame = 0;
  FILE* f = fopen(theFile, "rt");
  if (f == NULL) {
	  tada<<"Read file failed"<<endl;
	  return;
  }
  static char aDat[100]; // buffer to read
  while(!feof(f)) {
    // try to read date
    while(!feof(f)) {
      long int anInitPos = abs(ftell(f)); // position before reading
      fscanf(f, "%c", &(aDat[0]));
      if (aDat[0] ==' ' || aDat[0] =='\n' || aDat[0] =='\r' || aDat[0] == 0)
        continue;
      if (!feof(f) && aDat[0] <='9' && aDat[0] >= '0') { // digit, continue
        int a;
        for(a = 1; !feof(f) && aDat[a - 1] !=' ' && aDat[a - 1] !='\n' && aDat[a - 1] !='\r' && aDat[a - 1] !=0; a++)
          fscanf(f, "%c", &(aDat[a]));
        aDat[a - 1] = 0;
        int aRes = atoi(aDat);
        theDate = aRes; // date
        tada<<"Set date "<<theDate<<endl;
        theCoeff += theDate;
        thePlayers.DayEnd();
      } else { // start of acronym: out the character back to read game correctly
        if (!feof(f) && aDat[0] !='x') {
          fseek(f, anInitPos, SEEK_SET);
        }
        break;
      }
    }
    if (aDat[0] == 'x') { // end of frame: missed list
      thePlayers.FrameEnd();
      if (aCurrentGame) aCurrentGame->SetLastFrame(true);
      Game::ReadNick(aDat, f);
      while(aDat[0]) {
        Player* aUnlucky = thePlayers.GetByNick(aDat);
        if (!aUnlucky) {
          aUnlucky = new Player(aDat);
          thePlayers.Add(aUnlucky);
        }
        aUnlucky->Unlucky(theMisfCoeffs[aDat[0]][aDat[1]][aDat[2]]);
        if (isThisTour)
          aUnlucky->IncUnluckyThisTour();
        Game::ReadNick(aDat, f);
      }
      theCoeff += 1;
      continue; // go to check date
    }
    Game* aGame = new Game(f, thePlayers, tada);
	//tada<<"***** new game readed "<<aGame<<" next="<<aGame->Next()<<endl;
    if (aGame->GetPlayer(0) == 0) break; // cannot read anymore
    aGame->SetDate(theDate);
    aGame->SetThisTour(isThisTour);
    if (aCurrentGame) aCurrentGame->SetNext(aGame);
    else if (theFirstGame == 0) theFirstGame = aGame;
    else { // iterate to the end of games
      for(aCurrentGame = theFirstGame; aCurrentGame->Next(); aCurrentGame = aCurrentGame->Next());
      aCurrentGame->SetNext(aGame);
    }
    if (aCurrentGame && aCurrentGame->Date() != aGame->Date()) aCurrentGame->SetLastInDay(true);
    aCurrentGame = aGame;
  }
  fclose(f);
  // statistics
  int aGames = 0, aPlayers = 0;
  for(aCurrentGame = theFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    aGames++;
  }
  tada<<"Now collected data: "<<thePlayers.Num()<<" players; "<<aGames<<" games"<<endl;
}

void CalculateRatings(Game*& theFirstGame, Players& thePlayers, ofstream& tada, Results* theResults, const bool theThisTour) {
  // initial ratings are 500
  theResults->SetInitialRating(500.);
  int aPSize = thePlayers.Num();
  int aDelta = 32; // found the best number on the real numbers
  int aMinModifNum = 10; // allowed number of minimal modifications
  // calculate ratings
  Game *aCurrentGame = 0;
  tada<<"Start ratings calculation"<<endl;
  long double aPropability = CalculateBadness(theFirstGame, tada, theResults, theThisTour);; // general propability of the current sortings: the optimization function value
  int anIterations = 0;
  bool aModified = 1;
  while(aModified) {
    long double aTotalProp = CalculateBadness(theFirstGame, tada, theResults, theThisTour);
    if (aPropability > aTotalProp) { // it means that locally good changes cause worse result in general
      if (aDelta == 1) {
        aMinModifNum--;
        if (aMinModifNum == 0)
          break;
      } else aDelta /= 2;
      tada<<"Local changes cause worse result in global "<<aPropability<<" > "<<aTotalProp<<endl;
      aPropability = aTotalProp;
    } else {
      aPropability = aTotalProp;
    }
    int* aDeltas = new int[aPSize]; // before calculate deltas, then apply simultaneously
    aModified = 0;
    for(int aPNum = 0; aPNum < aPSize; aPNum++) {
      //tada<<"Try to update rating of player "<<aCurrentPlayer->Nick()<<endl;
      Player* aPlayer = thePlayers.Get(aPNum);
      double aRat = theResults->Rating(aPlayer) + aDelta;
      theResults->SetRating(aPlayer, aRat); // increase rating
      long double aProp = CalculateBadness(theFirstGame, tada, theResults, theThisTour);
      if (aRat <= 1000 && aProp > aPropability) {
        aDeltas[aPNum] = + aDelta;
        aModified = 1;
        //tada<<"Increase raiting "<<thePlayers.Get(aPNum)->Nick()<<" by "<<aDelta<<" up to "<<thePlayers.Get(aPNum)->Rating(theThisTour)<<endl;
        //continue;
        theResults->SetRating(aPlayer, aRat - aDelta); // leave rating
      } else {
        aRat = aRat - 2 * aDelta;
        theResults->SetRating(aPlayer, aRat); // decrease rating
        aProp = CalculateBadness(theFirstGame, tada, theResults, theThisTour);
        if (aProp > aPropability && aRat > 0) {
          aDeltas[aPNum] = -aDelta;
          aModified = 1;
          //tada<<"Decrease raiting "<<thePlayers.Get(aPNum)->Nick()<<" by "<<aDelta<<" down to "<<thePlayers.Get(aPNum)->Rating(theThisTour)<<endl;
          //continue;
        } else { // no modification
          aDeltas[aPNum] = 0;
        }
        theResults->SetRating(aPlayer, theResults->Rating(aPlayer) + aDelta); // leave rating
      }
    }
    // now set the calculated deltas
    for(int aPNum = 0; aPNum < aPSize; aPNum++) {
      theResults->SetRating(thePlayers.Get(aPNum), theResults->Rating(thePlayers.Get(aPNum)) + aDeltas[aPNum]); // leave rating
    }
    if (!aModified && aDelta > 1) {
      aModified = 1;
      aDelta /= 2;
    }
    tada<<anIterations++<<" prop="<<aPropability<<" aDelta="<<aDelta<<endl;
  }
  tada<<"A rating "<<(theThisTour ? "(this tour) " : "whole")<<" propab="<<aPropability<<endl;
  for(int aPNum = 0; aPNum < thePlayers.Num(); aPNum++) {
    tada<<theResults->Rating(thePlayers.Get(aPNum))<<" "<<thePlayers.Get(aPNum)->Nick()<<endl;
  }

  theResults->SortByRating();
}

void CalculateRatingsDiscrete(Game*& theFirstGame, Players& thePlayers, ofstream& tada, Results* theResults, const bool theThisTour) {
  // initial ratings are 500
  theResults->SetInitialRating(500.);

  int aPSize = thePlayers.Num();
  for(int aPNum = 0; aPNum < aPSize; aPNum++) {
    thePlayers.Get(aPNum)->ResetGamesNum();
  }
  // calculate ratings by discrete formula (used in the tournament places definition)
  for(Game *aCurrentGame = theFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    if (!aCurrentGame->ThisTour() && theThisTour)
      continue;
    double aTeam1Score = aCurrentGame->GetSummScores(true);
    double aTeam2Score = aCurrentGame->GetSummScores(false);
    //double aTeam1Raiting = aCurrentGame->GetPlayer(0)->Rating() + aCurrentGame->GetPlayer(1)->Rating();
    //double aTeam2Raiting = aCurrentGame->GetPlayer(2)->Rating() + aCurrentGame->GetPlayer(3)->Rating();
    // another formula, computed to take care about strong with weak pair disbalance
    double aTeam1Raiting = aCurrentGame->GetTeamRaiting(true, theResults);
    double aTeam2Raiting = aCurrentGame->GetTeamRaiting(false, theResults);

    double aTeam1Relative = aTeam1Score * (aTeam1Raiting + aTeam2Raiting);
    double aTeam2Relative = aTeam2Score * (aTeam1Raiting + aTeam2Raiting);
    double aB1, aB2;

    int aNumPl; // singles (2) or doubles (4)
    double aDeltas[4]; // for both singles and doubles

    if (aCurrentGame->IsSingle()) {
      if (aCurrentGame->GetPlayer(0)->GamesNum() == 0 || aCurrentGame->GetPlayer(1)->GamesNum() == 0) {
        aB1 = aB2 = 4; // singles are more valuable
      } else {
        aB1 = sqrt(aCurrentGame->GetPlayer(0)->GamesNum() + 2.);
        aB2 = sqrt(aCurrentGame->GetPlayer(1)->GamesNum() + 2.);
      }
      aNumPl = 2;
      // singles are more valuable
      aB1 /= 1.25; aB2 /= 1.25;
      aDeltas[0] = (aTeam1Relative * theResults->Rating(aCurrentGame->GetPlayer(0)) / aTeam1Raiting - theResults->Rating(aCurrentGame->GetPlayer(0))) / aB1;
      aDeltas[1] = (aTeam2Relative * theResults->Rating(aCurrentGame->GetPlayer(1)) / aTeam2Raiting - theResults->Rating(aCurrentGame->GetPlayer(1))) / aB2;
    } else {
      if (aCurrentGame->GetPlayer(0)->GamesNum() == 0 || aCurrentGame->GetPlayer(1)->GamesNum() == 0 ||
        aCurrentGame->GetPlayer(2)->GamesNum() == 0 || aCurrentGame->GetPlayer(3)->GamesNum() == 0) {
          aB1 = aB2 = 4;
      } else {
        aB1 = sqrt(aCurrentGame->GetPlayer(0)->GamesNum() + aCurrentGame->GetPlayer(1)->GamesNum() + 2.);
        aB2 = sqrt(aCurrentGame->GetPlayer(2)->GamesNum() + aCurrentGame->GetPlayer(3)->GamesNum() + 2.);
      }
      aNumPl = 4;
      aDeltas[0] = (aTeam1Relative * theResults->Rating(aCurrentGame->GetPlayer(0)) / aTeam1Raiting - theResults->Rating(aCurrentGame->GetPlayer(0))) / aB1;
      aDeltas[1] = (aTeam1Relative * theResults->Rating(aCurrentGame->GetPlayer(1)) / aTeam1Raiting - theResults->Rating(aCurrentGame->GetPlayer(1))) / aB1;
      aDeltas[2] = (aTeam2Relative * theResults->Rating(aCurrentGame->GetPlayer(2)) / aTeam2Raiting - theResults->Rating(aCurrentGame->GetPlayer(2))) / aB2;
      aDeltas[3] = (aTeam2Relative * theResults->Rating(aCurrentGame->GetPlayer(3)) / aTeam2Raiting - theResults->Rating(aCurrentGame->GetPlayer(3))) / aB2;
    }
    // store new raitings and add number of played games to each player
    for(int aPlrNum = 0; aPlrNum < aNumPl; aPlrNum++) {
      tada<<"Player "<<aCurrentGame->GetPlayer(aPlrNum)->Nick()<<" "<<aDeltas[aPlrNum]<<endl;
      theResults->SetRating(aCurrentGame->GetPlayer(aPlrNum), theResults->Rating(aCurrentGame->GetPlayer(aPlrNum)) + aDeltas[aPlrNum]);
      aCurrentGame->GetPlayer(aPlrNum)->IncGamesNum();
    }
  }
  theResults->SortByRating();
  // output players sorted by raitings
  tada<<"Discrete computed raintings"<<endl;
  for(int aPNum = 0; aPNum < thePlayers.Num(); aPNum++) {
    tada<<theResults->Rating(thePlayers.Get(aPNum))<<" "<<thePlayers.Get(aPNum)->Nick()<<" games played "<<thePlayers.Get(aPNum)->GamesNum()<<endl;
  }
}
