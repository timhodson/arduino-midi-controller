# arduino-midi-controller

Code for midi controller based on Ardunio Libraries

## Pot-test-1

This ardunio sketch sends midi messages to an XR18 mixer.
Both mixer and ardunio are connected to a Raspberry Pi which is acting as the hub and routing midi from one to the other.

Some of the midi cmmands directly map to hardware changes, the others are loading of a snapshot which then effectively toggles behaviour.

### Hardware

- Leonardo Pro Micro
- Small breadboard and some jumper wire.
- a potentiometer, 
- a push button 
- and a three position switch.

### Actions

#### Potentiometer

Controls the main LR output fader. Additionally the pot is filtered as the extreme ends of the pot were very noise, so had to limit the range.

#### 3 position Switch

- position up — mute input channel one
- position middle - unmute channel one
- position down — mute all channels
  - On falling pin, mute all channels - uses a snapshot in index 2
  - On rising pin, unmute all channels - uses a snapshot at index 3

#### Push to make switch

- Unmute input channel one