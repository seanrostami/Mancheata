
#include "MancalaAnalyzer.hpp"


/********************************************************************************************************************************/


#define VIBRATOR_PWM_PIN		9 // must be PWM to satisfy Vibrator's medium voltage requirement!
#define BUTTON_INT_PIN			2 // must be INT to respond to Button!
#define LED_PIN				13 // mostly for testing purposes

#define BUTTON_DEBOUNCE_DELAY		1750 // milliseconds, how long after the Button's interrupt occurs to ignore further interrupts (part of "debouncing")
#define BUTTON_NUM_ATTEMPTS		7 // how many intervals in which to accept a Button (must be at least 7!)
#define BUTTON_RECEPTIVE_DURATION	150 // milliseconds, how long to vibrate when alerting the player that input is enabled after debouncing one signal

#define VIBRATOR_STRENGTH		0.75 // percentage, between 0 and 1
#define VIBRATOR_MAXIMUM_SIGNAL		153 // input to analogWrite from 0 to 255, results in 0V to 5V, so 153 corresponds to Vibrator's rated max of 3V

#define RECOMMENDATION_ANNOUNCE_DELAY	1500 // milliseconds, how long to wait after calculating the recommendation to announce ("user experience")
#define RECOMMENDATION_PULSE_DURATION	500 // milliseconds, how long is each pulse used to announce the recommendation to the player (N pulses <-> pocket #N)
#define RECOMMENDATION_PULSE_DELAY	250 // milliseconds, how long to wait between the pulses used to recommend

#define LOOKAHEAD_DEPTH			5 /* number of turns to consider when predicting best move 
						note that this is usually different from the number of moves performed, since a player receives an extra turn for depositing the last stone into his store
						ex: 4 means simulate Player #1's move(s) followed by Player #2's move(s) followed by Player #1's move(s) followed by Player #2's move(s) */


/********************************************************************************************************************************/


volatile int num_presses = 0;

volatile bool ignore_button = false; // used to discard "fake" interrupts (faster/cleaner/easier than enabling/disabling interrupt)


/********************************************************************************************************************************/


void announceGeneric( int N ); // mostly for testing purposes: indicate via LED what input was actually received (vs. what user thinks was sent)

void onButtonPress( void ); // attached to FALLING event of BUTTON_INT_PIN

void announceReceptivity( void ); // indicates that program is ready to receive another Button-press, after previous press was "debounced"

void announceLookahead( void ); // mostly for testing purposes: indicates via LED which player is expected to move next

void announceRecommendation( MancalaBoard::pit_t choice0to5 );


/********************************************************************************************************************************/


void announceGeneric( int N ){
	while( N-- > 0 ){
		delay( 250 );
		digitalWrite( LED_PIN, HIGH );
		delay( 250 );
		digitalWrite( LED_PIN, LOW );
	}
}


// attached to FALLING event of BUTTON_INT_PIN
void onButtonPress( void ){
	if( !ignore_button ){
		ignore_button = true;
		num_presses++;
	}
}


void announceReceptivity( void ){
	analogWrite( VIBRATOR_PWM_PIN, min( (int)(VIBRATOR_STRENGTH*VIBRATOR_MAXIMUM_SIGNAL), VIBRATOR_MAXIMUM_SIGNAL ) ); // Vibrator ON
	delay( BUTTON_RECEPTIVE_DURATION );
	digitalWrite( VIBRATOR_PWM_PIN, LOW ); // Vibrator OFF
}


void announceLookahead( void ){
	digitalWrite( LED_PIN, HIGH );
	delay( 2000 );
	digitalWrite( LED_PIN, LOW );
}


void announceRecommendation( MancalaBoard::pit_t choice0to5 ){
	while( ( 1 + choice0to5-- ) > 0 ){ // notify player of recommendation: pulse Vibrator x choice1to6
		analogWrite( VIBRATOR_PWM_PIN, min( (int)(VIBRATOR_STRENGTH*VIBRATOR_MAXIMUM_SIGNAL), VIBRATOR_MAXIMUM_SIGNAL ) ); // Vibrator ON
		delay( RECOMMENDATION_PULSE_DURATION );
		digitalWrite( VIBRATOR_PWM_PIN, LOW ); // Vibrator OFF
		delay( RECOMMENDATION_PULSE_DELAY );
	}
}


/********************************************************************************************************************************/


void setup() {

	pinMode( VIBRATOR_PWM_PIN, OUTPUT );
	digitalWrite( VIBRATOR_PWM_PIN, LOW );
	
	pinMode( BUTTON_INT_PIN, INPUT_PULLUP ); // to deal with noise on pin, must use HIGH as default
	attachInterrupt( digitalPinToInterrupt(BUTTON_INT_PIN), onButtonPress, FALLING ); // since INPUT_PULLUP, pressing button will cause voltage to drop

	pinMode( LED_PIN, OUTPUT );
	digitalWrite( LED_PIN, LOW );

	// very first turn
	MancalaGame initial;
	announceLookahead();
	announceRecommendation( MancalaLookahead::Instance( LOOKAHEAD_DEPTH-1 )->Recommendation( &initial ) );
	
}


/********************************************************************************************************************************/


void loop() {
	
	static MancalaGame game;
	
	if( num_presses > 0 ){
		
		for( int i = 0; i < BUTTON_NUM_ATTEMPTS; i++ ){ // wait for all presses to be recorded, and also ignore "fake" interrupts
			delay( BUTTON_DEBOUNCE_DELAY ); // pause to allow "bounce" to fade
			announceReceptivity(); // gently signal to player that input is enabled
			ignore_button = false; // enable reception of input
		}
		delay( BUTTON_DEBOUNCE_DELAY ); // without this, user doesn't actually have time to press Button (delay happens at start of loop)

		announceGeneric( num_presses ); // mostly for testing: echo the supposed input

		if( num_presses <= MancalaBoard::NUM_PITS/2 ){

			MancalaBoard::pit_t choice = ( game.Player() ? num_presses-1 : MancalaBoard::OppositePitIndex( num_presses-1 ) );
			if( game.ValidMove( choice ) )
				game.PerformMove( choice ); // convert move# to the absolute version that PerformMove expects
			// if the input is within the proper range but invalid, do nothing: turn does not change, move is not performed, globals are reset, and recommendation is repeated if there was one already
	
			if( game.Player() ){ // irrelevant of who played the previous turn, if it is user's turn next then recommend a move
				delay( RECOMMENDATION_ANNOUNCE_DELAY );
				announceLookahead();
				announceRecommendation( MancalaLookahead::Instance( LOOKAHEAD_DEPTH-1 )->Recommendation( &game ) );
			}
			
		}
		else // if num_presses is out-of-bounds, interpret as "pass" (besides robustness, also used by user to become Player #2)
			game.Pass();
		
		num_presses = 0;
		ignore_button = false; // if user presses Button at last opportunity then ignore_button == 1 but won't be reset by above for loop

	}
	
}
