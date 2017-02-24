#include "sample.h"
#include "readgames.h"

#include <iostream>

using namespace std;

int Compute() {
  Game* aFirstGame = 0;
  Players aPl(0); // all registered players
  ofstream tada("tada.txt");
  tada<<"Hello!"<<endl;

  tada<<"Read misfortune guests coefficients"<<endl;
  FILE* misf =fopen("misf_coeff.txt", "rt");
  map<char, map<char, map<char, int> > > aMisfCoeffs; // 3 symbols of nick -> coefficient of misfortune
  while(misf && !feof(misf)) {
    static char aGuest[4];
  	static char aCoeff[5];
    Game::ReadNick(aGuest, misf);
	  fscanf(misf, "%s", aCoeff);
  	int aC = atoi(aCoeff);
    if (misf && aC > 0 && aGuest[0]) {
      aMisfCoeffs[aGuest[0]][aGuest[1]][aGuest[2]] = aC;
      tada<<"Misfortune coeff for guest "<<aGuest<<"="<<aC<<endl;
    }
  }
  fclose(misf);

  //ReadGames("OCNOpen3.txt", aFirstGame, aFirstPlayer, tada);
  int aSRandCoeff = 1;
  int aCurrentDate = 111111;
  ReadGames("OldData.txt", aFirstGame, aPl, tada, aSRandCoeff, aCurrentDate, aMisfCoeffs);
  ReadGames("today.txt", aFirstGame, aPl, tada, aSRandCoeff, aCurrentDate, aMisfCoeffs);
  tada<<"Random coefficient "<<aSRandCoeff<<endl;
  srand(aSRandCoeff);
  
  int aNBToday = 0;
  // set all players not here first
  int aPNum;
  for(aPNum = 0; aPNum < aPl.Num(); aPNum++) aPl.Get(aPNum)->IsHere(0);
  // read players presented today
  ifstream pl("players.txt");
  while(pl) {
    char aNick[4] = {0, 0, 0, 0};
    for(int b = 0; b < 3; b++) pl>>aNick[b];
    if (!pl) break;
    tada<<"'"<<aNick<<"' is here today"<<endl;
    aNBToday++;
    Player* aCurrent = aPl.GetByNick(aNick);
    if (aCurrent == 0) {
      aCurrent = new Player(aNick);
      aPl.Add(aCurrent);
    }
    aCurrent->IsHere(1);
  }
  // open initial raitings file to use them
  FILE* irat = fopen("init_raitings.txt", "rt");
  if (irat == NULL) {
    tada<<"File 'init_raitings.txt' not found!"<<endl;
    exit(1);
  }
  while(irat != NULL && !feof(irat)) {
    char aNick[4];
    fscanf(irat, "%s", aNick);
    if (feof(irat)) break;
	  int aRat = 0;
	  fscanf(irat, "%d", &aRat);
	  tada<<"Read initial raiting of "<<aNick<<" aRat="<<aRat<<endl;
    Player* aCurrent = aPl.GetByNick(aNick);
    if (aCurrent == 0) { // omit: this player is useless: not existing today
		  tada<<"Initial raiting for "<<aNick<<" is useless!"<<endl;
      continue;
    }
    aCurrent->SetRating(aRat);
  }
  fclose(irat);

  CalculateRatingsDiscrete(aFirstGame, aPl, tada);
  tada<<"Calculate ratings done"<<endl;
  // output misfortunes
  for(aPNum = 0; aPNum < aPl.Num(); aPNum++) 
    tada<<"Misfortune of "<<aPl.Get(aPNum)->Nick()<<":"<<aPl.Get(aPNum)->Misfortune()<<endl;

  // append to file "today"
  ofstream tod("today.txt", ios::app);
  int aPools = aNBToday / 4;
  if (aNBToday % 4 == 1) aPools--;
  tada<<"pools in game "<<aPools<<endl;
  Sample aSample(aPl, aFirstGame, aPools, 2, tada);
  tada<<"aSample.GetPlaces()="<<aSample.GetPlaces()<<endl;
  for(int aPlace = 0; aPlace < aSample.GetPlaces(); aPlace++) {
    Game* aGame = aSample.GetGame(aPlace);
	  // make 'is here' to false for people who was not here
	  aGame->GetPlayer(0)->IsHere(false);
	  aGame->GetPlayer(1)->IsHere(false);
	  aGame->GetPlayer(2)->IsHere(false);
	  aGame->GetPlayer(3)->IsHere(false);
    tada<<aGame->GetPlayer(0)->Nick()<<" + "<<aGame->GetPlayer(1)->Nick()<<" ("<<aGame->GetPlayer(0)->Rating() <<"+"<< aGame->GetPlayer(1)->Rating()<<") vs ("
        <<aGame->GetPlayer(2)->Rating() <<"+"<< aGame->GetPlayer(3)->Rating()<<") "<<aGame->GetPlayer(2)->Nick()<<" + "<<aGame->GetPlayer(3)->Nick()
        <<" chance "<<aGame->GetWinDelta2()<<endl;
        // add this game to the end
    int aNumGames = 1;
    Game* aLast = aFirstGame;
    while(aLast && aLast->Next() != 0) {
      aLast = aLast->Next();
      aNumGames++;
    }
    if (aLast)
      aLast->SetNext(aGame);
    else aFirstGame = aLast;
    tod<<aGame->GetPlayer(0)->Nick()<<" "<<aGame->GetPlayer(1)->Nick()<<" "
       <<aGame->GetPlayer(2)->Nick()<<" "<<aGame->GetPlayer(3)->Nick()<<endl;
    tod<<aGame->GetWinDelta2()<<" "<<aGame->GetWinDelta3()<<endl;
  }
  tod<<"x ";
  for(int aP = 0; aP < aPl.Num(); aP++)
    if (aPl.Get(aP)->IsHere()) tod<<aPl.Get(aP)->Nick()<<" ";
  tod<<"x"<<endl;
  return 0;
}

#ifdef SortPC
int main() {
  Compute();
}
#endif
