#ifndef SJR_MANCALA
#define SJR_MANCALA


#include <stdint.h>


/********************************************************************************************************************************/


typedef uint64_t distribution_t; // see struct GameState for purpose
typedef int8_t pocket_t; // indexes pockets (important to be SIGNED since sometimes do subtractions as part of modular arithmetic)
typedef int8_t stones_t; // counts stones (important to be SIGNED since sometimes need to consider score differentials, which may be negative)
// better to use int for pocket_t and stones_t?


/********************************************************************************************************************************/


#define NUM_POCKETS 12 // standard game in the United States, which is technically called "Kalah", has six pockets per player...
#define NUM_STONES_PER_POCKET 4 // ... and four stones per pocket at the start of the game

#define NUM_BITS_PER_POCKET ((8*sizeof(distribution_t))/NUM_POCKETS) // truncates
#define MAX_STONES_PER_POCKET ((1<<NUM_BITS_PER_POCKET)-1) // 2^NUM_BITS_PER_POCKET - 1

#define NUM_BITS_PER_PLAYER (NUM_BITS_PER_POCKET*NUM_POCKETS/2)

#define PLAYER1_SIDE_MASK 0x3fffffff // least-significant 30 bits are 1 and all other bits are 0, i.e. 0000000000000000000000000000000000111111111111111111111111111111

#define PLAYER2_SIDE_MASK 0xfffffffc0000000 // least-significant 30 bits and most-significant 4 bits are 0, remaining 30 bits are 1, i.e. 0000111111111111111111111111111111000000000000000000000000000000

#define LOWEST_POCKET_MASK 0x1f // binary 11111 (or 0000000000000000000000000000000000000000000000000000000000011111 in 64-bit)

#define INVALID_POCKET -1

#define INITIAL_DISTRIBUTION 0x210842108421084 /* twelve consecutive segments of 00100 (four stones), starting at the least significant side, 
i.e. 0000001000010000100001000010000100001000010000100001000010000100
this cannot be a const -- it is used to initialize a static variable */

const distribution_t POCKET_CLEAR_MASKS[NUM_POCKETS]; // defined in mancala.c

const distribution_t POCKET_ADD_MASKS[NUM_POCKETS]; // defined in mancala.c


/********************************************************************************************************************************/


struct GameState{
  distribution_t  distribution; /* encodes in its least-significant 60 bits the state of the twelve pockets:
  the least significant 30 bits encode Player #1's pockets, the next 30 bits encode Player #2's pockets, and the highest 4 bits are unused;
  each block of 30 bits is divided into six segments of 5 bits and each segment corresponds to a pocket;
  each segment is interpreted as an ordinary unsigned integer and that integer is the number of stones in the pocket, so each pocket can contain a maximum of 31 stones
  (I strongly suspect one can prove rigorously that a real game can never produce a pocket with more stones than this) */
  stones_t  p1score;
  stones_t  p2score;
  _Bool   p1turn; // user is always "Player #1"
  // could represent p1turn by the highest bit of distribution (the most significant four bits are unused), but I don't think it's worthwhile
};
// on the Nano, sizeof(struct GameState)==10 but sizeof(void*)==2 so worthwhile to use const* arguments

struct RecommendedMove{
  pocket_t choice;
  stones_t differential;
};


/********************************************************************************************************************************/

// TO DO: move some of these to mancala.c, and probably then can hide struct RecommendedMove altogether

stones_t CountAllPlayerStones( distribution_t d, _Bool p1turn );


inline stones_t CountAllStonesIn( distribution_t d, pocket_t pocketnum ){
  return ( d >> (pocketnum*NUM_BITS_PER_POCKET) ) & LOWEST_POCKET_MASK; // shift the relevant pocket's segment all the way to the bottom, then clear/zero everything else
}


inline distribution_t RemoveAllStonesFrom( distribution_t d, pocket_t pocketnum ){
  return d & POCKET_CLEAR_MASKS[pocketnum]; // clear/zero the relevant pocket's segment
}


inline distribution_t AddOneStoneTo( distribution_t d, pocket_t pocketnum ){
  return d + POCKET_ADD_MASKS[pocketnum]; // "add 1" to the relevant pocket's segment
}


inline pocket_t GetOppositePocketNum( pocket_t pocketnum ){
  return NUM_POCKETS - 1 - pocketnum; // sends 0,1,2,3,4,5 to 11,10,9,8,7,6 etc.
}


// checks whether the given pocket is on the side of the player specified by pgame->p1turn
inline _Bool CheckIfOwnSide( const struct GameState * pgame, pocket_t pocketnum ){
  return ( pgame->p1turn && ( pocketnum < (NUM_POCKETS/2) ) ) || ( !pgame->p1turn && ( pocketnum >= (NUM_POCKETS/2) ) );
}


/* assumes (doesn't check) that arguments are valid, e.g. that pocketnum is owned by the player specified by game.p1turn, that there are stones in that pocket, etc. 
parameter pocketnum is 0-based, so 0 to (NUM_POCKETS/2)-1 indicate Player #1's pockets while (NUM_POCKETS/2) to NUM_POCKETS-1 indicate Player #2's pockets */
struct GameState SimulateMove( struct GameState game, pocket_t pocketnum );


// updates *pcurrent with the result of SimulateMove, used only when a genuine move is made by a player
void PerformMove( struct GameState * pcurrent, pocket_t pocketnum );


/* determines the best score attainable by whichever player is specified by pgame->p1turn, with parameter alternations bounding the number of hypothetical turns to consider.
here, "best score" does not mean the highest score attainable by the current player.
rather, it means the largest *difference* in score between the current player and the opponent.
the implementation is recursive: among the possible moves available to the current player, the best is the one for which the opponent's "best score" relative to that resulting game state is lowest. */
struct RecommendedMove BestPossibleScoreAfter( const struct GameState * pgame, int alternations );


// returns the move that is claimed to produce the best score differential after alternations+1 turns
pocket_t RecommendMove( const struct GameState * pcurrent, int alternations );


/********************************************************************************************************************************/


#endif // SJR_MANCALA
