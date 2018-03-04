#include <math.h>

#include "game.h"
#include "results.h"
#include <iostream>
#include <cstdlib>

#define ABS(x) ((x)>=0?(x):-(x))
#define MIN(x, y) (((x) < (y))?(x):(y))

using namespace std;

void Game::ReadNick(char* theBuffer, FILE* file)
{
  int aNum = 0;
  while(!feof(file)) {
    fscanf(file, "%c", &(theBuffer[aNum]));
    if (theBuffer[aNum] ==' ' || theBuffer[aNum] =='\n' || theBuffer[aNum] =='\r') {
      if (aNum != 0) // this may be a number for singles
        break;
      continue;
    }
    if (aNum == 0 && theBuffer[0] == 'x') { // end of missed list
      theBuffer[0] = 0;
      return;
    }
    aNum++;
    if (aNum == 3) {
      theBuffer[3] = 0;
      break;
    }
  }
  if (aNum != 3 && (theBuffer[0] < '0' || theBuffer[0] > '9')) theBuffer[0] = 0; // file is ended before end of nick
}

Game::Game(Player* p1, Player* p2)
{
  pls[0] = p1;
  pls[1] = p2;
  mySingle = true;
  next = 0;
  myIsDisabled = false;
}

Game::Game(Player* p1, Player* p2, Player* p3, Player* p4) {
  pls[0] = p1;
  pls[1] = p2;
  pls[2] = p3;
  pls[3] = p4;
  mySingle = false;
  next = 0;
  myIsDisabled = false;
}

int ReadInteger(FILE* f, char* theBuffer) {
  if (theBuffer[0] != 0) { // read from buffer first
    int aRes = atoi(theBuffer);
    theBuffer[0] = 0;
    return aRes;
  }
	static char s[10];
	fscanf(f, "%s", s);
	return atoi(s);
}

Game::Game(FILE* file, Players& thePlayers, ofstream& tada) {
  myLastInFrame = false; // default value
  myLastInDay = false;
  myIsDisabled = false;
  mySingle = false;
  int a;
  pls[0]=0; // to check if read is incorrect
  char aNick[4] = "unk";
  Player* aCurrent;
  for(a = 0; a < 4; a++) { // read four players nicks
    ReadNick(aNick, file);
    if (a == 2 && aNick[0] >= '0' && aNick[0] <= '9') { // single game detection
      mySingle = true;
      break;
    }
    tada<<"Nick '"<<aNick<<"'"<<endl;
    if (aNick[0] == 0) break;
    aCurrent = thePlayers.GetByNick(aNick);
    if (!aCurrent) {
      //tada<<"New player '"<<aNick<<"'"<<" theFirst="<<theFirst<<endl;
      aCurrent = new Player(aNick, 500);
      thePlayers.Add(aCurrent);
    }
    pls[a] = aCurrent;
    aCurrent->PlayedInFrame();
  }
  if (a == 4) aNick[0] = 0;
  if (!feof(file)) {
    int aScore1, aScore2, aPlayNum;
    for(aPlayNum = 0; aPlayNum < 3; aPlayNum++) {
      aScore1 = ReadInteger(file, aNick);
      tada<<"Score1="<<aScore1<<endl;
      aScore2 = ReadInteger(file, aNick);
      tada<<"Score2="<<aScore2<<endl;
      scores[aPlayNum * 2] = aScore1;
      scores[aPlayNum * 2 + 1] = aScore2;
      plays[aPlayNum] = aScore1 - aScore2;
      if (aPlayNum == 1) { // check there is a next scores
        if (plays[0] * plays[1] > 0) {
          plays[2] = 0;
          break;
        }
      }
    }
  }
  next = 0;
}

Game* Game::Next() {
  if (next && next->isDisabled()) // skip disabled in the iteration
    return next->Next();
  return next;
}

void Game::SetScores(int thePlay1, int thePlay2, int thePlay3) {
  plays[0] = thePlay1;
  plays[1] = thePlay2;
  plays[2] = thePlay3;
  // clear cash because of new scores
  prb.clear();
}

// number of compinations f and s in f+s places: (f+s)!/f!/s!
static long double fact(int theFirst, int theSecond) {
  static map<int, long double> aValues;
  int aMin = MIN(theFirst, theSecond);
  map<int, long double>::iterator anIter = aValues.find(aMin);
  if (anIter != aValues.end()) return anIter->second;
  long double aResult = 1.;
  int a;
  for(a = theFirst + theSecond; a > 21; a--) aResult *= a;
  for(a = 2; a <= aMin; a++) aResult /= a;
  aValues[aMin] = aResult;
  return aResult;
}

double Game::GetTeamRaiting(bool theFirstTeam, const Results* theResults)
{
  if (mySingle) {
    return theFirstTeam ? theResults->Rating(pls[0]) : theResults->Rating(pls[1]);
  }
  double aP1R = theFirstTeam ? theResults->Rating(pls[0]) : theResults->Rating(pls[2]);
  if (aP1R > 1000.) aP1R = 1000.;
  double aP2R = theFirstTeam ? theResults->Rating(pls[1]) : theResults->Rating(pls[3]);
  if (aP2R > 1000.) aP2R = 1000.;
  return (1500. * aP1R - aP1R * aP1R + 1500. * aP2R - aP2R * aP2R) / (3000. - aP1R - aP2R);
}

long double Game::GetProbability(ofstream& tada, const Results* theResults) {
  int r1 = (int)(GetTeamRaiting(true, theResults));
  int r2 = (int)(GetTeamRaiting(false, theResults));
  int r = 100000 * r1 / r2;
  //cout<<"r="<<r<<endl;
  map<int, double>::iterator anI = prb.find(r);
  if (anI != prb.end()) {
	  //tada<<"Just return from map "<<anI->second<<endl;
	  return anI->second;
  }
  double p1 = double(r1)/(r1 + r2); // possibility to win 1 score by first team
  double p2 = double(r2)/(r1 + r2); // possibility to win 1 score by second team
  //tada<<"p1="<<p1<<" p2="<<p2<<endl;
  int aSet;
  long double aResult = 1.;
  //cout<<" scores ";
  for(aSet = 0; aSet < 3; aSet++) {
    if (plays[aSet] == 0) continue;
    int s1, s2;
    if (plays[aSet] > 0) {
      s1 = 21;
      s2 = 21 - plays[aSet];
    } else {
      s2 = 21;
      s1 = 21 + plays[aSet];
    }
	//tada<<"s1="<<s1<<" s2="<<s2<<endl;
    long double pp = 1. - powl(p1, double(s1)) * powl(p2, double(s2));
	//tada<<"pp="<<pp<<endl;
    aResult *= 1. - pow(pp, fact(s1, s2));
	//tada<<"aResult="<<aResult<<endl;
  }
  prb[r] = aResult;
  return aResult;
}

Player* Game::GetPlayer(int theIndex) {
  return pls[theIndex];
}

Player* Game::Together(Player* theP) {
  if (mySingle)
    return 0;
  if (pls[0] == theP) return pls[1];
  if (pls[1] == theP) return pls[0];
  if (pls[2] == theP) return pls[3];
  if (pls[3] == theP) return pls[2];
  return 0; // not found
}

Player* Game::Contra1(Player* theP) {
  if (mySingle) {
    if (pls[0] == theP) return pls[1];
    if (pls[1] == theP) return pls[0];
    return 0;
  }
  if (pls[0] == theP) return pls[2];
  if (pls[1] == theP) return pls[3];
  if (pls[2] == theP) return pls[0];
  if (pls[3] == theP) return pls[1];
  return 0; // not found
}

Player* Game::Contra2(Player* theP) {
  if (mySingle)
    return 0;
  if (pls[0] == theP) return pls[3];
  if (pls[1] == theP) return pls[2];
  if (pls[2] == theP) return pls[1];
  if (pls[3] == theP) return pls[0];
  return 0; // not found
}

bool Game::IsInGame(Player* theP)
{
  return pls[0] == theP || pls[1] == theP || (!mySingle && (pls[2] == theP || pls[3] == theP));
}

bool Game::FirstWin() {
  if (plays[0] > 0 && plays[1] > 0) return true;
  if (plays[0] < 0 && plays[1] < 0) return false;
  return (plays[2] > 0);
}
int Game::NumWin(bool theFirst) {
  if (theFirst) {
    if (plays[0] > 0 && plays[1] > 0) return 2;
    if (plays[0] < 0 && plays[1] < 0) return 0;
    return plays[2] > 0 ? 2 : 1;
  }
  if (plays[0] > 0 && plays[1] > 0) return 0;
  if (plays[0] < 0 && plays[1] < 0) return 2;
  return plays[2] > 0 ? 1 : 2;
}

bool Game::AreTogether(Player* theP1, Player* theP2) {
  if (mySingle)
    return false;
  return ((theP1 == pls[0] && theP2 == pls[1]) || (theP1 == pls[1] && theP2 == pls[0]) ||
          (theP1 == pls[2] && theP2 == pls[3]) || (theP1 == pls[3] && theP2 == pls[2]));
}

bool Game::AreContra(Player* theP1, Player* theP2) {
  if (mySingle)
    return (pls[0] == theP1 && pls[1] == theP2) || (pls[0] == theP2 && pls[1] == theP1);

  return ((theP1 == pls[0] && theP2 == pls[2]) || (theP1 == pls[0] && theP2 == pls[3]) ||
          (theP1 == pls[1] && theP2 == pls[2]) || (theP1 == pls[1] && theP2 == pls[3]) ||
	  (theP1 == pls[2] && theP2 == pls[0]) || (theP1 == pls[2] && theP2 == pls[1]) ||
	  (theP1 == pls[3] && theP2 == pls[0]) || (theP1 == pls[3] && theP2 == pls[1]));
}

bool Game::IsInFirstTeam(Player* theP)
{
  return mySingle ? (theP == pls[0]) : (theP == pls[0] || theP == pls[1]);
}

bool Game::IsInSecondTeam(Player* theP)
{
  return mySingle ? (theP == pls[1]) : (theP == pls[2] || theP == pls[3]);
}

#define SET_WIN_BONUS 3
#define GAME_WIN_BONUS 5

double Game::GetSummScores(bool theFirstTeam) {
  int aSumm = 0, aScores = 0;
  for(int a = 0; a < 3; a++) {
    int aPlays = plays[a];
    if (!theFirstTeam) aPlays = -aPlays;
    if (a == 2 && aPlays == 0) continue;
    if (aPlays > 0) {
      aScores += 21 + SET_WIN_BONUS;
      aSumm += 21 + SET_WIN_BONUS + 21 - aPlays;
    } else {
      aScores += 21 + aPlays;
      aSumm += 21 + SET_WIN_BONUS + 21 + aPlays;
    }
  }
  if ((FirstWin() && theFirstTeam) || (!FirstWin() && !theFirstTeam)) aScores += GAME_WIN_BONUS;
  aSumm += GAME_WIN_BONUS;
  return double(aScores) / double(aSumm);
}

double Game::GetWinDelta2() {
  double aRaitingTeam1 = GetTeamRaiting(true, false);
  double aRaitingTeam2 = GetTeamRaiting(false, false);
  if (aRaitingTeam1 > aRaitingTeam2) { // first must win
    double aK = aRaitingTeam1 / (aRaitingTeam1 + aRaitingTeam2);
    double x = (21 * 2. + SET_WIN_BONUS * 2 + GAME_WIN_BONUS) * (1. - aK) / aK ;
    if (x > 19 * 2. + 1) x = 19 * 2. + 1;
    if (x < 1.) x = 1.;
    return 42. - x;
  }
  double aK = aRaitingTeam2 / (aRaitingTeam1 + aRaitingTeam2);
  double x = (21 * 2. + SET_WIN_BONUS * 2 + GAME_WIN_BONUS) * (1. - aK) / aK;
  if (x > 19 * 2. + 1) x = 19 * 2. + 1;
  if (x < 1) x = 1;
  return -(42. - x); // second must win: negative result
}

double Game::GetWinDelta3() {
  double aRaitingTeam1 = GetTeamRaiting(true, false);
  double aRaitingTeam2 = GetTeamRaiting(false, false);
  if (aRaitingTeam1 > aRaitingTeam2) { // first must win
    double aK = aRaitingTeam1 / (aRaitingTeam1 + aRaitingTeam2);
    double x = (21 * 3. + SET_WIN_BONUS * 2 + GAME_WIN_BONUS) * (1. - aK) / aK - SET_WIN_BONUS;
    if (x > 19 * 3 + 4) x = 19 * 3. + 4;
    if (x < 1.) x = 1.;
    return 63. - x;
  }
  double aK = aRaitingTeam2 / (aRaitingTeam1 + aRaitingTeam2);
  double x = (21 * 3. + SET_WIN_BONUS * 2 + GAME_WIN_BONUS) * (1. - aK) / aK - SET_WIN_BONUS;
  if (x > 19 * 3. + 4) x = 19 * 3. + 4;
  if (x < 1) x = 1;
  return -(63. - x); // second must win: negative result
}

void Game::setDisabled(const bool theFlag, const bool theAllNext)
{
  myIsDisabled = theFlag;
  if (theAllNext && next)
    next->setDisabled(theFlag, theAllNext);
}

bool Game::isDisabled()
{
  return myIsDisabled;
}

bool Game::IsThreeSets()
{
  return plays[0] * plays[1] < 0;
}

int Game::MostPossibleRating(Player* thePl, Results* theResults)
{
  ofstream aTada("tadaTmp.txt");
  int anOldRating = int(theResults->Rating(thePl));
  double aMinProp = -1.e+100;
  int aResult = -1;
  for(int a = 1; a < 1075; a++) {
    theResults->SetRating(thePl, a);
    double aProp = GetProbability(aTada, theResults);
    if (aProp > aMinProp) {
      aMinProp = aProp;
      aResult = a;
    }
  }
  theResults->SetRating(thePl, anOldRating);
  return aResult;
}
