#include "player.h"
#include <iostream>

Player::Player(const char* theNick, const int theRating) {
  nick = new char[4];
  strcpy(nick, theNick);
  next = 0;
  isHere = true;
  misfortune = 0;
  numgames = 0;
  numUnlucky = 0;
  playedInFrame = false;
  justPassed = false;
  playedInSingle = 0;
  myIsMagnetic = false;
}

Player::~Player() {delete [] nick;}

char* Player::Nick() const {
  return nick;
}

void Player::Unlucky(const int theCoeff) {
  if (theCoeff) misfortune += theCoeff;
  else misfortune += 100;
  justPassed = true;
}
void Player::DayEnd() {
  misfortune -= 35; 
  if (misfortune < 0) misfortune = 0;
  justPassed = false;
}
void Player::FrameEnd() {
  if (playedInFrame) {
    playedInFrame=false; 
    misfortune -= 15; 
    if (misfortune < 0) misfortune = 0;
  }
  justPassed = false;
}

void Player::SetMagnetic(bool theMagnetic)
{
	myIsMagnetic = theMagnetic;
}

bool Player::IsMagnetic()
{
  return myIsMagnetic;
}
