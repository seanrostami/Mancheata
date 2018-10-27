
#include "MancalaBoard.hpp"


/********************************************************************************************************************************/


class MancalaGame{

	MancalaBoard board;

	public:

		// utilize default default-constructor, copy-constructor, assignment, (empty) destructor

		void operator = ( MancalaBoard b ){ // makes .SimulateMove more convenient
			board = b;
		}

		bool GameEndsNow( void ) const {
			return board.NumStonesOnSide( board.player ) == 0;
		}

		void FinalizeGame( void );

		bool ValidMove( MancalaBoard::pit_t i ) const {
			return MancalaBoard::PitOwnedByPlayer( i, board.player ) && board.NumStonesInPit( i ) > 0;
		}

		MancalaBoard SimulateMove( MancalaBoard::pit_t i ) const;

		void PerformMove( MancalaBoard::pit_t i ){
			board = SimulateMove( i );
		}

		bool Player( void ) const {
			return board.player;
		}

		void Pass( void ){
			board.ChangeTurn();
		}

		MancalaBoard::stones_t Margin( void ) const { // returns difference in scores *from the perspective of the player whose turn it is now*
			return board.player ? ( board.p1score - board.p2score ) : ( board.p2score - board.p1score );
		}

		MancalaBoard::stones_t Balance( void ) const { // returns difference in sides' numbers of stones, *from the perspective of the player whose turn it is now*
			return board.NumStonesOnSide( board.player ) - board.NumStonesOnSide( !board.player );
		}
};


/********************************************************************************************************************************/


// WARNING: assumes *current* player is the one with the empty side, awards opponent all remaining stones
void MancalaGame::FinalizeGame( void ){ 
	for( MancalaBoard::pit_t i = 0; i < MancalaBoard::NUM_PITS; i++ ){
		if( MancalaBoard::PitOwnedByPlayer( i, !board.player ) ){
			if( board.player )
				board.p2score += board.NumStonesInPit( i );
			else
				board.p1score += board.NumStonesInPit( i );
			board.RemoveAllStonesFrom( i );
		}
	}
}

MancalaBoard MancalaGame::SimulateMove( MancalaBoard::pit_t i ) const {

	MancalaBoard simulation = board;

	MancalaBoard::stones_t tosow = simulation.NumStonesInPit( i );

	// Check first if any stones will be placed in the player's own store and, if so, increase score and remove them (also determine if current player will play again)
	MancalaBoard::pit_t K = i + tosow - ( simulation.player ? MancalaBoard::NUM_PITS/2 : MancalaBoard::NUM_PITS );
	while( K >= 0 ){
		if( simulation.player )
			simulation.p1score++;
		else
			simulation.p2score++;
		tosow--;
		K -= 1 + MancalaBoard::NUM_PITS;
	}

	bool playagain = ( K + 1 + MancalaBoard::NUM_PITS == 0 );

	// Distribute counterclockwise into pits all stones that were not preemptively put into the player's own store
	simulation.RemoveAllStonesFrom( i );
	for( MancalaBoard::pit_t j = 1; j <= tosow; j++ )
		simulation.AddOneStoneTo( ( i + j ) % MancalaBoard::NUM_PITS );

	// If known (from above) that final stone doesn't land in player's own store then check whether "capture" occurred and adjust scores/pits and then opponent plays
	if( !playagain ){
		MancalaBoard::pit_t last = ( i + tosow ) % MancalaBoard::NUM_PITS; // position of final stone (deposits in the store were already subtracted above)
		if( simulation.NumStonesInPit( last ) == 1 && MancalaBoard::PitOwnedByPlayer( last, simulation.player ) ){
			if( simulation.player )
				simulation.p1score += 1 + simulation.NumStonesInPit( MancalaBoard::OppositePitIndex( last ) ); // also get the stone that was placed into the empty pit
			else
				simulation.p2score += 1 + simulation.NumStonesInPit( MancalaBoard::OppositePitIndex( last ) ); // also get the stone that was placed into the empty pit
			simulation.RemoveAllStonesFrom( last );
			simulation.RemoveAllStonesFrom( MancalaBoard::OppositePitIndex( last ) );
		}
		simulation.ChangeTurn();
	}

	return simulation;
}
