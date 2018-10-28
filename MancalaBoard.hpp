#ifndef SJR_MANCALABOARD
#define SJR_MANCALABOARD


//#include <cstdint>


/********************************************************************************************************************************/


/* MancalaBoard contains a "snapshot" of the board at some time: the number of stones in each pit, the number of stones in each store (i.e. scores), and which player has the turn.
	By modifying MancalaBoard, variations on the U.S.-style game Kalah are obtained. For example, different number of pits, different number of stones per pit, etc.
	MancalaBoard does NOT contain anything dependent on the particular style of play, of which there are many. These things are handled by MancalaGame. */
class MancalaBoard{

	friend class MancalaGame;

	public:

		typedef int8_t pit_t; // counts and indexes pockets (important to be SIGNED)
		typedef int8_t stones_t; // counts stones (important to be SIGNED)

	private:
  
	  	typedef uint64_t distribution_t;

	  	distribution_t 	distribution;
		/* encodes in its least-significant 60 bits the state of the twelve pockets:
		the least significant 30 bits encode Player #1's pockets, the next 30 bits encode Player #2's pockets, and the highest 4 bits are unused;
		each block of 30 bits is divided into six segments of 5 bits and each segment corresponds to a pocket;
		each segment is interpreted as an ordinary unsigned integer and that integer is the number of stones in the pocket, so each pocket can contain a maximum of 31 stones
		(probably it can be proved that a real game can never produce a pocket with more stones than this) */
	   
		stones_t p1score;
		stones_t p2score;
		bool player; // true <=> Player #1's turn

	public:

		static const pit_t		NUM_PITS		= 12;
		static const pit_t		NUM_STONES_PER_PIT	= 4;
		static const stones_t		NUM_BITS_PER_PIT; // = 5
		static const stones_t		NUM_BITS_PER_PLAYER; // = 30
		static const distribution_t	LOWEST_PIT_MASK; // = 0x1F
		static const distribution_t	PLAYER1_SIDE_MASK; // = 0x3FFFFFFF

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
			distribution_t spacious1 = 1; // important to use a type that is large enough to shift, safer than using 1ULL
			distribution += ( spacious1 << (i*NUM_BITS_PER_PIT) ); // right operand has a 1 in the rightmost bit of the segment corresponding to Pit #i and 0 elsewhere
		}

		void ChangeTurn( void ){
			player = !player;
		}

		stones_t NumStonesInPit( pit_t i ) const {
			return ( distribution >> (i*NUM_BITS_PER_PIT) ) & LOWEST_PIT_MASK; // shift Pit #i's segment to the bottom and clear/zero everything else
		}

		stones_t NumStonesOnSide( bool player ) const;

};


/********************************************************************************************************************************/


const MancalaBoard::stones_t		MancalaBoard::NUM_BITS_PER_PIT		= (8*sizeof(MancalaBoard::distribution_t))/NUM_PITS;
const MancalaBoard::stones_t		MancalaBoard::NUM_BITS_PER_PLAYER	= NUM_BITS_PER_PIT*NUM_PITS/2;
const MancalaBoard::distribution_t	MancalaBoard::LOWEST_PIT_MASK		= (1 << NUM_BITS_PER_PIT) - 1;
const MancalaBoard::distribution_t	MancalaBoard::PLAYER1_SIDE_MASK		= (1ULL << NUM_BITS_PER_PLAYER) - 1;


/********************************************************************************************************************************/


MancalaBoard::stones_t MancalaBoard::NumStonesOnSide( bool player ) const {
	distribution_t d = ( player ? ( distribution & PLAYER1_SIDE_MASK ) : ( distribution >> NUM_BITS_PER_PLAYER ) ); // arrange for the appropriate 30-bit segment to occupy the least-significant bits with 0 elsewhere
	stones_t total = 0;
	while( d ){ // consider each 5-bit segment to be an independent integer and sum them
		total += d & LOWEST_PIT_MASK;
		d >>= NUM_BITS_PER_PIT;
	}
	return total;
}


#endif // SJR_MANCALABOARD
