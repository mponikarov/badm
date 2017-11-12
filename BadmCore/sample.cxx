#include "sample.h"
#include "results.h"

// number of variants counted
#define VARS_NUM 100
#define SUB_VARS_NUM 200

#include <iostream>
#include <math.h>

using namespace std;

#define Max(x,y) (((x) > (y))?(x):(y))

static int GenerateID(const int thePlayer1, const int thePlayer2) {
  if (thePlayer1 == 0 || thePlayer2 == 0) return 0;
  return (thePlayer1 < thePlayer2)? (thePlayer1 * MAX_PLAYERS + thePlayer2) : (thePlayer2 * MAX_PLAYERS + thePlayer1);
}

static void DecodeID(const int theID, int& thePlayer1, int& thePlayer2) {
  thePlayer1 = theID % MAX_PLAYERS;
  thePlayer2 = theID / MAX_PLAYERS;
}

Sample::Sample(Players& thePlayers, Game* theFirstGame, const int thePlaces4, const int thePlaces2, ofstream& tada,
  Results* theResThis, Results* theResOld)
{
  myResThis = theResThis;
  myResOld = theResOld;
  Init(thePlayers, theFirstGame, thePlaces4, thePlaces2, tada);
}

const double Sample::TeamBadness(Players& theVariant, const int theStartIndex, bool isSingle, ofstream& tada, bool dump)
{
  double aResult = 0.;
  int aP0 = indexes[theVariant.Get(theStartIndex)];
  int aP1 = indexes[theVariant.Get(theStartIndex + 1)];

  if (isSingle)  {
    if (dump) {
      tada<<theVariant.Get(theStartIndex)->Nick()<<" vs "<<theVariant.Get(theStartIndex + 1)->Nick()<<endl;
    }
    aResult += myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP1);
    aResult += myBads.badness(Badness::BAD_CONTRASINGLE, aP0, aP1);
    if (dump)
      tada<<"Contra badness: 0vs1 "<<myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP1)<<" + total match: "<<myBads.badness(Badness::BAD_CONTRASINGLE, aP0, aP1)<<endl;

    aResult += myBads.badness(Badness::BAD_SINGLES, aP0); // twice at near frames is bad
    aResult += myBads.badness(Badness::BAD_SINGLES, aP1); // twice at near frames is bad
    if (dump)
      tada<<"Singless badness: 0and1 "<<myBads.badness(Badness::BAD_SINGLES, aP0)<<" + "<<myBads.badness(Badness::BAD_SINGLES, aP1)<<endl;

    for(int aThis = 0; aThis < 2; aThis++) {
      Results* aResults = (aThis == 1) ? myResThis : myResOld;
      double aP1R = aResults->Rating(theVariant.Get(theStartIndex));
      double aP2R = aResults->Rating(theVariant.Get(theStartIndex + 1));

      double aMiddle = (aP1R + aP2R) / 2.;
      double aRes = ((aP1R - aMiddle) * (aP1R - aMiddle) + (aP2R - aMiddle) * (aP2R - aMiddle)) / aMiddle / aMiddle / 2.; // less than 5 is too small, so 2
      if (!aThis) {// not this tour badness causes only half badness
        aRes /= (sqrt(1. * theVariant.Get(theStartIndex)->GamesNum() * theVariant.Get(theStartIndex + 1)->GamesNum()) * 1.25 + 3); // less than 3 is not enough
      }

      if (dump)
        tada<<"ThisTour "<<aThis<<" ratings "<<aP1R<<" vs "<<aP2R<<" badness="<<aRes<<endl;
      aResult += aRes;
    }
  } else {
    int aP2 = indexes[theVariant.Get(theStartIndex + 2)];
    int aP3 = indexes[theVariant.Get(theStartIndex + 3)];
    if (dump) {
      tada<<theVariant.Get(theStartIndex)->Nick()<<" "<<theVariant.Get(theStartIndex + 1)->Nick()<<" vs "<<theVariant.Get(theStartIndex + 2)->Nick()<<" "<<theVariant.Get(theStartIndex + 3)->Nick()<<endl;
    }
    aResult = myBads.badness(Badness::BAD_INTEAM, aP0, aP1);
    aResult += myBads.badness(Badness::BAD_INTEAM, aP2, aP3);
    if (dump)
      tada<<"InTeam badness: "<<myBads.badness(Badness::BAD_INTEAM, aP0, aP1)<<" and "<<myBads.badness(Badness::BAD_INTEAM, aP2, aP3)<<endl;
    aResult += myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP2);
    aResult += myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP3);
    aResult += myBads.badness(Badness::BAD_CONTRADOUBLE, aP1, aP2);
    aResult += myBads.badness(Badness::BAD_CONTRADOUBLE, aP1, aP3);
    if (dump)
      tada<<"Contra badness: 0vs2 "<<myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP2)<<" 0vs3 "<<myBads.badness(Badness::BAD_CONTRADOUBLE, aP0, aP3)<<" 1vs2 "<<myBads.badness(Badness::BAD_CONTRADOUBLE, aP1, aP2)<<" 1vs3 "<<myBads.badness(Badness::BAD_CONTRADOUBLE, aP1, aP3)<<endl;

    //double aRT1 = theVariant.Get(aStartIndex)->Rating() + theVariant.Get(aStartIndex + 1)->Rating();
    //double aRT2 = theVariant.Get(aStartIndex + 2)->Rating() + theVariant.Get(aStartIndex + 3)->Rating();
    // another formula, computed to take care about strong with weak pair disbalance
    for(int aThis = 0; aThis < 2; aThis++) {
      Results* aResults = (aThis == 1) ? myResThis : myResOld;
      double aP1R = aResults->Rating(theVariant.Get(theStartIndex));
      double aP2R = aResults->Rating(theVariant.Get(theStartIndex + 1));
      double aRT1 = (1500. * aP1R - aP1R * aP1R + 1500. * aP2R - aP2R * aP2R) / (3000. - aP1R - aP2R);
      aP1R = aResults->Rating(theVariant.Get(theStartIndex + 2));
      aP2R = aResults->Rating(theVariant.Get(theStartIndex + 3));
      double aRT2 = (1500. * aP1R - aP1R * aP1R + 1500. * aP2R - aP2R * aP2R) / (3000. - aP1R - aP2R);

      double aMiddle = (aRT1 + aRT2) / 2.;
      double aRes = ((aRT1 - aMiddle) * (aRT1 - aMiddle) + (aRT2 - aMiddle) * (aRT2 - aMiddle)) / aMiddle / aMiddle / 2.; // less than 5 is too small, so 2
      if (!aThis) {// not this tour badness causes only half badness
        aRes /= (sqrt(sqrt(1. * theVariant.Get(theStartIndex)->GamesNum() * theVariant.Get(theStartIndex + 1)->GamesNum() *
			theVariant.Get(theStartIndex + 2)->GamesNum() * theVariant.Get(theStartIndex + 3)->GamesNum())) * 1.25 + 3); // less than 3 is not enough
	  }

      if (dump)
        tada<<"ThisTour "<<aThis<<" ratings "<<aRT1<<" vs "<<aRT2<<" badness="<<aRes<<endl;
      aResult += aRes;
    }
    // bonus for magnetism
    if (theVariant.Get(theStartIndex)->IsMagnetic() && theVariant.Get(theStartIndex + 1)->IsMagnetic()) {
      aResult -= 0.5;
      if (dump)
        tada<<"Magnetic pair "<<-0.5<<endl;
    }
    if (theVariant.Get(theStartIndex + 2)->IsMagnetic() && theVariant.Get(theStartIndex + 3)->IsMagnetic()) {
      aResult -= 0.5;
      if (dump)
        tada<<"Magnetic pair "<<-0.5<<endl;
    }
  }

  // if the player is not just passed, increase the badness significantly
  for(int a = 0; a < (isSingle ? 2 : 4); a++) {
    if (!theVariant.Get(theStartIndex + a)->isJustPassed()) {
      aResult += 10.;
      if (dump)
        tada<<"Player "<<theVariant.Get(theStartIndex + a)->Nick()<<" is not just passed, +10"<<endl;
    }
  }
  if (dump)
	  tada<<"Total badness "<<aResult<<endl;
  return aResult;
}

void Sample::Init(Players& thePlayers, Game* theFirstGame, const int thePlaces4, const int thePlaces2, ofstream& tada) {
  Players aPlayers;
  aPlayers.TakeCopy(thePlayers); // some players will be removed: use comput to organize everything
  myPlaces4 = thePlaces4;
  myPlaces2 = thePlaces2;
  //cout<<"Is sample set number of places = "<<myPlaces<<endl;
  int aNumberOfPlayers = myPlaces2 * 2 + myPlaces4 * 4;
  int aPNum, aNumberOfHere = 0;
  for(aPNum = 0; aPNum < aPlayers.Num(); aPNum++) if (aPlayers.Get(aPNum)->IsHere()) aNumberOfHere++;
  //cout<<"In sample a number of here = "<<aNumberOfHere<<endl;
  if (aNumberOfHere < aNumberOfPlayers)  // reduce number of places
    return Init(aPlayers, theFirstGame, myPlaces4 - (myPlaces2 > 0 ?  0 : 1), myPlaces2 - (myPlaces2 > 0 ?  1 : 0), tada);

  // try to count all variants: to omit too constant fixes, mix player numbers in the players list
  for(int aChangeNum = 0; aChangeNum < 1000; aChangeNum++)
    aPlayers.ExchangePair();

    for(aPNum = 0; aPNum < aPlayers.Num(); aPNum++)
    indexes[aPlayers.Get(aPNum)] = aPNum;

  // remove players with too small misfortune
  int aMinMisfortune = 0, aPlayersWithMisf = aNumberOfHere;
  while(aPlayersWithMisf >= aNumberOfPlayers && aMinMisfortune < 5000) {
    aMinMisfortune++;
    aPlayersWithMisf = 0;
    for(aPNum = 0; aPNum < aPlayers.Num(); aPNum++) {
      Player* aP = aPlayers.Get(aPNum);
      if (aP->IsHere() && (aP->isJustPassed() || aP->Misfortune() >= aMinMisfortune))
        aPlayersWithMisf++;
    }
  }
  aMinMisfortune--;
  int aPlMinMisfortune = 0; // players with minimal misfortune (these players only could be omitted in the variant)
  for(aPNum = 0; aPNum < aPlayers.Num(); aPNum++) {
    Player* aP = aPlayers.Get(aPNum);
    if (aP->IsHere() && (aP->isJustPassed() || aP->Misfortune() >= aMinMisfortune)) {
      if (aP->Misfortune() == aMinMisfortune)
        aPlMinMisfortune++;
    } else {
      if (aP->IsHere()) aNumberOfHere--;
      aPlayers.Delete(aPNum);
      aPNum--;
    }
  }
  Players aVariant;
  aVariant.SetMinMisfortune(aMinMisfortune);
  int aPlMinMisfAllowed = aPlMinMisfortune - (aNumberOfHere - aNumberOfPlayers); // allowed number of used players with minimal misfortune
  // resort until first is player not with minimal misfortune
  if (aPlMinMisfortune != aNumberOfHere) {
    while(aPlayers.Get(0)->Misfortune() == aMinMisfortune)
      aPlayers.ExchangePair();
  }

  // update index of player in aPlayers by pointer
  indexes.clear();
  for(aPNum = 0; aPNum < aPlayers.Num(); aPNum++)
    indexes[aPlayers.Get(aPNum)] = aPNum + 1;

  // store badness for in-team pairs and contra pairs: ID of two player -> badness
  for(Game* aGame = theFirstGame; aGame != 0; aGame = aGame->Next()) {
    if (!aGame->ThisTour()) // only this tour is used for in and contra badness computation
      continue;
    if (aGame->IsLastInFrame()) {
      singlesLastFrame.clear();
    }

    int aPlayer0 = indexes[aGame->GetPlayer(0)];
    int aPlayer1 = indexes[aGame->GetPlayer(1)];
    if (aGame->IsSingle()) {
      if (aPlayer0 && aPlayer1) {
        myBads.contraSingle(aPlayer0, aPlayer1);
      }
      singlesLastFrame.insert(aPlayer0);
      singlesLastFrame.insert(aPlayer1);
    } else {
      int aPlayer2 = indexes[aGame->GetPlayer(2)];
      int aPlayer3 = indexes[aGame->GetPlayer(3)];
      if (aPlayer0 && aPlayer1) myBads.inTeam(aPlayer0, aPlayer1);
      if (aPlayer2 && aPlayer3) myBads.inTeam(aPlayer2, aPlayer3);
      if (aPlayer0 && aPlayer2) myBads.contraDouble(aPlayer0, aPlayer2);
      if (aPlayer0 && aPlayer3) myBads.contraDouble(aPlayer0, aPlayer3);
      if (aPlayer1 && aPlayer2) myBads.contraDouble(aPlayer1, aPlayer2);
      if (aPlayer1 && aPlayer3) myBads.contraDouble(aPlayer1, aPlayer3);
    }
    if (aGame->IsLastInDay())
      myBads.dayPassed();
    else if (aGame->IsLastInFrame())
      myBads.framePassed();
    // forget old badness (but not for singles badness)
    double aForgetCoeff = 1.;
    if (aGame->IsLastInFrame()) {
      aForgetCoeff = 0.9;
    }
  }
  // prepare a variant by variant to find the best and put it into myPlayers
  int* aVar = new int [aNumberOfHere]; // index of unused player at the position
  tada<<"Sorted players num "<<aNumberOfHere<<endl;
  aVar[0] = 0;
  int aNumInVariant;
  double aCurrentBadness = 0.;
  // initial variant
  for(aNumInVariant = 0; aNumInVariant < aNumberOfPlayers; aNumInVariant++) {
    aVariant.Add(aPlayers.Get(aNumInVariant));
    aVar[aNumInVariant] = aNumInVariant;
    aPlayers.SetUsed(aNumInVariant);
    if (IsLast(aNumInVariant))
      aCurrentBadness += TeamBadness(aVariant, aNumInVariant - (IsSingle(aNumInVariant) ? 1 : 3), IsSingle(aNumInVariant), tada, false);
    if (aPlayers.Get(aNumInVariant)->Misfortune() == aMinMisfortune && aVariant.PlMinMisf() > aPlMinMisfAllowed) {
      aNumInVariant++;
      break; // number of players with such misfortune is already enough
    }
  }
  for(int aN = aNumInVariant; aN < aNumberOfPlayers; aN++) { // nullify all next undefined
    aVariant.Reset(aN);
    aVar[aN] = -1;
  }
  double aBestBadness = aNumInVariant == aNumberOfPlayers ? aCurrentBadness : 1.e+100;
  myPlayers.TakeCopy(aVariant); // best variant yet
  int aCount = 0; // number of checked variants
  int aVar0Stop = myPlaces2 ? aNumberOfPlayers - 1 : 1;
  while(aVar[0] != aVar0Stop) { // if no singles first is allways at first place, iterate until it is so
    tada<<"Next version variant "<<aCurrentBadness<<endl;
    for(int aVarNum = 0; aVarNum < thePlaces4; aVarNum++)
      TeamBadness(aVariant, aVarNum * 4, false, tada, true);
    for(int aVarNum = 0; aVarNum < thePlaces2; aVarNum++)
      TeamBadness(aVariant, thePlaces4 * 4 + aVarNum * 2, true, tada, true);

    // searching for the next near variant
    aNumInVariant--;
    if (IsLast(aNumInVariant)) { // reduce current badness for this current team
      aCurrentBadness -= TeamBadness(aVariant, aNumInVariant - (IsSingle(aNumInVariant) ? 1 : 3), IsSingle(aNumInVariant), tada, false);
    }
    aCount++;
    while(aNumInVariant != aNumberOfPlayers && aVar[0] != aVar0Stop) {
      Player* anOld = aVariant.Get(aNumInVariant);
      if (anOld) // it could be zero for unitialized case, or old player at this place
        aPlayers.SetUnused(aVar[aNumInVariant]);
      aVariant.Set(aNumInVariant, 0);
      // take the next; for fully new it was -1, become 0
      bool isOK = false;
      int aNum = aVar[aNumInVariant];
      for(aNum++; !isOK; aNum++) {
        if (aNum >= aNumberOfHere) break; // too much, not OK
        if (aPlayers.IsUsed(aNum)) continue; // already used player
        if (aPlayers.Get(aNum)->Misfortune() == aMinMisfortune && aVariant.PlMinMisf() >= aPlMinMisfAllowed)
          continue; // number of players with such misfortune is already enough
        isOK = true;
        // check there are no repetition of variants
        if (!anOld) {
          if (IsSingle(aNumInVariant)) {
            switch(aNumInVariant % 2) {
              case 0: // first player in the game: must be bigger than first player of previous game (only if previous is also single)
                if (aNumInVariant && IsSingle(aNumInVariant - 2) && aNum < aVar[aNumInVariant - 2]) {
                  isOK = false;
                  aNum = aVar[aNumInVariant - 2];
                }
                break;
              case 1: // contra of the first player must have greater index than it
                if (aNum < aVar[aNumInVariant - 1]) {
                  isOK = false;
                  aNum = aVar[aNumInVariant - 1];
                }
                break;
            };
          } else {
            switch(aNumInVariant % 4) {
              case 0: // first player in the game: must be bigger than first player of previous game
                if (aNumInVariant && aNum < aVar[aNumInVariant - 4]) {
                  isOK = false;
                  aNum = aVar[aNumInVariant - 4];
                }
                break;
              case 1: // partner of the first player must have greater index than it
              case 3: // second contra must be greater than second contra
                if (aNum < aVar[aNumInVariant - 1]) {
                  isOK = false;
                  aNum = aVar[aNumInVariant - 1];
                }
                break;
              case 2: // first contra must be greater than first player
                if (aNum < aVar[aNumInVariant - 2]) {
                  isOK = false;
                  aNum = aVar[aNumInVariant - 2];
                }
                break;
            }
          }
        }
      }
      if (isOK) { // next variant for this player is selected correctly
        aVar[aNumInVariant] = aNum - 1; // last cycle iteration increases the counter
        aVariant.Set(aNumInVariant, aPlayers.Get(aVar[aNumInVariant]));
        aPlayers.SetUsed(aVar[aNumInVariant]);
        aNumInVariant++;
        if (IsSingle(aNumInVariant) ? (aNumInVariant % 2 == 0) : (aNumInVariant % 4 == 0)) {  // reduce current badness for newly created team
          double aTeamBadNess = TeamBadness(aVariant, aNumInVariant - (IsSingle(aNumInVariant - 1) ? 2 : 4), IsSingle(aNumInVariant - 1), tada, false);
          aCurrentBadness += aTeamBadNess;
          if (aNumInVariant == aNumberOfPlayers) { // try to select optimal variant
            aCount++;
            if (aCurrentBadness <= aBestBadness) {
              tada<<endl<<"*** Better variant with badness "<<aCurrentBadness<<endl;
              aBestBadness = aCurrentBadness;
              myPlayers.TakeCopy(aVariant);
              for(int aVarNum = 0; aVarNum < thePlaces4; aVarNum++)
                TeamBadness(aVariant, aVarNum * 4, false, tada, true);
              for(int aVarNum = 0; aVarNum < thePlaces2; aVarNum++)
                TeamBadness(aVariant, thePlaces4 * 4 + aVarNum * 2, true, tada, true);
            }
          } else if (aCurrentBadness >= aBestBadness) { // try to stop iteration before last team is formed
            aNumInVariant--;
            aCurrentBadness -= aTeamBadNess;
          }
        }
      } else {
        aVar[aNumInVariant] = -1;
        aNumInVariant--;
        if (IsLast(aNumInVariant)) { // reduce current badness for this current team
          aCurrentBadness -= TeamBadness(aVariant, aNumInVariant - (IsSingle(aNumInVariant) ? 1 : 3), IsSingle(aNumInVariant), tada, false);
        }
      }
    }
  }
  tada<<"Badness = "<<aBestBadness<<" number of checked variants = "<<aCount<<endl;
}

int Sample::GetPlaces4() {
  return myPlaces4;
}

int Sample::GetPlaces2() {
  return myPlaces2;
}

Game* Sample::GetGame(const int thePlace) { // all counted from zero
  if (thePlace < myPlaces4) {
    int aStart = thePlace * 4;
    return new Game(myPlayers.Get(aStart), myPlayers.Get(aStart + 1), myPlayers.Get(aStart + 2), myPlayers.Get(aStart + 3));
  } else {
    int aStart = myPlaces4 * 4 + (thePlace - myPlaces4) * 2;
    return new Game(myPlayers.Get(aStart), myPlayers.Get(aStart + 1));
  }
}

int Sample::NumInTeam(const int theNumInVariant)
{
  if (theNumInVariant < myPlaces4 * 4)
    return theNumInVariant % 4; // doubles
  return (theNumInVariant - myPlaces4 * 4) % 2; // singles
}

bool Sample::IsSingle(const int theNumInVariant)
{
  return theNumInVariant >= myPlaces4 * 4;
}

bool Sample::IsLast(const int theNumInVariant)
{
  if (theNumInVariant < myPlaces4 * 4)
    return theNumInVariant % 4 == 3; // doubles
  return theNumInVariant % 2 == 1; // singles
}
