#include "common.h"

#include "sample.h"
#include "readgames.h"
#include "results.h"
#include <set>
#include <sstream>

// to compute and show penalties
//#define FINAL_REPORT
// to show nice colors and final rating with penalties included
//#define CELEBRATION_REPORT

map<string, string> myNames;

Common::Common() 
{
  // decryption of nicks
  myNames["VTN"] = "Владимир Тюрин";
  myNames["MPV"] = "Михаил Поникаров";
  myNames["ADO"] = "Алексей Дергачев";
  myNames["VRO"] = "Владислав Ромашко";
  myNames["AVV"] = "Александр Воронов";
  myNames["SZY"] = "Сергей Заричный";
  myNames["OUV"] = "Олег Уваров";
  myNames["EPV"] = "Евгений Плеханов";
  myNames["KHR"] = "Алексей Хромов";
  myNames["MEV"] = "Максим Ермолаев";
  myNames["NDS"] = "Наталья Ермолаева";
  myNames["EAV"] = "Евгений Аксенов";
  myNames["AKL"] = "Александр Ковалев";
  myNames["SMN"] = "Станислав Новиков";
  myNames["OAN"] = "Олег Агашин";
  myNames["EPA"] = "Екатерина Смирнова";
  myNames["APL"] = "Антон Полетаев";
  myNames["MKR"] = "Маргарита Карпунина";
  myNames["MKA"] = "Мария Кручинина";
  myNames["APO"] = "Алексей Петров";
  myNames["AKT"] = "Александр Котов";
  myNames["AIV"] = "Александр Ишмурадов";
  myNames["AVA"] = "Анна Воронова";
  myNames["AKA"] = "Анастасия Куванова";
  myNames["VBN"] = "Виталий Белкин";
  myNames["TIA"] = "Татьяна Ишмурадова";
  myNames["SKV"] = "Сергей Хромов";
  myNames["KPV"] = "Константин Павлов";
  myNames["MSH"] = "Максим Шкирев";
  myNames["OTV"] = "Ольга Тимакова";
  myNames["ABO"] = "Алексей Быстров";

  myFirstGame = 0;
  myCurrentDate = 111111;
  myResThisTour = new Results;
  myResOld = new Results;
}

void Common::ReadAll() 
{
  myPlayers = new Players(0); // all registered players
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
  ReadGames("OldData.txt", myFirstGame, *myPlayers, tada, aSRandCoeff, myCurrentDate, aMisfCoeffs, false);
  ReadGames("ThisTour.txt", myFirstGame, *myPlayers, tada, aSRandCoeff, myCurrentDate, aMisfCoeffs, true);
  ReadGames("today.txt", myFirstGame, *myPlayers, tada, aSRandCoeff, myCurrentDate, aMisfCoeffs, true);

  tada<<"Random coefficient "<<aSRandCoeff<<endl;
  srand(aSRandCoeff);

  tada<<"Read magnetic people"<<endl;
  FILE* magn =fopen("magnetic.txt", "rt");
  while(magn && !feof(magn)) {
    static char aPl[4];
    Game::ReadNick(aPl, magn);
    if (magn && aPl[0]) {
      Player* aP = myPlayers->GetByNick(aPl);
      if (aP) {
        aP->SetMagnetic(true);
        tada<<"Magnetic "<<aP->Nick()<<endl;
      }
    }
  }
  if (magn)
    fclose(magn);

  int aNBToday = 0;
  // set all players not here first
  int aPNum;
  for(aPNum = 0; aPNum < myPlayers->Num(); aPNum++)
    myPlayers->Get(aPNum)->IsHere(0);
  // read players presented today
  ifstream pl("players.txt");
  while(pl) {
    char aNick[4] = {0, 0, 0, 0};
    for(int b = 0; b < 3; b++) pl>>aNick[b];
    if (!pl) break;
    tada<<"'"<<aNick<<"' is here today"<<endl;
    aNBToday++;
    Player* aCurrent = myPlayers->GetByNick(aNick);
    if (aCurrent == 0) {
      aCurrent = new Player(aNick);
      myPlayers->Add(aCurrent);
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
    Player* aCurrent = myPlayers->GetByNick(aNick);
    if (aCurrent == 0) { // omit: this player is useless: not existing today
      tada<<"Initial raiting for "<<aNick<<" is useless!"<<endl;
      continue;
    }
    myResThisTour->SetRating(aCurrent, aRat);
  }
  fclose(irat);
  // calculate both this tour and whole ratings
  CalculateRatings(myFirstGame, *myPlayers, tada, myResOld, false);
  //CalculateRatings(myFirstGame, *myPlayers, tada, true);
  // in 2016 this tour raiting is computed in discrete manner
  CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, myResThisTour, true);
}

int Common::GamesRead()
{
  int aResult = 0;
  for(Game* aCurrent = myFirstGame; aCurrent; aCurrent = aCurrent->Next())
    aResult++;
  return aResult;
}
int Common::PlayersRead()
{
  return myPlayers->Num();
}

std::string& Common::Name(const int theIndex)
{
  char* aNick = myPlayers->Get(theIndex)->Nick();
  if (myNames.find(aNick) == myNames.end()) {
    static string aNickString;
    aNickString = aNick;
    return aNickString;
  }
  return myNames[aNick];
}

const char* Common::Nick(const int theIndex)
{
  return myPlayers->Get(theIndex)->Nick();
}

double Common::Raiting(const int theIndex, const bool theThisTour, const bool theFinal)
{
  Player* aPl = myPlayers->Get(theIndex);
  if (theFinal)
    return myResFinal->Rating(aPl);
  return theThisTour ? myResThisTour->Rating(aPl) : myResOld->Rating(aPl);
}

int Common::GamesThisTour(const int theIndex)
{
  return myPlayers->Get(theIndex)->GamesNum();
}

double Common::Raiting(const char* theNick, const bool theThisTour)
{
  Player* aPl = myPlayers->GetByNick(theNick);
  return theThisTour ? myResThisTour->Rating(aPl) : myResOld->Rating(aPl);
}

int Common::PreparePlay(PlayerToday*& thePlayers)
{
  int aPSize = myPlayers->Num();
  map<Player*, int> anIndexes;
  thePlayers = new PlayerToday[aPSize + 10];
  // store the array data
  for(int aP = 0; aP < aPSize; aP++) {
    thePlayers[aP].myNick = myPlayers->Get(aP)->Nick();
    thePlayers[aP].myFullName = myNames[myPlayers->Get(aP)->Nick()];
    thePlayers[aP].myMisfortune = myPlayers->Get(aP)->Misfortune();
    thePlayers[aP].myIsToday = false; // default
    anIndexes[myPlayers->Get(aP)] = aP;
  }
  // read players presented today
  ifstream pl("players.txt");
  while(pl) {
    char aNick[4] = {0, 0, 0, 0};
    for(int b = 0; b < 3; b++) pl>>aNick[b];
    if (!pl) break;
    Player* aCurrent = myPlayers->GetByNick(aNick);
    if (aCurrent == 0) {
      thePlayers[aPSize].myNick = aCurrent->Nick();
      thePlayers[aPSize].myFullName = "";
      thePlayers[aPSize].myMisfortune = 0;
      thePlayers[aPSize].myIsToday = true; // the new is here
      aPSize++;
    }
    if (anIndexes.find(aCurrent) != anIndexes.end())
      thePlayers[anIndexes[aCurrent]].myIsToday = true;
  }
  thePlayers[aPSize].myNick = 0;
  return aPSize;
}

void Common::sort(PlayerToday* thePlayers, const int theNumPools4, const int theNumPools2)
{
  // get the really checked people
  for(int aPNum = 0; aPNum < myPlayers->Num(); aPNum++)
    myPlayers->Get(aPNum)->IsHere(0); // reset all
  for(int a = 0; thePlayers[a].myNick; a++) {
    if (thePlayers[a].myIsToday)
      myPlayers->GetByNick(thePlayers[a].myNick)->IsHere(1); // set only checked
  }
  // init and launch sortings
  ofstream tada("tadaSort.txt");
  mySample = new Sample(*myPlayers, myFirstGame, theNumPools4, theNumPools2, tada, myResThisTour, myResOld);
}

GameToday Common::sorted(int theIndex)
{
  GameToday aResult;
  for(int a = 0; a < (mySample->GetGame(theIndex)->IsSingle() ? 2 : 4); a++) {
    aResult.myNicks[a] = mySample->GetGame(theIndex)->GetPlayer(a)->Nick();
  }
  // compute the propability
  ofstream tada("tadaTmp.txt");
  Game* aGame;
  aResult.isSingle = mySample->GetGame(theIndex)->IsSingle();
  if (aResult.isSingle)
    aGame = new Game(mySample->GetGame(theIndex)->GetPlayer(0), mySample->GetGame(theIndex)->GetPlayer(1));
  else
    aGame = new Game(mySample->GetGame(theIndex)->GetPlayer(0), mySample->GetGame(theIndex)->GetPlayer(1), mySample->GetGame(theIndex)->GetPlayer(2), mySample->GetGame(theIndex)->GetPlayer(3));

  double aMaxProp = -1;
  int aScore1, aScore2;
  for(int aLoseScore = -19; aLoseScore < 20; aLoseScore++) {
    if (aLoseScore == 0) continue; // bad case
    aGame->SetScores(aLoseScore, aLoseScore, aLoseScore);
    double aProp = aGame->GetProbability(tada, myResThisTour);
    if (aProp > aMaxProp) {
      aMaxProp = aProp;
      aScore1 = aLoseScore < 0 ? 21 + aLoseScore : 21;
      aScore2 = aLoseScore > 0 ? 21 - aLoseScore : 21;
    }
  }
  delete aGame;
  ostringstream aStr;
  aStr<<aScore1<<" "<<aScore2<<" "<<aScore1<<" "<<aScore2;
  aResult.myPropScore = aStr.str();
  return aResult;
}

std::string Common::unlucky()
{
  // collect the set of nicks in the game
  int anIndex;
  set<char*> aNicksPlayed;
  for(anIndex = 0; anIndex < mySample->GetPlaces4(); anIndex++) {
    for(int a = 0; a < 4; a++) {
      aNicksPlayed.insert(mySample->GetGame(anIndex)->GetPlayer(a)->Nick());
    }
  }
  for(anIndex = mySample->GetPlaces4(); anIndex < mySample->GetPlaces4() + mySample->GetPlaces2(); anIndex++) {
    for(int a = 0; a < 2; a++) {
      aNicksPlayed.insert(mySample->GetGame(anIndex)->GetPlayer(a)->Nick());
    }
  }
  string aResult;
  for(int aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
    if (myPlayers->Get(aPNum)->IsHere() && aNicksPlayed.find(myPlayers->Get(aPNum)->Nick()) == aNicksPlayed.end()) {
      aResult += " ";
      aResult += myPlayers->Get(aPNum)->Nick();
    }
  }
  return aResult;
}

// return the name or nick
static string name(Player* thePlayer) {
  map<string, string>::iterator aFound = myNames.find(thePlayer->Nick());
  if (aFound == myNames.end())
    return string(thePlayer->Nick());
  return aFound->second;
}

static void OutputDelta(ostream& report, int aDelta) {
  if (aDelta == 0) ;//report<<"-";
  else {
    report<<"(";
    if (aDelta > 0) report<<"+";
    report<<aDelta;
    report<<")";
  }
}

static void FillPlayer(Game* theFirstGame, Player* thePlayer, Results* theResults, PlayerInfo& toFill) {
  toFill.myRating = int(theResults->Rating(thePlayer));
  toFill.myNick = thePlayer->Nick();
  toFill.myGames = thePlayer->GamesNum();
  toFill.myLoses = 0;
  toFill.myWins = 0;
  toFill.myUnluck = thePlayer->UnluckyThisTour();
  for(Game* aGame = theFirstGame; aGame; aGame = aGame->Next()) {
    if (aGame->ThisTour() && aGame->IsInGame(thePlayer)) {
      if ((aGame->FirstWin() && aGame->IsInFirstTeam(thePlayer)) || (!aGame->FirstWin() && !aGame->IsInFirstTeam(thePlayer))) toFill.myWins++;
      else toFill.myLoses++;
    }
  }
}

GameInfo Common::getGame(const int thePlayerIndex, const int theGameNum)
{
  Player* aPl = myPlayers->Get(thePlayerIndex);
  // seaching for the game by the number
  Game* aGame;
  int aGameIndex = 0;
  for(aGame = myFirstGame; aGame; aGame = aGame->Next()) {
    if (aGame->ThisTour() && aGame->IsInGame(aPl)) {
      if (aGameIndex == theGameNum)
        break;
      aGameIndex++;
    }
  }
  GameInfo aRes;
  FillPlayer(myFirstGame, aPl, myResThisTour, aRes.myThis);
  aRes.myThis.myRating = aGame->MostPossibleRating(aPl, myResThisTour);
  FillPlayer(myFirstGame, aGame->Contra1(aPl), myResThisTour, aRes.myContra1);
  if (!aGame->IsSingle()) {
    FillPlayer(myFirstGame, aGame->Together(aPl), myResThisTour, aRes.myAlly);
    FillPlayer(myFirstGame, aGame->Contra2(aPl), myResThisTour, aRes.myContra2);
  }

  ostringstream aScores;
  bool inFirstTeam = aGame->IsInFirstTeam(aPl);
  if (inFirstTeam) { // player is in the first team
    aScores<<aGame->Score(0)<<":"<<aGame->Score(1)<<" "<<aGame->Score(2)<<":"<<aGame->Score(3);
    if (aGame->IsThreeSets())
      aScores<<" "<<aGame->Score(4)<<":"<<aGame->Score(5);
  } else {
    aScores<<aGame->Score(1)<<":"<<aGame->Score(0)<<" "<<aGame->Score(3)<<":"<<aGame->Score(2);
    if (aGame->IsThreeSets())
      aScores<<" "<<aGame->Score(5)<<":"<<aGame->Score(4);
  }
  aRes.myScore = aScores.str();
  aRes.myWin = (aGame->FirstWin() && inFirstTeam) || (!aGame->FirstWin() && !inFirstTeam);
  aRes.myWinCoef = aGame->GetSummScores(inFirstTeam);
  aRes.myWinCoef = aRes.myWinCoef * 2 - 1;
  aRes.myDate = aGame->Date();
  aRes.isSingle = aGame->IsSingle();

  return aRes;
}

std::string Common::saveReport()
{
  ofstream report("report.htm");
  ofstream sql("sql.sql");
  ofstream tada("tadaTmp.txt");

  // init report file
  report<<"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">"<<endl;
  report<<"<body lang=RU><table border=3 cellpadding=4 cellspacing=0 bordercolor=\"#B0B0B0\">"<<endl;
  report<<"<tr align=center><b>"<<endl;
  report<<"<th>Игроки</th>"<<endl;
  report<<"<th>Рейтинг<br><font color=\"#C0C0C0\">(дельта)</font></th>"<<endl;
  report<<"<th>Результат</th>"<<endl;
  report<<"<th>Рейтинг<br><font color=\"#C0C0C0\">(дельта)</font></th>"<<endl;
  report<<"<th>Игроки</th></b></tr>"<<endl;

  // init sql file
  sql<<"start transaction;"<<endl;
  // calculate ratings by discrete formula (used in the tournament places definition)
  bool aIsFirstGameToday = true;
  Results* aResBeforeToday = new Results; // results of player before current date
  for(Game *aCurrentGame = myFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    bool isSingle = aCurrentGame->IsSingle();
    int aPl0 = 0;
    int aPl1 = isSingle ? 0 : 1;
    int aPl2 = isSingle ? 1 : 2;
    int aPl3 = isSingle ? 1 : 3;
    if (aCurrentGame->Date() == myCurrentDate) {
      // set all next games disabled (including this) to compute the intermediate rating (during this game sortings)
      aCurrentGame->setDisabled(true, true);
      if (aIsFirstGameToday) { // store the previous days values
        aIsFirstGameToday = false;
        CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, aResBeforeToday, true);
      }
      // current results
      // store initial ratings before this game
      Results aResBefore;
      CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, &aResBefore, true);
      // compute ratings after this game
      aCurrentGame->setDisabled(false, false);
      Results aRes;
      CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, &aRes, true);

      report<<"<tr align=center>";
      static const char* aWinBG = "\"#ffdddd\"";
      static const char* aLosBG = "\"#ddddff\"";
      std::string aSingleDoubleTd = aCurrentGame->IsSingle() ? "<td rowspan=2" : "<td";
      report<<aSingleDoubleTd<<" bgcolor="<<(aCurrentGame->FirstWin() ? aWinBG : aLosBG)<<"><b>"<<name(aCurrentGame->GetPlayer(aPl0))<<"</b></td>";
      double aRatingBefore = aResBefore.Rating(aCurrentGame->GetPlayer(aPl0));
      report<<aSingleDoubleTd<<">"<<(int)(aRatingBefore)<<" <font color=\"#C0C0C0\">";
      OutputDelta(report, int(aRes.Rating(aCurrentGame->GetPlayer(aPl0)) - aRatingBefore));
      report<<"</font>"<<"</td>";
      // two rows for score
      report<<"<td rowspan=2><b>"<<aCurrentGame->NumWin(true)<<" : "<<aCurrentGame->NumWin(false)<<"</b><br>";
      report<<aCurrentGame->Score(0)<<":"<<aCurrentGame->Score(1)<<" ";
      report<<aCurrentGame->Score(2)<<":"<<aCurrentGame->Score(3);
      if (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3)
        report<<" "<<aCurrentGame->Score(4)<<":"<<aCurrentGame->Score(5);
      report<<"</td>";

      aRatingBefore = aResBefore.Rating(aCurrentGame->GetPlayer(aPl2));
      report<<aSingleDoubleTd<<">"<<(int)(aRatingBefore)<<" <font color=\"#C0C0C0\">";
      OutputDelta(report, int(aRes.Rating(aCurrentGame->GetPlayer(aPl2)) - aRatingBefore));
      report<<"</font>"<<"</td>";
      report<<aSingleDoubleTd<<" bgcolor="<<(aCurrentGame->FirstWin() ? aLosBG : aWinBG)<<"><b>"<<name(aCurrentGame->GetPlayer(aPl2))<<"</b></td>";
      report<<"</tr>"<<endl;

      report<<"<tr align=center>";
      if (!aCurrentGame->IsSingle()) {
        report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aWinBG : aLosBG)<<"><b>"<<name(aCurrentGame->GetPlayer(aPl1))<<"</b></td>";
        aRatingBefore = aResBefore.Rating(aCurrentGame->GetPlayer(aPl1));
        report<<"<td>"<<(int)(aRatingBefore)<<" <font color=\"#C0C0C0\">";
        OutputDelta(report, int(aRes.Rating(aCurrentGame->GetPlayer(aPl1)) - aRatingBefore));
        report<<"</font>"<<"</td>";
        aRatingBefore = aResBefore.Rating(aCurrentGame->GetPlayer(aPl3));
        report<<"<td>"<<(int)(aRatingBefore)<<" <font color=\"#C0C0C0\">";
        OutputDelta(report, int(aRes.Rating(aCurrentGame->GetPlayer(aPl3)) - aRatingBefore));
        report<<"</font>"<<"</td>";
        report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aLosBG : aWinBG)<<"><b>"<<name(aCurrentGame->GetPlayer(aPl3))<<"</b></td>";
      }
      report<<"</tr>"<<endl;

      sql<<"insert into games (id, date, id1, id2, id3, id4, set1_12, set1_34, set2_12, set2_34, set3_12, set3_34, type, tournament)"<<endl;
      sql<<"  select max_id as id, \"20"<<(myCurrentDate%100)<<"-"<<((myCurrentDate/100)%100)<<"-"<<(myCurrentDate/10000)<<"\" as date, id1, id2, id3, id4,"<<endl;
      sql<<"    "<<aCurrentGame->Score(0)<<" as set1_12, "<<aCurrentGame->Score(1)<<" as set1_34, ";
      sql<<aCurrentGame->Score(2)<<" as set2_12, "<<aCurrentGame->Score(3)<<" as set2_34, ";
      if (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3)
        sql<<aCurrentGame->Score(4)<<" as set3_12, "<<aCurrentGame->Score(5)<<" as set3_34,"<<endl;
      else sql<<"0 as set3_12, 0 as set3_34,"<<endl;
      sql<<"    0 as type, 44 as tournament"<<endl;
      sql<<"  from"<<endl;
      sql<<"    (select max(id)+1 as max_id from games) maxid_table,"<<endl;
      sql<<"    (select id as id1 from players where nick='"<<aCurrentGame->GetPlayer(aPl0)->Nick()<<"') id1_table,"<<endl;
      if (aCurrentGame->IsSingle()) {
        sql<<"    (select 0 as id2 from players where nick='"<<aCurrentGame->GetPlayer(aPl1)->Nick()<<"') id2_table,"<<endl;
      } else {
        sql<<"    (select id as id2 from players where nick='"<<aCurrentGame->GetPlayer(aPl1)->Nick()<<"') id2_table,"<<endl;
      }
      sql<<"    (select id as id3 from players where nick='"<<aCurrentGame->GetPlayer(aPl2)->Nick()<<"') id3_table,"<<endl;
      if (aCurrentGame->IsSingle()) {
        sql<<"    (select 0 as id4 from players where nick='"<<aCurrentGame->GetPlayer(aPl3)->Nick()<<"') id4_table;"<<endl;
      } else {
        sql<<"    (select id as id4 from players where nick='"<<aCurrentGame->GetPlayer(aPl3)->Nick()<<"') id4_table;"<<endl;
      }

      aCurrentGame->setDisabled(false, true);

      if (aCurrentGame->Next()) { // separator between games: after undisabling all next
        if (aCurrentGame->IsLastInFrame())
          report<<"<tr bgcolor=\"#B0B0B0\"><td colspan=5></td></tr>"<<endl; 
        else 
          report<<"<tr><td colspan=5></td></tr>"<<endl; 
      }
    }
    // store new raitings and add number of played games to each player
    for(int aPlrNum = 0; aPlrNum < (isSingle ? 2 : 4); aPlrNum++) {
      if (aCurrentGame->ThisTour())
        aCurrentGame->GetPlayer(aPlrNum)->IncGamesNum();
    }

  }
  // finalize sql
  sql<<"commit;"<<endl;
  Results aRes;
  CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, &aRes, true);

  int aNumInTable = 0; // place in the table
  // maximum number of games, including unlucky
  int aMaxGames = 0;
  for(int aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
    Player* aP = myPlayers->Get(aPNum);
    if (aP->GamesNum() != 0)
      aNumInTable++;
    int aGames = aP->GamesNum() + aP->UnluckyThisTour();
    if (aGames > aMaxGames)
      aMaxGames = aGames;
  }

#ifdef FINAL_REPORT
  // compute old positions with penalties
  Game *aCurrentGame;
  map<Player*, int> anOldGamesNum;
  for(aCurrentGame = myFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    if (!aCurrentGame->ThisTour()) continue;
    anOldGamesNum[aCurrentGame->GetPlayer(0)]++;
    anOldGamesNum[aCurrentGame->GetPlayer(1)]++;
    if (!aCurrentGame->IsSingle()) {
      anOldGamesNum[aCurrentGame->GetPlayer(2)]++;
      anOldGamesNum[aCurrentGame->GetPlayer(3)]++;
    }
    if (aCurrentGame->Date() == myCurrentDate) {
      // set all next games disabled (including this) to compute the intermediate rating (during this game sortings)
      aCurrentGame->setDisabled(true, true);
      CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, true);
      // compute the max games at that time
      int aPNum, aMaxGames = 1;
      for(aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
        Player* aP = myPlayers->Get(aPNum);
        if (anOldGamesNum.find(aP) != anOldGamesNum.end() && anOldGamesNum[aP] > aMaxGames) {
          aMaxGames = anOldGamesNum[aP];
        }
      }
      // apply penalty
      for(aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
        Player* aP = myPlayers->Get(aPNum);
        if (anOldGamesNum.find(aP) == anOldGamesNum.end())
          continue;
        int aGamesNum = anOldGamesNum[aP] + aP->UnluckyThisTour();
        if (aGamesNum * 2 < aMaxGames) {
          double aPercent = aGamesNum * 2. / aMaxGames;
          aP->SetRating(aP->Rating(true) * aPercent, true);
        }
      }
      SortByRating(*myPlayers, true, false);
      int aNumInTable = 0;
      for(aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
        Player* aP = myPlayers->Get(aPNum);
        if (aP->GamesNum()) {
          aNumInTable++;
          aPlOldPosition[aP] = aNumInTable;
        }
      }
      aCurrentGame->setDisabled(false, true);
      break;
    }
  }
  CalculateRatingsDiscrete(myFirstGame, *myPlayers, tada, true);

  // apply penalties
  map<Player*, double> aPlPenalty;
  map<Player*, int> aPlPenaltyGames; // number of games to play to avoid penalties
  for(int aPNum = 0; aPNum < myPlayers->Num(); aPNum++) {
    Player* aP = myPlayers->Get(aPNum);
    int aGamesNum = aP->GamesNum() + aP->UnluckyThisTour();
    if (aGamesNum * 2 < aMaxGames) {
      double aPercent = aGamesNum * 2. / aMaxGames;
      aPlPenalty[aP] = (int)(aP->Rating(true) * (1. - aPercent));
      aPlPenaltyGames[aP] = int(aMaxGames / 2 - aGamesNum);
      aP->SetFinalRating(aP->Rating(true) * aPercent);
    } else aP->SetFinalRating(aP->Rating(true));
  }
  SortByRating(*myPlayers, true, true);
#else // just set final rating equal to the rating
#endif

  report<<"</table><br>"<<endl<<endl;
  // output players sorted by raitings
  report<<"<table border=3 cellpadding=4 cellspacing=0 bordercolor=\"#B0B0B0\">"<<endl;
  report<<"<tr align=center><th colspan=2>Игрок</th>"<<endl;
  report<<"<th colspan=1 nowrap>Рейтинг<br><font color=\"#C0C0C0\">(дельта)</font></th>";
#ifdef FINAL_REPORT
  report<<"<th nowrap>Штраф<br><font color=\"#C0C0C0\">(доиграть)</font></th>";
#endif
  //<th>Изменение рейтинга<br><font color=\"#C0C0C0\">(за весь турнир)</font></th>";
  report<<"<th colspan=1 nowrap>Победы<br><font color=\"#C0C0C0\">(из 3х партий)</font></th>";
  report<<"<th colspan=1 nowrap>Поражения<br><font color=\"#C0C0C0\">(из 3х партий)</font></th>";
  report<<"<th colspan=1 nowrap>Игры<br><font color=\"#C0C0C0\">(скамейка)</font></th></tr>"<<endl;
  // columnts in the table
  stringstream repPosition, repNames, repRating, repWin, repLose, repGame, repPenalt;
  const std::string anEmptyTable("<table border=0>");
  repPosition<<anEmptyTable; repNames<<anEmptyTable; repRating<<anEmptyTable; repWin<<anEmptyTable; repLose<<anEmptyTable; repGame<<anEmptyTable; repPenalt<<anEmptyTable;

  aNumInTable = 0; // place in the table
  std::list<Player*>::const_iterator aResPlayer = aRes.Sorted().begin();
  for(; aResPlayer != aRes.Sorted().end(); aResPlayer++) {
	Player* aP = *aResPlayer;
    if (aP->GamesNum() != 0) {
      aNumInTable++;
      repPosition<<"<tr align=right><td><b>"<<aNumInTable<<"</b></td>";

      repPosition<<"<td align=center nowrap> ";
      int aPosDelta = aResBeforeToday->Position(aP)  == Results::NO_POSITION ? 0 : aResBeforeToday->Position(aP) - aNumInTable;
      while(aPosDelta > 0) {
        repPosition<<"<img src=\"up.png\"/>";
        aPosDelta--;
      }
      while(aPosDelta < 0) {
        repPosition<<"<img src=\"down.png\"/>";
        aPosDelta++;
      }
      repPosition<<"</td></tr>";

#ifdef CELEBRATION_REPORT
      const char* aCelColor = "#ffffff";
      if (aNumInTable == 1) aCelColor = "#ffd700";
      else if (aNumInTable == 2) aCelColor = "#c0c0c0";
      else if (aNumInTable == 3) aCelColor = "#cd7f32";
      else if (aNumInTable == 7) aCelColor = "#ff69b4";
      repNames<<"<tr><td nowrap bgcolor=\""<<aCelColor<<"\">"<<name(aP)<<"</td></tr>";
#else
      repNames<<"<tr><td nowrap>"<<name(aP)<<"</td></tr>";
#endif

      double aPercent = 0.;
      int aGamesNum = aP->GamesNum() + aP->UnluckyThisTour();
      if (aGamesNum * 2 < aMaxGames) {
        aPercent = aGamesNum * 2. / aMaxGames;
      }

      // raiting and delta of the player
      repRating<<"<tr><td align=right>"
#ifdef CELEBRATION_REPORT
        <<(int)(aP->Rating(true) - (aPercent == 0 ? 0. : aPlPenalty[aP]))
#else
        <<(int)(aRes.Rating(aP))
#endif
        <<"</td><td align=left nowrap><font color=\"#C0C0C0\">";
      OutputDelta(repRating, (int)(aRes.Rating(aP) - aResBeforeToday->Rating(aP)));
      repRating<<"</font></td></tr>";
#ifdef FINAL_REPORT
      // penalty
      if (aPercent != 0) {
        repPenalt<<"<tr><td align=center nowrap>"<<aPlPenalty[aP]<<" <font color=\"#C0C0C0\">("<<aPlPenaltyGames[aP]<<")</font> </td></tr>";
      } else {
        repPenalt<<"<tr><td align=center> </font>-</td></tr>";
      }
#endif
      // loses, wins and games
      int aLoses = 0, aWins = 0;
      int aLoses3 = 0, aWins3 = 0;
      int aLosesToday = 0, aWinsToday = 0; // games today
      for(Game *aCurrentGame = myFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
        if (!aCurrentGame->ThisTour())
          continue;
        bool is3 = (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3);
        if (aCurrentGame->IsInFirstTeam(aP)) {
          if (aCurrentGame->FirstWin()) {
            aWins++; 
            if (is3) aWins3++;
          } else {
            aLoses++;
            if (is3) aLoses3++;
          }
          if (aCurrentGame->Date() == myCurrentDate) {
            if (aCurrentGame->FirstWin()) {
              aWinsToday++; 
            } else {
              aLosesToday++;
            }
          }
        }
        if (aCurrentGame->IsInSecondTeam(aP)) {
          if (!aCurrentGame->FirstWin()) {
            aWins++; 
            if (is3) aWins3++;
          } else {
            aLoses++;
            if (is3) aLoses3++;
          }
          if (aCurrentGame->Date() == myCurrentDate) {
            if (aCurrentGame->FirstWin()) {
              aLosesToday++; 
            } else {
              aWinsToday++;
            }
          }
        }
      }
      // wins
      repWin<<"<tr><td align=right>"<<endl;
      while(aWinsToday > 0) {
        repWin<<"<img src=\"plus.png\"/>";
        aWinsToday--;
      }
      repWin<<aWins<<"</td><td align=left><font color=\"#C0C0C0\">("<<aWins3<<")</font></td></tr>";
      // loses
      repLose<<"<tr><td align=right>"<<endl;
      while(aLosesToday > 0) {
        repLose<<"<img src=\"minus.png\"/>";
        aLosesToday--;
      }
      repLose<<aLoses<<"</td><td align=left><font color=\"#C0C0C0\">("<<aLoses3<<")</font></td></tr>";
      // total games
      repGame<<"<tr><td align=right>"<<aWins + aLoses<<" </td>";
      repGame<<"<td align=left><font color=\"#C0C0C0\">("<<aP->UnluckyThisTour()<<")</font></td>";
      repGame<<"</tr>"<<endl;
    }
  }
  const std::string aTableEnd("</table>\r\n");
  repPosition<<aTableEnd; repNames<<aTableEnd; repRating<<aTableEnd; repWin<<aTableEnd; repLose<<aTableEnd; repGame<<aTableEnd; repPenalt<<aTableEnd;
  report<<"<tr><td align=center>"<<repPosition.str()<<"</td><td align=center>"<<repNames.str()<<"</td><td align=center>"<<repRating.str();
#ifdef FINAL_REPORT
  report<<"<td align=center>"<<repPenalt.str()<<"</td>"<<endl;
#endif
  report<<"</td><td align=center>"<<repWin.str()<<"</td><td align=center>"<<repLose.str()<<"</td><td align=center>"<<repGame.str()<<"</td></tr>"<<endl;
  // done
  report<<"</table></body>"<<endl;
  report<<"</head></html>"<<endl;
  report.close();
  // re-read file to return the text
  ifstream is ("report.htm", ifstream::binary);
  if (is) {
    // get length of file:
    is.seekg (0, is.end);
    int length = (int)(is.tellg());
    is.seekg (0, is.beg);

    char * buffer = new char [length + 1];
    // read data as a block:
    is.read (buffer,length);
    is.close();
    buffer[length] = 0;
    return string(buffer);
  }
  return string("");
}

std::string Common::magneticReport()
{
  stringstream aResult;
  aResult<<"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">"<<endl;
  aResult<<"<body lang=RU><table border=3 cellpadding=4 cellspacing=0 bordercolor=\"#B0B0B0\">"<<endl;
  Results aRatings;
  ofstream tada("tadaM.txt");
  CalculateRatings(myFirstGame, *myPlayers, tada, &aRatings, true);
  Game *aCurrentGame;
  for(aCurrentGame = myFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    Player *aP1 = 0, *aP2, *aV1, *aV2;
    if (aCurrentGame->IsSingle() || !aCurrentGame->ThisTour())
      continue;
    bool isFirst = true;
    if (aCurrentGame->GetPlayer(0)->IsMagnetic() && aCurrentGame->GetPlayer(1)->IsMagnetic()) {
      aP1 = aCurrentGame->GetPlayer(0);
      aP2 = aCurrentGame->GetPlayer(1);
      aV1 = aCurrentGame->GetPlayer(2);
      aV2 = aCurrentGame->GetPlayer(3);
    } else if (aCurrentGame->GetPlayer(2)->IsMagnetic() && aCurrentGame->GetPlayer(3)->IsMagnetic()) {
      isFirst = false;
      aP1 = aCurrentGame->GetPlayer(2);
      aP2 = aCurrentGame->GetPlayer(3);
      aV1 = aCurrentGame->GetPlayer(0);
      aV2 = aCurrentGame->GetPlayer(1);
    }
    if (aP1) {
      double aVRating = aCurrentGame->GetTeamRaiting(!isFirst, &aRatings);
      aResult<<aP1->Nick()<<" "<<aP2->Nick()<<" vs "<<aV1->Nick()<<" "<<aV2->Nick()<<" ("<<(int)aVRating<<")  ";
      aResult<<aCurrentGame->Score(isFirst ? 0 : 1)<<":"<<aCurrentGame->Score(isFirst ? 1 : 0)<<" "<<aCurrentGame->Score(isFirst ? 2 : 3)<<":"<<aCurrentGame->Score(isFirst ? 3 : 2);
      if (aCurrentGame->IsThreeSets())
        aResult<<" "<<aCurrentGame->Score(isFirst ? 4 : 5)<<":"<<aCurrentGame->Score(isFirst ? 5 : 4);
      aResult<<"  => "<<aVRating * aCurrentGame->GetSummScores(isFirst) * 2;
      aResult<<"<br>"<<endl;
    }
  }
  aResult<<"</body>"<<endl;
  aResult<<"</head></html>"<<endl;
  return aResult.str();
}
