
extern "C"{
#include "mancala.h"
}

// currently using Arduino IDE for building, probably silently #includes certain things, probably need to explicitly #include something here if a different toolchain is used


/********************************************************************************************************************************/


#define VIBRATOR_PWM_PIN                9 // must be PWM to satisfy Vibrator's medium voltage requirement!
#define BUTTON_INT_PIN                  2 // must be INT to respond to Button!
#define LED_PIN                         13 // mostly for testing purposes

#define BUTTON_ACCUMULATION_DELAY       12000 //milliseconds, how long *in total* to spend "debouncing" the Button's signals
#define INTERRUPT_RESET_DELAY           1500 // milliseconds, how long after the Button's interrupt occurs to ignore further interrupts (part of "debouncing")
#define VIBRATOR_RECEPTIVE_DURATION     150 // milliseconds, how long to vibrate when alerting the player that input is enabled after debouncing one signal

#define VIBRATOR_STRENGTH               0.8 // percentage, between 0 and 1
#define VIBRATOR_MAXIMUM_SIGNAL         153 // input to analogWrite from 0 to 255, results in 0V to 5V, so 153 corresponds to Vibrator's rated max of 3V

#define RECOMMENDATION_DELAY            1500 // milliseconds, how long to wait after calculating the recommendation to announce ("user experience")
#define RECOMMEND_PULSE_DURATION        500 // milliseconds, how long is each pulse used to announce the recommendation to the player (N pulses <-> pocket #N)
#define RECOMMEND_PULSE_DELAY           250 // milliseconds, how long to wait between the pulses used to recommend

#define LOOKAHEAD_DEPTH                 4 /* number of turns to consider when predicting best move 
note that this is usually different from the number of moves performed, since a player receives an extra turn for depositing the last stone into his store
ex: 4 means simulate Player #1's move(s) followed by Player #2's move(s) followed by Player #1's move(s) followed by Player #2's move(s) */


/********************************************************************************************************************************/


volatile int num_presses = 0;

volatile _Bool ignore_button = 0; // used to discard "fake" interrupts (faster/cleaner/easier than enabling/disabling interrupt)


/********************************************************************************************************************************/


void announcePlayer( _Bool p1 ); // mostly for testing purposes: indicates via LED which player is expected to move next

void onButtonPress( void );

void announceReceptivity( void ); // indicates that program is ready to receive another Button-press, after previous press was "debounced"

void announceLookahead( void ); // mostly for testing purposes: indicates via LED which player is expected to move next

void announceRecommendation( pocket_t choice0to5 );

extern inline pocket_t GetOppositePocketNum( pocket_t pocketnum ); // inlined in mancala.h


/********************************************************************************************************************************/


void announcePlayer( _Bool p1 ){
  delay( 500 );
  digitalWrite( LED_PIN, HIGH );
  delay( 250 );
  digitalWrite( LED_PIN, LOW );
  if( !p1 ){ // two blinks for Player #2, one blink for Player #1
    delay( 250 );
    digitalWrite( LED_PIN, HIGH );
    delay( 250 );
    digitalWrite( LED_PIN, LOW );
  }
}


// attached to FALLING event of BUTTON_INT_PIN
void onButtonPress( void ){
  if( !ignore_button ){
    ignore_button = 1;
    num_presses++;
  }
}


void announceReceptivity( void ){
  analogWrite( VIBRATOR_PWM_PIN, min( (int)(VIBRATOR_STRENGTH*VIBRATOR_MAXIMUM_SIGNAL), VIBRATOR_MAXIMUM_SIGNAL ) ); // Vibrator ON
  delay( VIBRATOR_RECEPTIVE_DURATION );
  digitalWrite( VIBRATOR_PWM_PIN, LOW ); // Vibrator OFF
}


void announceLookahead( void ){
  digitalWrite( LED_PIN, HIGH );
  delay( 2000 );
  digitalWrite( LED_PIN, LOW );
}


void announceRecommendation( pocket_t choice0to5 ){
  while( ( 1 + choice0to5-- ) > 0 ){ // notify player of recommendation: pulse Vibrator x choice1to6
    analogWrite( VIBRATOR_PWM_PIN, min( (int)(VIBRATOR_STRENGTH*VIBRATOR_MAXIMUM_SIGNAL), VIBRATOR_MAXIMUM_SIGNAL ) ); // Vibrator ON
    delay( RECOMMEND_PULSE_DURATION );
    digitalWrite( VIBRATOR_PWM_PIN, LOW ); // Vibrator OFF
    delay( RECOMMEND_PULSE_DELAY );
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
  struct GameState initial = { .distribution = INITIAL_DISTRIBUTION, .p1score = 0, .p2score = 0, .p1turn = 1 };
  delay( RECOMMENDATION_DELAY );
  announceLookahead();
  announceRecommendation( RecommendMove( &initial, LOOKAHEAD_DEPTH-1 ) );
  announcePlayer( 1 );
  
}


/********************************************************************************************************************************/


void loop() {
  
  static struct GameState game = { .distribution = INITIAL_DISTRIBUTION, .p1score = 0, .p2score = 0, .p1turn = 1 };
  
  if( num_presses > 0 ){
    
    for( int i = 0; i < (BUTTON_ACCUMULATION_DELAY/INTERRUPT_RESET_DELAY); i++ ){ // wait for all presses to be recorded, and also ignore "fake" interrupts
      delay( INTERRUPT_RESET_DELAY ); // pause to allow "bounce" to fade
      ignore_button = 0; // enable reception of input
      announceReceptivity(); // gently signal to player that input is enabled
    }
    delay( INTERRUPT_RESET_DELAY ); // without this, user doesn't actually have time to press Button (delay happens at start of loop)

    if( num_presses <= NUM_POCKETS/2 ){

      PerformMove( &game, ( game.p1turn ? num_presses-1 : GetOppositePocketNum( num_presses-1 ) ) ); // convert move# to the absolute version that PerformMove expects
  
      if( game.p1turn ){ // irrelevant of who played the previous turn, if it is user's turn next then recommend a move
        delay( RECOMMENDATION_DELAY );
        announceLookahead();
        announceRecommendation( RecommendMove( &game, LOOKAHEAD_DEPTH-1 ) );
      }
      
    }
    else // if number of presses is illegal/meaningless, interpret as "pass" (besides robustness, also used by user to be Player #2 rather than Player #1)
      game.p1turn = !game.p1turn;
    
    num_presses = 0;
    ignore_button = 0; // if user presses Button at last opportunity then ignore_button == 1 but won't be reset by above for loop
    
    announcePlayer( game.p1turn );
  }
  
}
