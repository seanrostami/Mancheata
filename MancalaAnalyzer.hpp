#ifndef SJR_MANCALAANALYZER
#define SJR_MANCALAANALYZER


#include "MancalaGame.hpp"


/********************************************************************************************************************************/


class MancalaLookahead {

	public:

		static MancalaLookahead* Instance( int alternations ){ // [Singleton]
			MancalaLookahead::alternations = alternations;
			if( MancalaLookahead::instance == 0 )
				MancalaLookahead::instance = new MancalaLookahead;
			return MancalaLookahead::instance;
		}
		
		MancalaBoard::pit_t Recommendation( const MancalaGame * pgame ){
			return pgame->Balance() < 0 ? BestBalanceAfter( pgame, alternations ).choice : BestMarginAfter( pgame, alternations ).choice;
			/* The idea here is that if the user has at least as many stones on his 
			side than the opponent then the algorithm should try to maximize the 
			difference between the players' scores. But, if the user has fewer 
			stones (what the exact cutoff should be is debatable) then the 
			algorithm should try to rectify that.

			The problem with naive maximization of score differential ("margin") 
			is that if all of one player's pits become empty then the opponent 
			captures all remaining stones on his own side, and this can be substantial. 
			Although the BestMarginAfter algorithm does indeed take this into account, 
			the depth of lookahead may not be sufficient to notice this possibility 
			until it is too late. Hence, the hybrid algorithm. */
		}

	private:

		static MancalaLookahead* instance; // initialized below [Singleton]

		MancalaLookahead() {}; // prohibit instantiation [Singleton]

		struct Forecast{
			MancalaBoard::pit_t choice;
			MancalaBoard::stones_t differential;
			Forecast() : choice(-1), differential(-127) {}
		};

		Forecast BestMarginAfter( const MancalaGame * pgame, int alternations );
		Forecast BestBalanceAfter( const MancalaGame * pgame, int alternations );
		/* Two subtle but important points are (1) which quantity to maximize is decided 
		once and then the subsequent lookahead considers all moves from that perspective, 
		even if the basis on which the quantity was first chosen changes during the 
		lookahead (2) the determination of which move should be performed on a turn is 
		based only on the true state of the game, and so the second move recommended may 
		not be the move that was "intended" to follow the first move recommended. */

		static int alternations;

};


static MancalaLookahead* MancalaLookahead::instance = 0; // [Singleton]

static int MancalaLookahead::alternations = 1;


/********************************************************************************************************************************/


MancalaLookahead::Forecast MancalaLookahead::BestMarginAfter( const MancalaGame * pgame, int alternations ){

	MancalaGame hypothetical = *pgame;

	Forecast prediction;

	if( pgame->GameEndsNow() ){ // current player has no stones on side, game ends immediately without move
		hypothetical.FinalizeGame(); // award opponent everything on his side
		prediction.differential = hypothetical.Margin(); // resulting margin is what it is
		return prediction;
	}

	MancalaBoard::stones_t diff;

	for( MancalaBoard::pit_t choice = 0; choice < MancalaBoard::NUM_PITS; choice++ ){

		if( !pgame->ValidMove( choice ) )
			continue;

		hypothetical = pgame->SimulateMove( choice ); // when considering hypothetical moves, important to always start from the given state

		if( hypothetical.Player() == pgame->Player() ){ // the player making this move will receive another move
			diff = BestMarginAfter( &hypothetical, alternations ).differential;
			// best margin guaranteed by this move: same as that guaranteed after the turn's extra move
		}
		else{ // turn will transfer to opponent after this move, but lookahead may not consider further turns
			if( alternations >= 1 ){ // lookahead allowed to consider more turns
				diff = -BestMarginAfter( &hypothetical, alternations-1 ).differential;
				// best margin guaranteed by this move: the value p1-p2, where p2-p1 [sic] is the best margin that the *opponent* can force (assumption is that opponent plays well, whatever that means))
			}
			else{ // lookahead must stop after this move
				diff = -hypothetical.Margin();
				// best margin guaranteed by this move: whatever the board shows now (but remember that hypothetical is from opponent's perspective)
			}
		}

		if( diff > prediction.differential ){ // record which move guarantees the most
			prediction.differential = diff;
			prediction.choice = choice;
		}
		
	}

	return prediction;
}


MancalaLookahead::Forecast MancalaLookahead::BestBalanceAfter( const MancalaGame * pgame, int alternations ){

	MancalaGame hypothetical = *pgame;

	Forecast prediction;

	if( pgame->GameEndsNow() ){ // current player has no stones on side, game ends immediately without move
		prediction.differential = hypothetical.Balance(); // resulting balance is what it is
		return prediction;
	}

	MancalaBoard::stones_t diff;

	for( MancalaBoard::pit_t choice = 0; choice < MancalaBoard::NUM_PITS; choice++ ){

		if( !pgame->ValidMove( choice ) )
			continue;

		hypothetical = pgame->SimulateMove( choice ); // when considering hypothetical moves, important to always start from the given state

		if( hypothetical.Player() == pgame->Player() ){ // the player making this move will receive another move
			diff = BestMarginAfter( &hypothetical, alternations ).differential;
			// best balance guaranteed by this move: same as that guaranteed after the turn's extra move
		}
		else{ // turn will transfer to opponent after this move, but lookahead may not consider further turns
			if( alternations >= 1 ){ // lookahead allowed to consider more turns
				diff = -BestMarginAfter( &hypothetical, alternations-1 ).differential;
				// best balance guaranteed by this move: the value p1-p2, where p2-p1 [sic] is the best margin that the *opponent* can force (assumption is that opponent plays well, whatever that means))
			}
			else{ // lookahead must stop after this move
				diff = -hypothetical.Balance();
				// best balance guaranteed by this move: whatever the board shows now (but remember that hypothetical is from opponent's perspective)
			}
		}

		if( diff > prediction.differential ){ // record which move guarantees the most
			prediction.differential = diff;
			prediction.choice = choice;
		}
		
	}

	return prediction;
}


#endif // SJR_MANCALAANALYZER
