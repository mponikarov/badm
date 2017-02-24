#include "players.h"
#include "player.h"

#include <iostream>

Players::Players(const int theNum) {
  myNum = 0;
  myMinMisfortune = -1;
  myPlMinMisf = 0;
  if (theNum == 0) {
    myP = new Player*[MAX_PLAYERS];
    myUsed = new bool[MAX_PLAYERS];
  } else {
    myP = new Player*[theNum];
    myUsed = new bool[theNum];
  }
}

Players::~Players() {
  delete [] myP;
  delete [] myUsed;
}

void Players::Add(Player* thePlayer) {
  myP[myNum] = thePlayer;
  myUsed[myNum] = false;
  myNum++;
  if (thePlayer->Misfortune() == myMinMisfortune)
    myPlMinMisf++;
}

void Players::Set(const int thePosition, Player* thePlayer) {
  if (myP[thePosition] && myP[thePosition]->Misfortune() == myMinMisfortune)
    myPlMinMisf--;
  myP[thePosition] = thePlayer;
  if (thePlayer && thePlayer->Misfortune() == myMinMisfortune)
    myPlMinMisf++;
}

void Players::Reset(const int thePos) {
  myP[thePos] = 0;
  myUsed[thePos] = false;
  if (thePos + 1 > myNum) myNum = thePos + 1;
}

Player* Players::GetByNick(const char* theNick) {
  for(int a = 0; a < myNum; a++) {
    if (strcmp(myP[a]->Nick(), theNick) == 0)
      return myP[a];
  }
  return 0;
}

Player* Players::GetUnusedPlayer(const int theNum) {
  int aNumHere = -1;
  for(int aNum = 0; aNum < myNum; aNum++) {
    if (myP[aNum]->IsHere() && !myUsed[aNum]) aNumHere++;
    if (aNumHere == theNum) {
      myUsed[aNum] = true;
      return myP[aNum];
    }
  }
  return 0;
}

void Players::TakeCopy(Players& theSource) {
  myNum = theSource.myNum;
  for(int a = 0; a < myNum; a++) {
    myP[a] = theSource.myP[a];
    myUsed[a] = false; // do not take used flags
  }
}

void Players::ClearUsed() {
  for(int a = 0; a < myNum; a++) {
    myUsed[a] = false;
  }
}

void Players::ExchangePair() {
  int aP1 = rand() % myNum;
  int aP2 = rand() % myNum;
  if (aP1 != aP2) {
    Player* aTmp = myP[aP1];
    myP[aP1] = myP[aP2];
    myP[aP2] = aTmp;
  }
}

const int Players::GetID(const Player* thePlayer) const
{
  for(int a = 0; a < myNum; a++) {
    if (thePlayer == myP[a]) return a;
  }
  return myNum; // not found
}

void Players::Delete(const int theNum)
{
  // shift
  for(int a = theNum + 1; a < myNum; a++) {
    myP[a - 1] = myP[a];
    myUsed[a - 1] = myUsed[a];
  }
  myNum--;
}

void Players::DayEnd()
{
  for(int a = 0; a < myNum; a++) {
    myP[a]->DayEnd();
  }
}
void Players::FrameEnd()
{
  for(int a = 0; a < myNum; a++) {
    myP[a]->FrameEnd();
  }
}
