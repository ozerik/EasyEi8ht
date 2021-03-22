/*             EASYEI8HT Trigger Sequencer
                Modular for the Masses
                Juanito Moore, February 2021
        This is a performance trigger sequencer conceived and
    designed from the beinning as an intuitive, instantly usable
    trigger sequencer.

    See user manual for how to use the module.

        This code is documented to help you select your preferred
    behavior, specifically how long you want the default sequence,
    if the keys should produce a trigger for every clock while
    pressed or just a single trigger per key press, trigger length,
    default freestyle mode, whether it should be self-clocked or
    accept an external clock by default, and default self-clocked BPM.

        All these can be changed using the "configuration mode" of
    the module, but reset to the defaults when powered down. See the
    user manual for how to do that.EVENT TIMER MODE

    This trigger sequencer can be an event timer. In this mode, you've got a
    loop that's 32.764 seconds long. You can fill it with however many trigger
    events you like, as long as it's less than 400, and the trigger events
    will play back, with a resolution of 4 milliseconds.

    Get to this mode by setting the switch to CONFIG, and pressing SHIFT and
    buttons 1, 3, and 7, which is like the Heavy Metal Horns hand sign, right?
    Easy way to remember.


    Version history:

    0.91.100 -- added EVENT TIMER mode

    0.90.01 -- fixed behavior of the external clock and reset signal
    monitoring. It used to see long clock HIGH signals as many clocks

    0.90.00 -- first time I though of doing a revision history. Work
    in progress




*/



unsigned long tempo;
unsigned long flash;
unsigned long debounce;
unsigned long debounce2;
unsigned long sixteenth;
unsigned long newTap;
unsigned long oldTap;
unsigned long pressed;
word tapAvg[33];
word glitch[9];
unsigned long temporary;
word elapsed;


unsigned long quarterNote = 500; // sets the default internal tempo, quarter notes, in milliseconds. 500 = 120BPM
word sixteenthNote;
int swingValue;
word sequence[400];
byte played[400];



byte recPins;
byte oldRecPins;
byte tempRecPins;
byte oldTempRecPins;
byte playPins;
byte switchPosition;
byte seqNum = 3;              // sets the default sequence number, change this ONLY when...
word seqLength;
word seqStep;
word oldStep;
byte triggered;
byte mute = B11111111;
byte extTap = 1;              // = 0 means external clock, = 1 means internal clock (10 or 11 means event timer mode)
byte rollOneshot = 1;         // = 0 means kepresses roll, = 1 means kepresses are one-shot
byte FSmode = 0;              // selects the default freestyle mode
const byte triggerLength = 8;  // sets how long triggers go HIGH. 10 is probably the longest you want to do, 2 might be too short.
byte resetted;
word USreset;
byte tapped;
int tapNum = -9;
int oldTapNum;
byte UQFS = 10;
int glitchCounter;
byte pingPong;

int TV;
int TV2;




void setup() {
  SLselect();

  pinMode(0, INPUT_PULLUP);         //PIND  B0000000x (breaks when Serial is used)
  pinMode(1, OUTPUT);               //PORTD B000000x0 (breaks when Serial is used)

  pinMode(2,  INPUT_PULLUP);        //PIND B00000x00   button 1
  pinMode(3,  INPUT_PULLUP);        //PIND B0000x000          2
  pinMode(4,  INPUT_PULLUP);        //PIND B000x0000          3
  pinMode(5,  INPUT_PULLUP);        //PIND B00x00000          4
  pinMode(6,  INPUT_PULLUP);        //PIND B0x000000          5
  pinMode(7,  INPUT_PULLUP);        //PIND Bx0000000          6
  pinMode(8,  INPUT_PULLUP);        //PINB B0000000x          7
  pinMode(9,  INPUT_PULLUP);        //PINB B000000x0          8


  pinMode(10, OUTPUT);              //PORTB B00000x00  output 1
  pinMode(11, OUTPUT);              //PORTB B0000x000         2
  pinMode(12, OUTPUT);              //PORTB B000x0000         3
  pinMode(13, OUTPUT);              //PORTB B00x00000         4
  pinMode(A0, OUTPUT);              //PORTC B0000000x         5
  pinMode(A1, OUTPUT);              //PORTC B000000x0         6
  pinMode(A2, OUTPUT);              //PORTC B00000x00         7
  pinMode(A3, OUTPUT);              //PORTC B0000x000         8

  pinMode(A4, INPUT_PULLUP);        //PINC  B000x0000   clock trigger in
  pinMode(A5, OUTPUT);              //PORTC B00x00000   flashy light

  Serial.begin(115200);

}





void loop() {
  Serial.print(recPins);
  Serial.print(F(" recPins    "));
  Serial.print(switchPosition);
  Serial.print(F(" switchPosition     "));
  Serial.println(extTap);

  { // reset detector
    if (((PIND & B00000001) == 0) && (resetted == 0)) {
      resetted = 1;
      seqStep = 0;
    }
    if ((PIND & B00000001) != 0) resetted = 0;
  }

  { // pin detector
    recPins = PIND >> 2;
    recPins |= ((PINB << 6) & B11000000);
    recPins = ~recPins;
    if (recPins > oldRecPins) {
      pressed = millis();
      tempRecPins = recPins;
    }
  }

  { // shift button detect
    if ((analogRead(A7) < 100) && tapped == 0) {
      tapped = 1;             // the tapped variable is always 1 when the shift key is pressed
      debounce = millis();
      onShiftTap();
    }

    if (tapped == 1) {
      if ((analogRead(A7) > 900) && millis() - debounce > 25) {
        tapped = 2;
        onShiftRelease();
        debounce = millis();
      }
      if (switchPosition == 3) {
        if (millis() - debounce > 1500) {
          if (extTap > 19) extTap -= 10;
          if (extTap > 14) extTap -= 5;
        }
      }
    }
    if ((millis() - debounce > 25) && tapped == 2) {
      tapped = 0;
    }
  }

  { // mode switch detect
    int sP = analogRead(A6);
    switchPosition = 1;
    if (sP < 900) switchPosition = 2;
    if (sP < 100) switchPosition = 3;
    if (switchPosition < 3) {
      DDRB  |= ~B11000011;
      DDRC  |= ~B11110000;
    }
  }



  { // EVENT TIMER
    if (extTap > 9) {
      if (switchPosition == 1) {
        if ((extTap == 10) || (extTap == 11)) {
          for (int i = 0; i < 400; i++) {
            sequence[i] = 0;
            played[i] = 0;
          }
          USreset = 65500;
          extTap += 5;
          seqLength = 0;
        }

        if ((extTap == 15) || (extTap == 16)) {
          if ((recPins > oldRecPins) || (resetted == 1)) {
            resetted = 2;
            tempo = (word)millis();
            extTap += 5;
          }
        }
        if (tapped == 1) {
          if (USreset == 65500) {
            USreset = elapsed;
          }
        }
      }
      if (extTap > 19) elapsed = (word)millis() - tempo;
      if ((elapsed > USreset) || (resetted == 1)) { // restarts loop
        resetted = 2;
        tempo = (word)millis();
        PORTC |= B00100000;
        flash = millis();
        for (int i = 0; i < 400; i++) {
          bitWrite(played[i], 4, 0);
        }
      }
      { // writes current press into variables
        if (recPins > oldRecPins) {
          byte OB = recPins ^ oldRecPins;
          byte channel;
          if (OB > 0)   channel = 0;
          if (OB > 1)   channel = 1;
          if (OB > 3)   channel = 2;
          if (OB > 7)   channel = 3;
          if (OB > 15)  channel = 4;
          if (OB > 31)  channel = 5;
          if (OB > 63)  channel = 6;
          if (OB > 127) channel = 7;
          if (tapped == 1) {
            for (int i = seqLength; i >= 0; i--) {
              if (channel == (played[i] & B00000111)) {
                played[i] ^= B00100000;
                played[i] &= B11111000;
                played[i] |= channel;
              }
            }
          }
          if (tapped == 0) {
            for (int i = 0; i < 400; i++) {   // erases channel on muted channel new key press
              if ((channel == (played[i] & B00000111)) && (bitRead(played[i], 5) == 1)) sequence[i] = 65535;
              if ((oldTapNum == 0) && (i == seqLength)) i = 400;
            }
            seqLength++;
            sequence[seqLength] = elapsed;
            played[seqLength] = channel;
          }
        }
      }



      { // playback (code repeats to be faster)
        seqStep++;
        if (seqStep > seqLength) seqStep = 1;
        if ((elapsed > (sequence[seqStep])) && (bitRead(played[seqStep], 5) == 0) && (bitRead(played[seqStep], 4)) == 0) {
          playPins = 0;
          bitWrite(playPins, played[seqStep], 1);
          bitWrite(played[seqStep], 4, 1);
          flash = millis();
        }
        PORTB |= (playPins << 2) & B00111100;
        PORTC |= (playPins >> 4) & B00001111;
        seqStep++;
        if (seqStep > seqLength) seqStep = 1;
        if ((elapsed > (sequence[seqStep])) && (bitRead(played[seqStep], 5) == 0) && (bitRead(played[seqStep], 4)) == 0) {
          playPins = 0;
          bitWrite(playPins, played[seqStep], 1);
          bitWrite(played[seqStep], 4, 1);
          flash = millis();
        }
        PORTB |= (playPins << 2) & B00111100;
        PORTC |= (playPins >> 4) & B00001111;
        seqStep++;
        if (seqStep > seqLength) seqStep = 1;
        if ((elapsed > (sequence[seqStep])) && (bitRead(played[seqStep], 5) == 0) && (bitRead(played[seqStep], 4)) == 0) {
          playPins = 0;
          bitWrite(playPins, played[seqStep], 1);
          bitWrite(played[seqStep], 4, 1);
          flash = millis();
        }
        PORTB |= (playPins << 2) & B00111100;
        PORTC |= (playPins >> 4) & B00001111;
        seqStep++;
        if (seqStep > seqLength) seqStep = 1;
        if ((elapsed > (sequence[seqStep])) && (bitRead(played[seqStep], 5) == 0) && (bitRead(played[seqStep], 4)) == 0) {
          playPins = 0;
          bitWrite(playPins, played[seqStep], 1);
          bitWrite(played[seqStep], 4, 1);
          flash = millis();
        }
        PORTB |= (playPins << 2) & B00111100;
        PORTC |= (playPins >> 4) & B00001111;
      }

      if (millis() - flash > triggerLength) {
        PORTB &= B11000011;
        PORTC &= B11010000;
        playPins = 0;
      }
    }
  }
  { // REGULAR MODE
    if (extTap < 10) {

      { // self-clocked playback, also advances the seqStep variable
        if (extTap == 1) {
          sixteenthNote = quarterNote / 4;
          if (seqStep >= seqLength) {
            seqStep = 0;
          }
          if (millis() - tempo > quarterNote) {
            seqStep++;
            flash = millis();
            tempo = millis();
            if ((seqStep & B00000001) == 0) {
              sixteenth = (millis() + swingValue);
            } else {
              sixteenth = (millis() - swingValue);
            }
            DDRC  |= B00100000;
            PORTC |= B00100000;   //flashy LED light ON!
          }

          if (millis() - sixteenthNote > sixteenth) {
            seqStep++;
            flash = millis();
            if ((seqStep & B00000001) == 0) {
              sixteenth = (millis() + swingValue);
            } else {
              sixteenth = (millis() - swingValue);
            }
          }
        }
      } // end of self-clocked playback

      { // externally clocked mode
        if (extTap == 0) {
          if (seqStep >= seqLength) {
            seqStep = 0;
          }
          //      if ((triggered == 0) && (analogRead(A4) < 1000)) {
          if ((triggered == 0) && (bitRead(PINC, 4) == 0)) {
            oldTap = tempo;
            tempo = millis();
            sixteenthNote = tempo - oldTap;
            seqStep++;
            triggered = 1;
            if (swingValue == 0) {
              flash = millis();
              if ((seqStep & B00000011) == 0) {
                PORTC |= B00100000;   //flashy LED light ON!
              }
            }
            if (swingValue < 0) {
              if (bitRead(seqStep, 0) == 0) {
                flash = millis();
                if ((seqStep & B00000011) == 0) {
                  PORTC |= B00100000;   //flashy LED light ON!
                }
              } else triggered = 5;
            }
            if (swingValue > 0) {
              //          seqStep++;
              if (bitRead(seqStep, 0) == 1) {
                flash = millis();
                if ((seqStep & B00000011) == 0) {
                  PORTC |= B00100000;   //flashy LED light ON!
                }
              } else triggered = 5;
            }
          }
          if (triggered == 5) {
            if (millis() - tempo > abs(swingValue)) {
              triggered = 3;
              flash = millis();
              if ((seqStep & B00000011) == 0) {
                PORTC |= B00100000;   //flashy LED light ON!
              }

            }
          }
        }
      } // end of external clock mode


      { // records trigger events into sequence
        if (switchPosition == 1) {


          if (tapped == 0) {                    // shift key not pressed -- enter triggers into the pattern

            if (millis() - flash < (triggerLength - 1)) {
              if (rollOneshot == 0) {           // roll, hits every clock
                sequence[seqStep] |= recPins;
              }
              if (millis() - pressed < 100) {   // oneshot, doesn't roll
                sequence[seqStep] |= tempRecPins;
                pressed = 0;
              }
            }

            { // this bit erases tracks when they're muted and a new key is pressed
              byte erase = recPins & ~mute;
              mute = mute | erase;
              for (int j = 0; j < 256; j++) {
                sequence[j] &= ~erase;
              }
            }
          } else { // shift key is held, this mutes keys
            //        if (recPins > 0) {
            if (recPins > oldRecPins) {
              mute = mute ^ recPins ^ oldRecPins;
            }
            //        }
          }
        }
        playPins = sequence[seqStep] & mute;
      }


      { // Freestyle mode THIS IS SO MUCH FUN
        if (switchPosition == 2) {
          if (tapped != 0) playPins = 0;
          if (FSmode == 0) { // roll mode
            playPins |= recPins;
          }
          if (FSmode == 1) {      // oneshot mode
            if ((pressed > 0) && (millis() - flash < 4)) {
              playPins |= recPins;
              pressed = 0;
            }
          }

          { // unquantized freestyle mode
            if (FSmode == 2) {
              if (pressed > 0) {
                playPins = 0;
                pressed = 0;
                debounce2 = millis();
                flash = millis();
                UQFS = 1;
              }
            }
          }

          { // stutter glitch mode
            if (FSmode == 3) {

              if (recPins > 0) {
                playPins = 0;
              }
              { // key 1, just one step, 16th notes, shift-1, 32nd notes
                if (bitRead(recPins, 0) == 1) {
                  if (millis() - flash < 4) {
                    if (glitch[1] == 0) {
                      glitch[1] = seqStep + 1;
                    }
                  }
                  if ((millis() - temporary > (sixteenthNote / 2)) && (tapped == 1)) {
                    temporary = millis();
                    flash = millis();
                    playPins = sequence[glitch[1]];
                  }
                  if ((millis() - temporary > (sixteenthNote) && (tapped == 0))) {
                    temporary = millis();
                    flash = millis();
                    playPins = sequence[glitch[1]];
                  }
                } else glitch[1] = 0;
              }


              { // key 2, plays the last two triggers, shift-2 does 32nd notes
                if (bitRead(recPins, 1) == 1) {
                  if (glitch[2] == 0) {
                    glitch[2] = seqStep + 1;
                    glitchCounter = 3;
                  }
                  if ((oldStep != seqStep) || (millis() - temporary < 1)) {
                    temporary++;
                    glitchCounter++;
                    if (glitchCounter > 1) glitchCounter = 0;
                  }
                  if (tapped == 0) {
                    byte gcc = glitch[2] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength; // glitch count correction
                    playPins = sequence[gcc];
                  }
                  if (millis() - temporary > (sixteenthNote / 2) && (tapped == 1)) {
                    temporary = millis();
                    flash = millis();
                    if (glitchCounter > 1) glitchCounter = 0;
                    byte gcc = glitch[2] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                  }
                } else glitch[2] = 0;
              }

              { // key 3, plays three steps in 16th notes, shift-3 does 32nd notes
                if (bitRead(recPins, 2) == 1) {
                  if (glitch[3] == 0) {
                    glitch[3] = seqStep + 1;
                    glitchCounter = 3;
                  }
                  if ((tapped == 1) && ((oldStep != seqStep) || (millis() - temporary < 1))) {
                    temporary++;
                  }
                  if (glitchCounter > 2) glitchCounter = 0;

                  if (tapped == 0) {
                    if (oldStep != seqStep) glitchCounter++;
                    if (glitchCounter > 2) glitchCounter = 0;
                    byte gcc = glitch[3] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength; // glitch count correction
                    playPins = sequence[gcc];
                  }
                  if (millis() - temporary > (sixteenthNote / 2) && (tapped == 1)) {
                    temporary = millis();
                    flash = millis();
                    if (glitchCounter > 2) glitchCounter = 0;
                    byte gcc = glitch[3] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    glitchCounter++;
                  }
                } else glitch[3] = 0;
              }

              { // key 4, plays last three steps ping-pong mode, 16th or 32nds
                if (bitRead(recPins, 3) == 1) {
                  if (glitch[4] == 0) {
                    glitch[4] = seqStep + 1;
                    glitchCounter = 0;
                  }
                  if ((tapped == 1) && ((oldStep != seqStep) || (millis() - temporary < 1))) {
                    temporary++;
                    if (glitchCounter < 1) pingPong = 1;
                    if (glitchCounter > 1) pingPong = 0;
                    if (pingPong) {
                      glitchCounter++;
                    } else glitchCounter--;
                  }
                  if (tapped == 0) {
                    if (oldStep != seqStep) {
                      flash = millis();
                      if (glitchCounter < 1) pingPong = 1;
                      if (glitchCounter > 1) pingPong = 0;
                      if (pingPong) {
                        glitchCounter++;
                      } else glitchCounter--;
                    }
                    byte gcc = glitch[4] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength; // glitch count correction
                    playPins = sequence[gcc];
                  }
                  if ((tapped == 1) && (millis() - temporary > (sixteenthNote / 2))) {
                    temporary = millis();
                    flash = millis();
                    byte gcc = glitch[4] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength; // glitch count correction
                    playPins = sequence[gcc];
                  }
                } else glitch[4] = 0;
              }

              { // key 5, plays next four steps

                if (bitRead(recPins, 4) == 1) {
                  if (glitch[5] == 0) {
                    glitch[5] = seqStep + 1;
                    glitchCounter = 0;
                  }
                  if (tapped == 0) {
                    byte gcc = glitch[5] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    if (oldStep != seqStep) glitchCounter++;
                  }
                  if ((tapped == 1) && (millis() - temporary > (sixteenthNote / 2))) {
                    temporary = millis();
                    flash = millis();
                    byte gcc = glitch[5] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    glitchCounter++;
                  }
                  if (glitchCounter > 3) glitchCounter = 0;
                } else glitch[5] = 0;
              }

              { // key 6, plays next five steps ping-pong

                if (bitRead(recPins, 5) == 1) {
                  if (glitch[6] == 0) {
                    glitch[6] = seqStep + 1;
                    glitchCounter = 0;
                  }
                  if (tapped == 0) {
                    byte gcc = glitch[6] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    if (oldStep != seqStep) {
                      if (glitchCounter < 1) pingPong = 1;
                      if (glitchCounter > 4) pingPong = 0;
                      if (pingPong) {
                        glitchCounter++;
                      } else glitchCounter --;
                    }
                  }
                  if ((tapped == 1) && (millis() - temporary > (sixteenthNote / 2))) {
                    temporary = millis();
                    flash = millis();
                    byte gcc = glitch[6] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    if (glitchCounter < 1) pingPong = 1;
                    if (glitchCounter > 4) pingPong = 0;
                    if (pingPong) {
                      glitchCounter++;
                    } else glitchCounter --;
                  }
                } else glitch[6] = 0;
              }

              { // key 7, plays next eight steps

                if (bitRead(recPins, 6) == 1) {
                  if (glitch[7] == 0) {
                    glitch[7] = seqStep + 1;
                    glitchCounter = 0;
                  }
                  if (tapped == 0) {
                    byte gcc = glitch[7] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    if (oldStep != seqStep) glitchCounter++;
                  }
                  if ((tapped == 1) && (millis() - temporary > (sixteenthNote / 2))) {
                    temporary = millis();
                    flash = millis();
                    byte gcc = glitch[7] + glitchCounter;
                    if (gcc > seqLength) gcc = gcc - seqLength;
                    playPins = sequence[gcc];
                    glitchCounter++;
                  }
                  if (glitchCounter > 6) glitchCounter = 0;
                } else glitch[7] = 0;
              }

              { // key 8, creates 8 random numbers in the sequence, and plays those sequence steps

                if (bitRead(recPins, 7) == 1) {
                  if (glitch[8] == 0) {
                    for (byte i = 1; i < 9; i++) {
                      glitch[i] = random(1, seqLength);
                    }
                  }
                  if (tapped == 0) {
                    playPins = sequence[glitch[glitchCounter++]];
                    if (oldStep != seqStep) playPins = sequence[glitch[glitchCounter++]];
                  }
                  if ((tapped == 1) && (millis() - temporary > (sixteenthNote / 2))) {
                    temporary = millis();
                    flash = millis();
                    playPins = sequence[glitch[glitchCounter++]];
                  }
                  if (glitchCounter > 8) glitchCounter = 1;
                }  else glitch[8] = 0;
              }
            }
          }
        }
      }


      { // Plays all the triggers in playPins variable and turns them off too
        if (millis() - flash < 4) {     // plays all the triggers in playPins
          debounce2 = millis();
          PORTB |= (playPins << 2) & B00111100;
          DDRB  |= B00111100;
          PORTC |= (playPins >> 4) & B00001111;
          DDRC  |= B00001111;
          PORTD |= B00000010;       //here's the swing output
        }

        if ((millis() - flash > triggerLength) && UQFS > 5) {
          DDRB &=  B11000011;
          PORTB &= B11000011;
          DDRC &=  B11110000;
          PORTC &= B11110000;
          PORTD &= B11111101;
        }
        if (UQFS == 1) {
          PORTB |= ((recPins ^ oldRecPins) << 2) & B00111100;
          DDRB  |= B00111100;
          PORTC |= ((recPins ^ oldRecPins) >> 4) & B00001111;
          DDRC  |= B00001111;
          UQFS = 10;
        }
        if (millis() - flash > 15) {
          if (bitRead(PINC, 4) == 1) triggered = 0;
          if (bitRead(PINC, 0) == 1) resetted = 0;
        }
        if (millis() - flash > 40) {
          PORTC &= B11011111;
          oldTempRecPins = tempRecPins;
        }
      }
    }
  } // end of REGULAR MODE


  { // Configuration Mode
    if (switchPosition == 3) {

      { // this turns all the trigger LEDs on dimly to communicate "CONFIG MODE"
        DDRB  &= (playPins << 2) & ~B11000011;
        DDRC  &= (playPins >> 4) & ~B11110000;
        PORTB |= B00111100;
        bitWrite(PORTB, (FSmode + 2), 0); // this lights up the freestyle mode
        PORTC |= B00001111;
        DDRC  |= B00100000;
      }

      if (recPins > oldRecPins) {

        byte shift = 0;
        if (analogRead(A7) < 100) {
          shift = 1;
        }

        if (((recPins & B00000001) != 0) && shift == 1) {   // key 1, shorter sequence
          seqNum--;
          if (seqNum == 0) {
            seqNum = 1;
          }
        }

        if ((recPins & B00000010) != 0) {   // key 2, cycles through freestyle modes
          if (shift == 0) {
            FSmode++;
            if (FSmode == 4) {
              FSmode = 0;
            }
          } else {                          // shift-key 2, togggle roll or one-shot
            rollOneshot++;
            if (rollOneshot == 2) rollOneshot = 0;
          }
        }

        if (((recPins & B00000100) != 0) && shift == 1) {   // key 3, longer sequence
          seqNum++;
          if (seqNum == 8) {
            seqNum = 7;
          }
        }

        if ((recPins & B00001000) != 0) {   // key 4, tweaks tempo slower for tap mode
          if (shift == 1) {                 // shift 4 nudges pattern 5ms earlier
            tempo = tempo - 5;
          } else quarterNote++;
        }

        if ((recPins & B00010000) != 0) {   // key 5, tweaks tempo faster for tap mode
          if (shift == 1) {
            tempo = tempo + 5;              // shift 5 nudges pattern 5ms later
          } else quarterNote--;
        }

        if ((recPins & B00100000) != 0) {   // key 6, swing value change
          swingValue = swingValue + 2;
        }

        if (((recPins & B01000000) != 0) && (shift == 1) && (extTap < 2)) {   // shift-key 7, toggles tap and external trigger mode
          extTap = !extTap;
        }

        if ((recPins & B10000000) != 0) {   // key 8, swing value change
          swingValue = swingValue - 2;
        }

        if (recPins == B10100000) {         // keys 6 and 8, reset swing value
          swingValue = 0;
        }

        if ((recPins == B01000101) && (shift == 1)) {         // shift key plus keys 1, 3, and 7... toggles EVENT TIMER mode
          if (extTap > 9) {
            for (int i = 0; i < 400; i++) {
              sequence[i] = 0;
            }
            extTap -= 20;
          } else {
            extTap += 10;
          }

        }
        if (extTap < 10) SLselect();
      }
      if (millis() - flash > triggerLength) {
        DDRB  &= B11000011; //these two lines mean all LEDs become INPUT
        DDRC  &= B11110000;
      }
    } else {
      if (millis() - flash > triggerLength) {
        PORTB &= B11000011;
        PORTC &= B11110000;
      }
      DDRB  |= B00111100; //these two lines mean all LEDs become OUTPUTs
      DDRC  |= B00001111;
    }
  } // end of Configuration Mode




  oldRecPins = recPins;
  oldStep = seqStep;
}





void SLselect () {

  switch (seqNum) {                   // selects the sequence length
    case 1:
      seqLength = 16;
      break;
    case 2:
      seqLength = 24;
      break;
    case 3:
      seqLength = 32;
      break;
    case 4:
      seqLength = 48;
      break;
    case 5:
      seqLength = 64;
      break;
    case 6:
      seqLength = 128;
      break;
    case 7:
      seqLength = 256;
  }
}














void onShiftRelease() {
  { // this  bit jerks the pattern back to whenever you tapped the button UNLESS
    // you're holding it for a configuration thing
    if ((millis() - newTap < 200) && (tapNum < 0)) {
      tempo = newTap;
    }
  }
}


void onShiftTap() {  // happens once when shift key is pressed

  { // tap tempo mode
    if (switchPosition == 3) {
      { // TAP TEMP DETECTION HERE
        newTap = millis();
        if (tapNum == -9) {
          oldTap = millis();
        }
        if ((tapNum > -9) && (tapNum < 1)) {
          //        newTap = millis();
          tapAvg[tapNum + 9] = newTap - oldTap;
          if (millis() - oldTap > 1200) {
            tapNum = -6;
          }
          if (tapNum == -2) {
            for (byte i = 11; i != 21; i++) {
              tapAvg[i] = 0;
            }
          }
        }
        if (tapNum > 0) {
          //        newTap = millis();
          tapAvg[tapNum] = newTap - oldTap;
          average();
          if (millis() - oldTap > 2500) {   // two and a half seconds with no taps
            tapNum = -6;                    // restarts the tap tempo calculations
          }
        }
        if (tapNum == 21) {
          tapNum = 0;
        }
        oldTap = newTap;
        tapNum++;
      }
    }
  }
}



void average() {
  quarterNote = 0;
  byte count = 0;
  for (byte i = 1; i < 21; i++) {
    if ((tapAvg[i] > 100) && (tapAvg[i] < 1100)) {
      quarterNote = quarterNote + tapAvg[i];
      count++;
    } else {
      i++;
    }
  }
  quarterNote = quarterNote / count;
  sixteenthNote = quarterNote / 4;
}
