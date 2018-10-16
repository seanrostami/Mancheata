
#include "mancala.h"


/********************************************************************************************************************************/

const distribution_t POCKET_CLEAR_MASKS[NUM_POCKETS] = { 0xfffffffffffffe0, 0xffffffffffffc1f, 0xfffffffffff83ff, 0xffffffffff07fff, 0xffffffffe0fffff, 0xfffffffc1ffffff, 0xffffff83fffffff, 0xfffff07ffffffff, 0xfffe0ffffffffff, 0xffc1fffffffffff, 0xf83ffffffffffff, 0x7fffffffffffff }; // one 5-bit segment is 00000, all other bits 1

const distribution_t POCKET_ADD_MASKS[NUM_POCKETS] = { 0x1, 0x20, 0x400, 0x8000, 0x100000, 0x2000000, 0x40000000, 0x800000000, 0x10000000000, 0x200000000000, 0x4000000000000, 0x80000000000000 }; // one 5-bit segment is 00001, all other bits 0


/********************************************************************************************************************************/

extern inline stones_t         CountAllStonesIn( distribution_t d, pocket_t pocketnum ); // inlined in mancala.h
extern inline distribution_t  RemoveAllStonesFrom( distribution_t d, pocket_t pocketnum ); // inlined in mancala.h
extern inline distribution_t  AddOneStoneTo( distribution_t d, pocket_t pocketnum ); // inlined in mancala.h
extern inline pocket_t        GetOppositePocketNum( pocket_t pocketnum ); // inlined in mancala.h
extern inline _Bool           CheckIfOwnSide( const struct GameState * pgame, pocket_t pocketnum ); // inlined in mancala.h


/********************************************************************************************************************************/


// counts all stones in player's six pockets
stones_t CountAllPlayerStones( distribution_t d, _Bool p1turn ){
  distribution_t dd = ( p1turn ? ( d & PLAYER1_SIDE_MASK ) : ( ( d & PLAYER2_SIDE_MASK ) >> NUM_BITS_PER_PLAYER ) );
  stones_t total = 0;
  while( dd ){
    total += dd & LOWEST_POCKET_MASK;
    dd >>= NUM_BITS_PER_POCKET;
  }
  return total;
}


struct GameState SimulateMove( struct GameState game, pocket_t pocketnum ){ // don't use const* for first argument: really want to treat game as a local variable and modify

  stones_t tosow = CountAllStonesIn( game.distribution, pocketnum );

  // Check early if any stones will be placed in the player's own store and, if so, increase score and remove them
  pocket_t K = pocketnum + tosow - ( game.p1turn ?  NUM_POCKETS/2 : NUM_POCKETS );
  while( K >= 0 ){ // better not to include <stdlib.h> just to do division-with-remainder
    if( game.p1turn ) // each time the distribution would pass player's store, increase their score by one stone...
      game.p1score++;
    else
      game.p2score++;
    tosow--; // ... and decrease the total stones that will be sowed later by one
    K -= NUM_POCKETS+1; // including the player's store, one trip around the board uses NUM_POCKETS+1 stones
  }
  _Bool playagain = ( K + NUM_POCKETS+1 == 0 ); // this <=> original K was multiple of NUM_POCKETS+1

  // Distribute counterclockwise into pockets all stones that were not preemptively put into the player's own store
  game.distribution = RemoveAllStonesFrom( game.distribution, pocketnum );
  for( pocket_t i = 0; i < tosow; i++ )
    game.distribution = AddOneStoneTo( game.distribution, ( pocketnum + 1 + i ) % NUM_POCKETS );

  // If known (from above) that final stone lands in player's own store, player plays again...
  if( playagain )
    return game; // game.p1turn unchanged

  // ... otherwise, check if player "captures", adjust scores and remove stones if so, and then mark next turn for opponent
  pocket_t last = ( pocketnum + tosow ) % NUM_POCKETS; // position of final stone (deposits in the store already subtracted)
  if( CountAllStonesIn( game.distribution, last ) == 1 && CheckIfOwnSide( &game, last ) ){ // if final stone placed in an empty pocket owned by current player
    if( game.p1turn ) // increase the score
      game.p1score += 1 + CountAllStonesIn( game.distribution, GetOppositePocketNum( last ) );
    else
      game.p2score += 1 + CountAllStonesIn( game.distribution, GetOppositePocketNum( last ) );
    game.distribution = RemoveAllStonesFrom( game.distribution, last ); // empty this pocket...
    game.distribution = RemoveAllStonesFrom( game.distribution, GetOppositePocketNum( last ) ); // ... and opposing pocket
  }

  game.p1turn = !game.p1turn;

  return game;
}


void PerformMove( struct GameState * pcurrent, pocket_t pocketnum ){
  *pcurrent = SimulateMove( *pcurrent, pocketnum );
}


struct RecommendedMove BestPossibleScoreAfter( const struct GameState * pgame, int alternations ){

  struct RecommendedMove prediction = { .differential = -100, .choice = INVALID_POCKET };

  if( CountAllPlayerStones( pgame->distribution, pgame->p1turn ) == 0 ){ // regardless of lookahead depth requested, if out of stones then game would end and score differential would be fixed
    prediction.differential = ( pgame->p1turn ? ( pgame->p1score - pgame->p2score ) : ( pgame->p2score - pgame->p1score ) ) - CountAllPlayerStones( pgame->distribution, !pgame->p1turn ); // opponent's score now includes all the stones collected due to current player's emptiness
    return prediction;
  }

  struct GameState hypothetical;
  pocket_t choice;
  stones_t differential;

  for( pocket_t prechoice = 0; prechoice < NUM_POCKETS/2; prechoice++ ){
    choice = ( pgame->p1turn ? prechoice : GetOppositePocketNum( prechoice ) );

    if( CountAllStonesIn( pgame->distribution, choice ) == 0 ) // can't use empty pockets
      continue;

    hypothetical = SimulateMove( *pgame, choice ); // CAN perform choice even if alternations == 0, provided that the choice leads to a "bonus turn" and therefore does not require alternation
    if( hypothetical.p1turn == pgame->p1turn ){ // choice resulted in "bonus turn" because turn did not transfer
      differential = BestPossibleScoreAfter( &hypothetical, alternations ).differential;
    }
    else{ // choice requires turn to transfer, not necessarily possible
      if( alternations >= 1 ){ // turn allowed to transfer
        differential = -BestPossibleScoreAfter( &hypothetical, alternations-1 ).differential;
        /* If choice is performed by the current player then the opponent, who is playing well 
        by assumption, will play so that the best score differential FROM THE PERSPECTIVE OF THE 
        OPPONENT is produced. The return value of BestPossibleScoreAfter is this best differential 
        and, since it is from the perspective of the opponent, is of the form opponent-current. 
        From the perspective of the current player, that exact same future state has a score 
        differential of current-opponent. Hence, the return is negated. */
      }
      else{ // cannot transfer turn (alternations == 0)
        differential = ( pgame->p1turn ? ( hypothetical.p1score - hypothetical.p2score ) : ( hypothetical.p2score - hypothetical.p1score ) );
        /* in this case, differential is merely whatever current player can get by performing choice and immediately tabulating */
      }
    }
    if( differential > prediction.differential ){ // record the move whose guaranteed minimum differential is largest
      prediction.differential = differential;
      prediction.choice = choice;
    }
  }
  return prediction;
}


pocket_t RecommendMove( const struct GameState * pcurrent, int alternations ){
  return BestPossibleScoreAfter( pcurrent, alternations ).choice;
}
