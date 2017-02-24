#include "sample.h"
#include "readgames.h"
#include <math.h>

#include <iostream>
#include <string>

using namespace std;

static void OutputDelta(ofstream& report, int aDelta) {
  if (aDelta == 0) report<<"-";
  else {
    if (aDelta > 0) report<<"+";
    report<<aDelta;
  }
}

int Summary() {
  ofstream report("report.htm");
  ofstream sql("sql.sql");

  // decryption of nicks
  map<string, string> aDec;
  aDec["VTN"] = "Владимир Тюрин";
  aDec["MPV"] = "Михаил Поникаров";
  aDec["ADO"] = "Алексей Дергачев";
  aDec["VRO"] = "Владислав Ромашко";
  aDec["AVV"] = "Александр Воронов";
  aDec["SZY"] = "Сергей Заричный";
  aDec["OUV"] = "Олег Уваров";
  aDec["EPV"] = "Евгений Плеханов";
  aDec["KHR"] = "Алексей Хромов";
  aDec["MEV"] = "Максим Ермолаев";
  aDec["NDS"] = "Наталья Ермолаева";
  aDec["EAV"] = "Евгений Аксенов";
  aDec["AKL"] = "Александр Ковалев";
  aDec["SMN"] = "Станислав Новиков";
  aDec["OAN"] = "Олег Агашин";
  aDec["EPA"] = "Екатерина Смирнова";
  aDec["APL"] = "Антон Полетаев";

  Game* aFirstGame = 0;
  Players aPl(0); // all registered players
  ofstream tada("tada.txt");
  tada<<"Hello!"<<endl;

  map<char, map<char, map<char, int> > > aMisfCoeffs; // 3 symbols of nick -> coefficient of misfortune
  int aSRandCoeff = 1;
  int aCurrentDate = 0;
  ReadGames("OldData.txt", aFirstGame, aPl, tada, aSRandCoeff, aCurrentDate, aMisfCoeffs);
  ReadGames("today.txt", aFirstGame, aPl, tada, aSRandCoeff, aCurrentDate, aMisfCoeffs);
  cout<<"A current date "<<aCurrentDate<<endl;
  
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
  tada<<"Final today="<<aCurrentDate<<endl;

  // init report file
  report<<"<body lang=RU><table border=1 cellpadding=3>"<<endl;
  report<<"<tr align=center><b><th>Игроки</th><th>Рейтинг<br>и дельта</th><th>Результат</th><th>Рейтинг<br>и дельта</th><th>Игроки</th></b></tr>"<<endl;
  // init sql file
  sql<<"start transaction;"<<endl;
  // calculate ratings by discrete formula (used in the tournament places definition)
  map<Player*, double> aPlDeltas; // accumulated deltas
  map<Player*, double> allDeltas; // deltas for whole tournament
  map<Player*, double> aPlPositions;  // position of the player before current date
  for(Game *aCurrentGame = aFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
    double aTeam1Score = aCurrentGame->GetSummScores(true);
    double aTeam2Score = aCurrentGame->GetSummScores(false);
    double aTeam1Raiting = aCurrentGame->GetPlayer(0)->Rating() + aCurrentGame->GetPlayer(1)->Rating();
    double aTeam2Raiting = aCurrentGame->GetPlayer(2)->Rating() + aCurrentGame->GetPlayer(3)->Rating();
    double aTeam1Relative = aTeam1Score * (aTeam1Raiting + aTeam2Raiting);
    double aTeam2Relative = aTeam2Score * (aTeam1Raiting + aTeam2Raiting);
    double aB1, aB2;
    if (aCurrentGame->GetPlayer(0)->GamesNum() == 0 || aCurrentGame->GetPlayer(1)->GamesNum() == 0 ||
      aCurrentGame->GetPlayer(2)->GamesNum() == 0 || aCurrentGame->GetPlayer(3)->GamesNum() == 0) {
        aB1 = aB2 = 4;
    } else {
      aB1 = sqrt(aCurrentGame->GetPlayer(0)->GamesNum() + aCurrentGame->GetPlayer(1)->GamesNum() + 2.);
      aB2 = sqrt(aCurrentGame->GetPlayer(2)->GamesNum() + aCurrentGame->GetPlayer(3)->GamesNum() + 2.);
    }
    double aDeltas[4];
    aDeltas[0] = (aTeam1Relative * aCurrentGame->GetPlayer(0)->Rating() / aTeam1Raiting - aCurrentGame->GetPlayer(0)->Rating()) / aB1;
    aDeltas[1] = (aTeam1Relative * aCurrentGame->GetPlayer(1)->Rating() / aTeam1Raiting - aCurrentGame->GetPlayer(1)->Rating()) / aB1;
    aDeltas[2] = (aTeam2Relative * aCurrentGame->GetPlayer(2)->Rating() / aTeam2Raiting - aCurrentGame->GetPlayer(2)->Rating()) / aB2;
    aDeltas[3] = (aTeam2Relative * aCurrentGame->GetPlayer(3)->Rating() / aTeam2Raiting - aCurrentGame->GetPlayer(3)->Rating()) / aB2;
    allDeltas[aCurrentGame->GetPlayer(0)] += aDeltas[0];
    allDeltas[aCurrentGame->GetPlayer(1)] += aDeltas[1];
    allDeltas[aCurrentGame->GetPlayer(2)] += aDeltas[2];
    allDeltas[aCurrentGame->GetPlayer(3)] += aDeltas[3];
    tada<<"Next game, date="<<aCurrentGame->Date()<<" current="<<aCurrentDate<<endl;
    if (aCurrentGame->Date() == aCurrentDate) {
      tada<<"Pass!"<<endl;
      if (aPlPositions.empty()) { // store positions of the players before current day games
        for(int anIter = 0; anIter < aPl.Num(); anIter++) {
          int aPos = 1;
          for(int anIter2 = 0; anIter2 < aPl.Num(); anIter2++) {
            if (aPl.Get(anIter)->Rating() < aPl.Get(anIter2)->Rating()) aPos++;
          }
          aPlPositions[aPl.Get(anIter)] = aPos;
        }
      }
      aPlDeltas[aCurrentGame->GetPlayer(0)] += aDeltas[0];
      aPlDeltas[aCurrentGame->GetPlayer(1)] += aDeltas[1];
      aPlDeltas[aCurrentGame->GetPlayer(2)] += aDeltas[2];
      aPlDeltas[aCurrentGame->GetPlayer(3)] += aDeltas[3];
      report<<"<tr align=center>";
      static const char* aWinBG = "\"#ffdddd\"";
      static const char* aLosBG = "\"#ddddff\"";
      report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aWinBG : aLosBG)<<"><b>"<<aCurrentGame->GetPlayer(0)->Nick()<<"</b></td>";
      report<<"<td>"<<(int)(aCurrentGame->GetPlayer(0)->Rating())<<(aDeltas[0] >= 0 ? " + " : " - ")<<abs((int)(aDeltas[0]))<<"</td>";
      // two rows for score
      report<<"<td rowspan=2><b>"<<aCurrentGame->NumWin(true)<<" : "<<aCurrentGame->NumWin(false)<<"</b><br>";
      report<<aCurrentGame->Score(0)<<":"<<aCurrentGame->Score(1)<<" ";
      report<<aCurrentGame->Score(2)<<":"<<aCurrentGame->Score(3);
      if (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3)
        report<<" "<<aCurrentGame->Score(4)<<":"<<aCurrentGame->Score(5);
      report<<"</td>";

      report<<"<td>"<<(int)(aCurrentGame->GetPlayer(2)->Rating())<<(aDeltas[2] >= 0 ? " + " : " - ")<<abs((int)(aDeltas[2]))<<"</td>";
      report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aLosBG : aWinBG)<<"><b>"<<aCurrentGame->GetPlayer(2)->Nick()<<"</b></td>";
      report<<"</tr>"<<endl;

      report<<"<tr align=center>";
      report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aWinBG : aLosBG)<<"><b>"<<aCurrentGame->GetPlayer(1)->Nick()<<"</b></td>";
      report<<"<td>"<<(int)(aCurrentGame->GetPlayer(1)->Rating())<<(aDeltas[1] >= 0 ? " + " : " - ")<<abs((int)(aDeltas[1]))<<"</td>";
      report<<"<td>"<<(int)(aCurrentGame->GetPlayer(3)->Rating())<<(aDeltas[3] >= 0 ? " + " : " - ")<<abs((int)(aDeltas[3]))<<"</td>";
      report<<"<td bgcolor="<<(aCurrentGame->FirstWin() ? aLosBG : aWinBG)<<"><b>"<<aCurrentGame->GetPlayer(3)->Nick()<<"</b></td>";
      report<<"</tr>"<<endl;

      if (aCurrentGame->Next()) { // separator between games
        if (aCurrentGame->IsLastInFrame())
          report<<"<tr height=10></tr>"<<endl; 
        else 
          report<<"<tr height=1></tr>"<<endl; 
      }

      sql<<"insert into games (id, date, id1, id2, id3, id4, set1_12, set1_34, set2_12, set2_34, set3_12, set3_34, type, tournament)"<<endl;
      sql<<"  select max_id as id, \"20"<<(aCurrentDate%100)<<"-"<<((aCurrentDate/100)%100)<<"-"<<(aCurrentDate/10000)<<"\" as date, id1, id2, id3, id4,"<<endl;
      sql<<"    "<<aCurrentGame->Score(0)<<" as set1_12, "<<aCurrentGame->Score(1)<<" as set1_34, ";
      sql<<aCurrentGame->Score(2)<<" as set2_12, "<<aCurrentGame->Score(3)<<" as set2_34, ";
      if (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3)
        sql<<aCurrentGame->Score(4)<<" as set3_12, "<<aCurrentGame->Score(5)<<" as set3_34,"<<endl;
      else sql<<"0 as set3_12, 0 as set3_34,"<<endl;
      sql<<"    0 as type, 28 as tournament"<<endl;
      sql<<"  from"<<endl;
      sql<<"    (select max(id)+1 as max_id from games) maxid_table,"<<endl;
      sql<<"    (select id as id1 from players where nick='"<<aCurrentGame->GetPlayer(0)->Nick()<<"') id1_table,"<<endl;
      sql<<"    (select id as id2 from players where nick='"<<aCurrentGame->GetPlayer(1)->Nick()<<"') id2_table,"<<endl;
      sql<<"    (select id as id3 from players where nick='"<<aCurrentGame->GetPlayer(2)->Nick()<<"') id3_table,"<<endl;
      sql<<"    (select id as id4 from players where nick='"<<aCurrentGame->GetPlayer(3)->Nick()<<"') id4_table;"<<endl;
    }

    // store new raitings and add number of played games to each player
    for(int aPlrNum = 0; aPlrNum < 4; aPlrNum++) {
      //tada<<"Player "<<aCurrentGame->GetPlayer(aPlrNum)->Nick()<<" "<<aDeltas[aPlrNum]<<endl;
      aCurrentGame->GetPlayer(aPlrNum)->SetRating(aCurrentGame->GetPlayer(aPlrNum)->Rating() + aDeltas[aPlrNum]);
      aCurrentGame->GetPlayer(aPlrNum)->IncGamesNum();
    }
  }
  // finalize sql
  sql<<"commit;"<<endl;
  // sort players by raiting
  for(int anIter =  aPl.Num() - 1; anIter > 0; anIter--) {
    for(int aPlN = 0; aPlN < anIter; aPlN++) {
      if (aPl.Get(aPlN)->Rating() < aPl.Get(aPlN + 1)->Rating()) {
        Player* aTmp = aPl.Get(aPlN);
        aPl.Set(aPlN, aPl.Get(aPlN + 1));
        aPl.Set(aPlN + 1, aTmp);
      }
    }
  }
  report<<"</table><br>"<<endl;
  // output players sorted by raitings
  report<<"<table border=1 cellpadding=3>"<<endl;
  report<<"<tr align=center><th colspan=3>Игрок</th><th>Рейтинг</th><th>Изменение рейтинга<br><font color=\"#C0C0C0\">(за весь турнир)</font></th>";
  report<<"<th>Побед<br><font color=\"#C0C0C0\">(из 3х партий)</font></th>";
  report<<"<th>Поражений<br><font color=\"#C0C0C0\">(из 3х партий)</font></th><th>Игр</th></tr>"<<endl;
  for(int aPNum = 0; aPNum < aPl.Num(); aPNum++) {
    Player* aP = aPl.Get(aPNum);
    report<<"<tr><td><b>"<<aPNum + 1<<"</b></td>";

    report<<"<td align=center>";
    int aPosDelta = aPlPositions[aP] - (aPNum + 1);
    if (aPosDelta == 0) report<<"-";
    while(aPosDelta > 0) {
      report<<"<img src=\"up.png\"/>";
      aPosDelta--;
    }
    while(aPosDelta < 0) {
      report<<"<img src=\"down.png\"/>";
      aPosDelta++;
    }
    report<<"</td>";
#ifdef SortPC
    report<<"<td><b>"<<aDec[string(aP->Nick())]<<"</b> ("<<aP->Nick()<<")"<<"</td>";
#else
    report<<"<td>"<<aP->Nick()<<"</td>";
#endif

    report<<"<td align=center>"<<(int)(aP->Rating())<<"</td>";
    // delta raiting of the player
    report<<"<td align=center>";
    OutputDelta(report, (int)(aPlDeltas[aP]));
    report<<"<font color=\"#C0C0C0\"> (";
    OutputDelta(report, (int)(allDeltas[aP]));
    report<<")</font></td>";
    // loses, wins and games
    int aLoses = 0, aWins = 0;
    int aLoses3 = 0, aWins3 = 0;
    int aLosesToday = 0, aWinsToday = 0; // games today
    for(Game *aCurrentGame = aFirstGame; aCurrentGame; aCurrentGame = aCurrentGame->Next()) {
      bool is3 = (aCurrentGame->NumWin(true) + aCurrentGame->NumWin(false) == 3);
      if (aP == aCurrentGame->GetPlayer(0) || aP == aCurrentGame->GetPlayer(1)) {
        if (aCurrentGame->FirstWin()) {
          aWins++; 
          if (is3) aWins3++;
        } else {
          aLoses++;
          if (is3) aLoses3++;
        }
        if (aCurrentGame->Date() == aCurrentDate) {
          if (aCurrentGame->FirstWin()) {
            aWinsToday++; 
          } else {
            aLosesToday++;
          }
        }
      }
      if (aP == aCurrentGame->GetPlayer(2) || aP == aCurrentGame->GetPlayer(3)) {
        if (!aCurrentGame->FirstWin()) {
          aWins++; 
          if (is3) aWins3++;
        } else {
          aLoses++;
          if (is3) aLoses3++;
        }
        if (aCurrentGame->Date() == aCurrentDate) {
          if (aCurrentGame->FirstWin()) {
            aLosesToday++; 
          } else {
            aWinsToday++;
          }
        }
      }
    }
    report<<"<td align=center>"<<endl;
    while(aWinsToday > 0) {
      report<<"<img src=\"plus.png\"/>";
      aWinsToday--;
    }
    report<<aWins<<" <font color=\"#C0C0C0\">("<<aWins3<<")</font></td>";
    report<<"<td align=center>"<<endl;
    while(aLosesToday > 0) {
      report<<"<img src=\"minus.png\"/>";
      aLosesToday--;
    }
    report<<aLoses<<" <font color=\"#C0C0C0\">("<<aLoses3<<")</font></td>";
    report<<"<td align=center>"<<aWins + aLoses<<"</td>";

    report<<"</tr>"<<endl;
  }
  // done
  report<<"</table></body>"<<endl;
 
  return 0;
}

#ifdef SortPC
int main() {
  Summary();
}
#endif
