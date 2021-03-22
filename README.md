# EasyEi8ht

This user manual has been updated for software version 0.90.100
 
Mode 1: R/P
Record/Play
Hitting any key will enter that trigger into the pattern
Holding the shift key and pressing any key toggles mute for that channel. The pattern is stored unless:
Hitting the key of a muted channel erases the pattern and lets you enter a new pattern
Default behavior is one-shot, meaning holding a key down only triggers once. This behavior can be changed in configure mode, but defaults back to one-shot when powered down
 
Mode 2: FS
Freestyle
There’s four freestyle modes, indicated by which key’s LED is dark.
FS Mode 1: hitting a key rolls triggers for that channel. They aren’t recorded. Holding shift mutes all tracks besides the track or tracks you are active playing
FS Mode 2: hitting a key triggers that channel once. Holding shift mutes all tracks besides what you’re playing.
FS Mode 3: this is an unquantized mode. Keys you hit will instantly send a trigger to that channel. This mode will work without any clock running, internal or external. The module becomes a trigger generator, a drum pad of sorts.
FL Mode 4: this is a breakbeat/glitch mode. The keys play one, two, three, four, seven or eight, or random, pattern steps on repeat. The pattern stays on track in the background, so when you release the key your beat will be in the correct spot. Holding the shift key in this mode will create 32nd beat fills, letting you create truly monster glitch fills in your drum patterns.
 
 
Mode 3: CNF
Configuration
  Key	| Shift? |	Function                                                   
 --- | -------| ------------------------------------------------------------
 1   | no     | n/a                                                         
 1   | yes    |Sets the sequence length shorter, to a minimum of 16 beats   
2 | no | Cycles through the four freestyle modes
2 | yes | Toggles Record/Play mode one-shot or roll behavior
3 | no | n/a
3 | yes | Sets the sequence length longer, to a maximum of 256 beats
4 | no | Adds one millisecond to the length of a quarter note, slowing the BPM
4 | yes | Nudges the pattern 5ms earlier
5 | no | Subtracts one millisecond from the quarter note, speeding up the BPM
5 | yes | Nudges the pattern 5ms later
6 | no |Swing value change
6|yes|n/a
7|no|n/a
7|yes|Toggles between external and internal clock
8|no|Swing value changes the other way
8|yes|n/a
1, 3, and 7|yes|Toggles EVENT TIMER mode (very unreliable mode)

 
 
While the module is in configuration mode, all but one of the LEDs will light dimly. The LED that isn’t lit shows which freestyle mode is active.
 
When the switch is in CNF mode, tapping the shift key allows you to tap a steady rhythm into the module and it will average the taps and set a tempo.
 
Tapping the shift key in a tempo first establishes a few taps, then zeroes out part of the average that's already been input, and then up to 20 taps get averaged. Fewer taps is okay, just not as precise. More taps roll over. One missed tap or accidental switch-bounce problems get ignored. A single tap after about 1.5 seconds will snap the pattern back when it drifts, BUT there's no way to know if your drum pattern will be as-expected when doing this. The module doesn't know if it's drifting too fast and needs to skip the next step or going too slow and needs to delay the next step. It's better to use the nudge-and-tweak system to get your pattern matching something external.


EVENT TIMER mode

Once you get in this mode, the module will wait for you to press a key. As soon as you do, a timer starts, and any subsequent keypresses get recorded into a new sequence. Pressing the shift key sets the loop point of the sequence.
When the loop is playing, you can add new trigger events.
Holding the shift key while pressing a key mutes that channel. Repeating that unmutes that channel. Recording a new key press while a channel is muted deletes that channel’s pattern.
While in event timer mode, switching to CNF mode and holding the shift key for 1.5 seconds blanks the pattern and resets the loop length.
Once you input 400 trigger events (including muted or deleted trigger events) the module will misbehave, refusing to accept new triggers or deleting the first triggers entered, I haven’t tested that yet.

Be aware that the event trigger mode is unreliable — recorded triggers don’t always play, keypresses don’t always register — but it’s fun! 

