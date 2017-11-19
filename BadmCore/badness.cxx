#include "badness.h"

Badness::Badness()
  : myNbPlayers(100)
{}

void Badness::inTeam(const int thePl1, const int thePl2)
{
  int anID = GenerateID(BAD_INTEAM, thePl1, thePl2);
  registerOccur(anID);
}

void Badness::contraDouble(const int thePl1, const int thePl2)
{
  int anID = GenerateID(BAD_CONTRADOUBLE, thePl1, thePl2);
  registerOccur(anID);
}

void Badness::contraSingle(const int thePl1, const int thePl2)
{
  int anID = GenerateID(BAD_CONTRASINGLE, thePl1, thePl2);
  registerOccur(anID);
  anID = GenerateID(BAD_SINGLES, thePl1);
  registerOccur(anID);
  anID = GenerateID(BAD_SINGLES, thePl2);
  registerOccur(anID);
  anID = GenerateID(BAD_CONTRADOUBLE, thePl1, thePl2); // also as contra double
  registerOccur(anID);
}

void Badness::framePassed()
{
  map<int, pair<int, double> >::iterator anOcc = myOccur.begin();
  for(; anOcc != myOccur.end(); anOcc++) {
    anOcc->second.second += 0.2;
  }
}

void Badness::dayPassed()
{
  map<int, pair<int, double> >::iterator anOcc = myOccur.begin();
  for(; anOcc != myOccur.end(); anOcc++) {
    anOcc->second.second = floor(anOcc->second.second) + 1; // the passed frames are removed
  }
}

int Badness::GenerateID(BadnessKind theKind, const int thePl1, const int thePl2)
{
  int aPID = 0;
  if (thePl1 != 0 && thePl2 != 0)
    aPID = (thePl1 < thePl2)? (thePl1 * myNbPlayers + thePl2) : (thePl2 * myNbPlayers + thePl1);
  aPID = aPID * 10 + theKind;
  return aPID;
}

int Badness::GenerateID(BadnessKind theKind, const int thePl1)
{
  return (thePl1 * myNbPlayers * myNbPlayers) * 10 + theKind;
}

void Badness::registerOccur(const int theID) {
  map<int, pair<int, double> >::iterator aFound = myOccur.find(theID);
  if (aFound == myOccur.end()) {
    myOccur[theID] = pair<int, double>(0, 0.);
    aFound = myOccur.find(theID);
  }
  aFound->second.first++;
  aFound->second.second = 0;
}

double Badness::badness(BadnessKind theKind, const int thePl1, const int thePl2)
{
  int anID = thePl2 == -1 ? GenerateID(theKind, thePl1) : GenerateID(theKind, thePl1, thePl2);
  map<int, double>::iterator aFind = myBad.find(anID);
  if (aFind != myBad.end()) {
    return aFind->second;
  }
  // compute badness
  double aBadness;
  switch(theKind) {
  case BAD_INTEAM:
    aBadness = 0.01;
    break;
  case BAD_CONTRADOUBLE:
    aBadness = 0.002; // don't need to be too big: it is minor
    break;
  case BAD_CONTRASINGLE:
    aBadness = 0.2;
    break;
  case BAD_SINGLES:
    aBadness = 0.1;
    break;
  }
  map<int, pair<int, double> >::iterator anOccur = myOccur.find(anID);
  if (anOccur == myOccur.end()) { // not occured => no badness
    aBadness = 0;
  } else {
    aBadness *= pow(1.3, (double)(anOccur->second.first - 1)); // 1.3 in power of occurences (< 1.4  > 1.1)
    aBadness /= 5. * anOccur->second.second; // time influence
  }
  return aBadness;
}
