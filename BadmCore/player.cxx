#include "player.h"
#include <iostream>

Player::Player(const char* theNick, const int theRating) {
  nick = new char[4];
  strcpy(nick, theNick);
  rating = theRating;
  finalRating = theRating;
  thisTourRating = theRating;
  next = 0;
  isHere = true;
  misfortune = 0;
  numgames = 0;
  numUnlucky = 0;
  playedInFrame = false;
  justPassed = false;
  playedInSingle = 0;
}

Player::~Player() {free(nick);}

void Player::SetRating(const double theRating, const bool theThisTour) {
  if (theThisTour)
    thisTourRating = theRating;
  else
    rating = theRating;
}

double Player::Rating(const bool theThisTour) const {
  return theThisTour ? thisTourRating : rating;
}

void Player::SetFinalRating(const double theRating) {
  finalRating = theRating;
}

double Player::FinalRating() const {
  return finalRating;
}

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
