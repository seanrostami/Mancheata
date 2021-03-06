#ifndef SJR_MANCALABOARD
#define SJR_MANCALABOARD


//#include <cstdint>
/* Arduino IDE automatically includes some stuff, like stdint, when 
building from .ino, but need stdint when using other build tools */


/********************************************************************************************************************************/


/* MancalaBoard contains a "snapshot" of the board at some time: the number of stones in each pit, the number of stones in each store (i.e. scores), and which player has the turn.
	By modifying MancalaBoard, variations on the U.S.-style game Kalah are obtained. For example, different number of pits, different number of stones per pit, etc.
	Also, there are methods to query or change the state of the board. These methods perform somewhat atomic operations that are ignorant of the particular style of gameplay 
	and universal to all such styles. For example, removing all stones from a pit is something that must always be done while the rules for capture can be slightly different 
	from one style to another. Thus, the former is within the scope of MancalaBoard and the latter is not. The particulars of gameplay are handled by MancalaGame. */

class MancalaBoard{

	friend class MancalaGame;

	public:
	
		typedef int8_t stones_t; // counts stones (important to be SIGNED)

		typedef int8_t pit_t; // counts and indexes pits (important to be SIGNED)
	
		static const pit_t		NUM_PITS		= 12;
	
			/* with NUM_PITS==12, 
				     PLAYER #2
			   (11) (10) (09) (08) (07) (06)
			()                               ()
			   (00) (01) (02) (03) (04) (05)
				     PLAYER #1
				      (=USER)
			*/

	private:
  
	  	typedef uint64_t distribution_t; // see member distribution for explanation

	  	distribution_t 	distribution;
		/* Member distribution encodes in its least-significant 60 bits the state of the twelve pits:
		- the least significant 30 bits encode Player #1's pits, the next 30 bits encode Player #2's pits, and the highest 4 bits are unused;
		- each block of 30 bits is divided into six segments of 5 bits and each segment corresponds to a pit;
		- each segment is interpreted as an ordinary unsigned integer and that integer is the number of stones in the pit, so each pit can contain a maximum of 31 stones.
		Note that it is extremely unlikely, and probably impossible (proof?), to have more than 31 stones in a single pit. 
		A single move can increase the number of stones in a pit by at most one, and each pit starts with four stones. 
		So, even if no stones were ever removed from play (capture, etc.) and somehow there were a pit that increased 
		by one during every move without exception, and that pit were never itself selected as a move, it would still 
		require 28 moves to exceed the maximum. */
	   
		stones_t p1score;
		stones_t p2score;
		bool player; // true <=> Player #1's turn
	
		static const stones_t		NUM_BITS_PER_PIT = (8*sizeof(MancalaBoard::distribution_t))/NUM_PITS; // = 5, when NUM_PITS==12
		static const stones_t		NUM_BITS_PER_PLAYER = NUM_BITS_PER_PIT*NUM_PITS/2; // = 30, when NUM_PITS==12
		static const distribution_t	LOWEST_PIT_MASK = (1 << NUM_BITS_PER_PIT) - 1; // = 0x1F, when NUM_PITS==12
		static const distribution_t	PLAYER1_SIDE_MASK = (((distribution_t)1) << NUM_BITS_PER_PLAYER) - 1; // = 0x3FFFFFFF, when NUM_PITS==12

	public:		

		static pit_t OppositePitIndex( pit_t i ){
			return NUM_PITS-1 - i; // sends 0,1,2,3,4,5 to 11,10,9,8,7,6 etc.
		}

		static bool PitOwnedByPlayer( pit_t i, bool player ) {
			return player ? ( 0 <= i && i < (NUM_PITS/2) ) : ( (NUM_PITS/2) <= i && i < NUM_PITS );
		}

		MancalaBoard() : distribution( 0x210842108421084 ), p1score(0), p2score(0), player(true) {
			// 0x210842108421084 is twelve consecutive segments of 00100 (four stones), starting at the least significant side
		}

		// utilize default copy-constructor, default assignment, default (empty) destructor

		void RemoveAllStonesFrom( pit_t i ){
			distribution &= ~( LOWEST_PIT_MASK << (i*NUM_BITS_PER_PIT) ); // right operand has all bits 0 in the segment corresponding to Pit #i and 1 elsewhere
		}

		void AddOneStoneTo( pit_t i ){
			distribution += ( ((distribution_t)1) << (i*NUM_BITS_PER_PIT) ); // right operand has a 1 in the rightmost bit of the segment corresponding to Pit #i and 0 elsewhere
		}

		void ChangeTurn( void ){
			player = !player;
		}

		stones_t NumStonesInPit( pit_t i ) const {
			return ( distribution >> (i*NUM_BITS_PER_PIT) ) & LOWEST_PIT_MASK; // shift Pit #i's segment to the bottom and clear/zero everything else
		}

		stones_t NumStonesOnSide( bool player ) const {
			distribution_t d = ( player ? ( distribution & PLAYER1_SIDE_MASK ) : ( distribution >> NUM_BITS_PER_PLAYER ) ); // arrange for the appropriate 30-bit segment to occupy the least-significant bits with 0 elsewhere
			stones_t total = 0;
			while( d ){ // consider each 5-bit segment to be an independent integer and sum them
				total += d & LOWEST_PIT_MASK;
				d >>= NUM_BITS_PER_PIT;
			}
			return total;
		}

};


#endif // SJR_MANCALABOARD
