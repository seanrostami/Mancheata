# Mancheata

pronounced "mawn-CHEAT-a", a program written in C++ for a wearable Arduino-based device that recommends moves for the popular game Mancala


## AUTHOR ##

[Sean Rostami](https://www.linkedin.com/in/sean-rostami-77255a141/) 


## CONTACT ##

<sean.rostami@gmail.com>


## TABLE OF CONTENTS ##

1) PURPOSE
2) STYLE
3) USAGE
4) SOURCE
5) BINARY
6) HARDWARE


## PURPOSE ##

_Mancheata_ is intended to run on a "wearable computer" and discreetly recommends what moves to perform in the popular game _Mancala_. In other words, it allows you to cheat. It is written in C++ [for an Arduino microcontroller](https://www.arduino.cc/reference/en/), receives updates about the true state of the game from the wearer via a physical button, and transmits its recommendation to the wearer via a vibration motor (see USAGE and IMPLEMENTATION, below, for more details).

Although it is known that Player #1 can always win with perfect play, there is no conceptual understanding for Mancala of what "perfect play" means. In the absence of such a deliberate strategy, Mancheata merely simulates a few turns of play and recommends the first move of the sequence that guarantees a certain balance of points and "material", which is to say the number of stones on the player's own side (for details, see MancalaAnalyzer.hpp).


## STYLE ##

Within the family of games generally referred to as "Mancala", Mancheata assumes that it is the [version most popular in the United States](https://en.wikipedia.org/wiki/Kalah). The main points of this style, properly called _Kalah_, are:

- Each player owns **six** "pits" and one "store". Each pit starts the game with **four** stones.

- A move consists of a player removing all stones from one of his own pits and distributing them in a **counterclockwise** path around the board.

- While distributing into pits, the player also deposits one stone into his own store, but NOT into the opponent's store.

- If the **last** stone of the distribution is deposited into an empty pit owned by the distributing player then he deposites into his store both that stone and all stones in the opposing pit. _This applies even if the opposing pit is empty._

- If the **last** stone of the distribution is deposited into one of the player's own pits then he can/must perform another move. Otherwise, play passes to the opponent.

- The game ends when a player _starts_ a turn and with all his pits empty (most explanations of Mancala are vague about this part). When this happens, the player's opponent deposits all stones from his own pits into his own store. The winner is the player whose store contains the most stones.

![MANCALA](https://github.com/seanrostami/Mancheata/raw/master/MANCALA.JPG "board and initial distribution")


## USAGE ##

_coming soon..._


## SOURCE ##

- mancheata.ino (Arduino-specific code, including ```setup()``` and ```loop()```)

- MancalaBoard.hpp (general-purpose header+source for the state of a Mancala board, independent of hardware and IO)

- MancalaGame.hpp (general-purpose header+source for Mancala's gameplay, independent of hardware and IO)

- MancalaAnalyzer.hpp (header+source for the lookahead algorithm, independent of hardware and IO)


## BINARY ##

- Mancheata20181027.hex (occupies 3838 bytes, according to Arduino IDE)


## HARDWARE ##

My own version uses an [Arduino Nano microcontroller](https://store.arduino.cc/arduino-nano), powered by an ordinary 9V Battery, with a [Button](https://www.sparkfun.com/products/14460) for input and a [Vibrator](https://www.sparkfun.com/products/8449) for output. It is convenient to fasten the Nano and the Battery to a belt or similar via [velcro](https://www.lowes.com/pd/VELCRO-0-75-in-White-Roll-Fastener/3037111).

![PROTOTYPE](https://github.com/seanrostami/Mancheata/raw/master/PROTOTYPE.JPG "prototype")

The Battery is connected to the Nano's Vin/GND via a [standard connector](https://www.sparkfun.com/products/91), which is safe because the Nano regulates Vin.

For convenience, I clipped one pair of pins on the Button and flattened the other two. Those two remaining pins must be connected to a pin capable of INTerrupt (I chose D2) and GND, and the former must be set to ```INPUT_PULLUP``` for two reasons.

The Vibrator's leads must be connected to a pin capable of PWM (I chose D9) and GND. The PWM is needed because the Vibrator's recommended maximum voltage is 3V, and ```analogWrite``` is used to simulate voltages from 0V to 3V using the Nano's 5V supply. The Nano's "standard" GND pin is already occupied by the Button, so I used the GND pin on the ICSP region.

The "lookahead depth", which is the number of turns to simulate before making a recommendation, can be changed at the top of the mancheata.ino file. Obviously, increasing it makes the overall result of the game more certain and more lopsided. However, the time required to perform the lookahead may be unrealistic if the depth is too high (I did not yet determine what is the maximum depth that can be used on a Nano in a real-world game). Worse, too high a depth may cause stack overflow since lookahead is accomplished by multiple-recursion.


_last updated Oct 27, 2018_
