//
// C++ Implementation: localbero
//
// Description: 
//
//
// Author: FThauer FHammer <webmaster@pokerth.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "localbero.h"

using namespace std;

LocalBeRo::LocalBeRo(HandInterface* hi, int id, int dP, int sB, GameState gS)
: BeRoInterface(), myHand(hi), myBeRoID(gS), myID(id), dealerPosition(dP), smallBlindPosition(0), smallBlind(sB), highestSet(false), minimumRaise(2*sB), firstRun(true), firstRound(true), firstHeadsUpRound(true), currentPlayersTurn(0), firstRoundLastPlayersTurn(0), logBoardCardsDone(false)
{
	currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();
	lastPlayersTurnIt = myHand->getRunningPlayerList()->begin();

// 	int i;

	//SmallBlind-Position ermitteln 
// 	for(i=0; i<MAX_NUMBER_OF_PLAYERS; i++) {
// 		if (myHand->getPlayerArray()[i]->getMyButton() == 2) smallBlindPosition = i;
// 	}

}		


LocalBeRo::~LocalBeRo()
{
}

void LocalBeRo::nextPlayer() {

// 	myHand->getPlayerArray()[playersTurn]->action();

	
// 	cout << "playerID in nextPlayer(): " << (*currentPlayersTurnIt)->getMyID() << endl;

	(*currentPlayersTurnIt)->action();

}

void LocalBeRo::run() {

	int i;
	PlayerListIterator it;

	if (firstRun) {

		// determine smallBlindPosition
		PlayerListIterator smallBlindPositionIt;
	
		for(smallBlindPositionIt=myHand->getActivePlayerList()->begin(); smallBlindPositionIt!=myHand->getActivePlayerList()->end(); smallBlindPositionIt++) {
			if((*smallBlindPositionIt)->getMyButton() == BUTTON_SMALL_BLIND) break;
		}
	
		// determine running player before smallBlind (for heads up: determine dealer/smallblind)
		PlayerListIterator it_1, it_2;
		size_t i;
	
		// running player before smallBlind
		if(myHand->getActivePlayerList()->size() > 2) {
			it_1 = smallBlindPositionIt;
			for(i=0; i<myHand->getActivePlayerList()->size(); i++) {	
				if(it_1 == myHand->getActivePlayerList()->begin()) it_1 = myHand->getActivePlayerList()->end();
				it_1--;
				it_2 = find(myHand->getRunningPlayerList()->begin(), myHand->getRunningPlayerList()->end(), *it_1);
				// running player found
				if(it_2 != myHand->getRunningPlayerList()->end()) {
					lastPlayersTurnIt = it_2;
					break;
				}
			}
		}
		// heads up: bigBlind begins -> dealer/smallBlind is running player before bigBlind
		else {
			it_1 = find(myHand->getRunningPlayerList()->begin(), myHand->getRunningPlayerList()->end(), *smallBlindPositionIt);
			if( it_1 == myHand->getRunningPlayerList()->end() ) {
				cout << "ERROR - lastPlayersTurnIt-detection in localBeRo" << endl;
			}
			// smallBlind found
			else {
				lastPlayersTurnIt = it_1;
			}
		}
	
		currentPlayersTurnIt = lastPlayersTurnIt;







		myHand->getGuiInterface()->dealBeRoCards(myBeRoID);
		firstRun = false;

	}

	else {
		//log the turned cards
		if(!logBoardCardsDone) {

			int tempBoardCardsArray[5];

			myHand->getBoard()->getMyCards(tempBoardCardsArray);

			switch(myBeRoID) {
				case GAME_STATE_FLOP: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2]);
				break;
				case GAME_STATE_TURN: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3]);
				break;
				case GAME_STATE_RIVER: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3], tempBoardCardsArray[4]);
				break;
				default: { cout << "ERROR in localbero.cpp - wrong myBeRoID" << endl;}
			}
			logBoardCardsDone = true;

		}

		bool allHighestSet = true;

		// prfe, ob alle Sets gleich sind ( falls nicht, dann allHighestSet = 0 )
// 		for(i=0; i<MAX_NUMBER_OF_PLAYERS; i++) {
// 			if(myHand->getPlayerArray()[i]->getMyActiveStatus() && myHand->getPlayerArray()[i]->getMyAction() != 1 && myHand->getPlayerArray()[i]->getMyAction() != 6)	{
// 				if(highestSet != myHand->getPlayerArray()[i]->getMySet()) { allHighestSet=0; }
// 			}
// 		}

		// test if all running players have same sets (else allHighestSet = false)
		for(it=myHand->getRunningPlayerList()->begin(); it!=myHand->getRunningPlayerList()->end(); it++) {
			if(highestSet != (*it)->getMySet()) {
				allHighestSet = false;
			}
		}


		// prfen, ob aktuelle bero wirklich dran ist
		if(!firstRound && allHighestSet) { 
	
			// aktuelle bero nicht dran, weil alle Sets gleich sind
			//also gehe in naechste bero
			myHand->setActualRound(myBeRoID+1);

			//Action loeschen und ActionButtons refresh
// 			for(i=0; i<MAX_NUMBER_OF_PLAYERS; i++) {
// 				if(myHand->getPlayerArray()[i]->getMyAction() != 1 && myHand->getPlayerArray()[i]->getMyAction() != 6) myHand->getPlayerArray()[i]->setMyAction(0);
// 			}

			//Action loeschen und ActionButtons refresh
			for(it=myHand->getRunningPlayerList()->begin(); it!=myHand->getRunningPlayerList()->end(); it++) {
				(*it)->setMyAction(PLAYER_ACTION_NONE);
			}

			//Sets in den Pot verschieben und Sets = 0 und Pot-refresh
			myHand->getBoard()->collectSets();
			myHand->getBoard()->collectPot();
			myHand->getGuiInterface()->refreshPot();
			
			myHand->getGuiInterface()->refreshSet();
			myHand->getGuiInterface()->refreshCash();
			for(i=0; i<MAX_NUMBER_OF_PLAYERS; i++) { myHand->getGuiInterface()->refreshAction(i,PLAYER_ACTION_NONE); }
			
			myHand->switchRounds();
		}
		else {
			// aktuelle bero ist wirklich dran

			// Anzahl der effektiv gespielten Runden (des human player) erhöhen
			if(myHand->getPlayerArray()[0]->getMyActiveStatus() && myHand->getPlayerArray()[0]->getMyAction() != 1) {
				myHand->setBettingRoundsPlayed(myBeRoID);
			}
			
			//// !!!!!!!!!!!!!!!!1!!!! very buggy, rule breaking -> TODO !!!!!!!!!!!!11!!!!!!!! //////////////

			//// headsup:
			//// preflop: dealer is small blind and begins
			//// flop, trun, river: big blind begins

			//// !!!!!!!!! attention: exception if player failed before !!!!!!!!

// 			if( !(myHand->getActualQuantityPlayers() < 3 && firstHeadsUpRound == 1) || myHand->getPlayerArray()[playersTurn]->getMyActiveStatus() == 0 ) { 
// 			not first round in heads up (for headsup dealer is smallblind so it is dealers turn)
		
				// naechsten Spieler ermitteln
// 				int i;
// 				for(i=0; (i<MAX_NUMBER_OF_PLAYERS && ((myHand->getPlayerArray()[playersTurn]->getMyActiveStatus()) == 0 || (myHand->getPlayerArray()[playersTurn]->getMyAction())==1 || (myHand->getPlayerArray()[playersTurn]->getMyAction())==6)) || i==0; i++) {
// 					playersTurn = (playersTurn+1)%(MAX_NUMBER_OF_PLAYERS);
// 				}
// 				firstHeadsUpRound = 0; 




				// determine next running player
				currentPlayersTurnIt++;
				if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();









// 			}
// 			else { firstHeadsUpRound = 0; }

			////// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ////////////////

			//Spieler-Position vor SmallBlind-Position ermitteln 
// 			int activePlayerBeforeSmallBlind = smallBlindPosition;

// 			for(i=0; (i<MAX_NUMBER_OF_PLAYERS && !(myHand->getPlayerArray()[activePlayerBeforeSmallBlind]->getMyActiveStatus()) || (myHand->getPlayerArray()[activePlayerBeforeSmallBlind]->getMyAction())==1 || (myHand->getPlayerArray()[activePlayerBeforeSmallBlind]->getMyAction())==6) || i==0; i++) {

// 				activePlayerBeforeSmallBlind = (activePlayerBeforeSmallBlind + MAX_NUMBER_OF_PLAYERS - 1 ) % (MAX_NUMBER_OF_PLAYERS);
// 			}

// 			myHand->getPlayerArray()[playersTurn]->setMyTurn(1);

			(*currentPlayersTurnIt)->setMyTurn(true);


			//highlight active players groupbox and clear action
			myHand->getGuiInterface()->refreshGroupbox((*currentPlayersTurnIt)->getMyID(),2);
			myHand->getGuiInterface()->refreshAction((*currentPlayersTurnIt)->getMyID(),0);

			// wenn wir letzter aktiver Spieler vor SmallBlind sind, dann flopFirstRound zuende
			// ausnahme bei heads up !!! --> TODO
// 			if(myHand->getPlayerArray()[playersTurn]->getMyID() == activePlayerBeforeSmallBlind && myHand->getActivePlayerList()->size() >= 3) { firstRound = 0; }
// 			if(myHand->getActivePlayerList()->size() < 3 && (myHand->getPlayerArray()[playersTurn]->getMyID() == dealerPosition || myHand->getPlayerArray()[playersTurn]->getMyID() == smallBlindPosition)) { firstRound = 0; }




			if( (*currentPlayersTurnIt) == (*lastPlayersTurnIt) ) {
				firstRound = false;
			}






			if((*currentPlayersTurnIt)->getMyID() == 0) {
				// Wir sind dran
				myHand->getGuiInterface()->meInAction();
			}
			else {
				//Gegner sind dran
				myHand->getGuiInterface()->beRoAnimation2(myBeRoID);
			}
		}
	}
}


