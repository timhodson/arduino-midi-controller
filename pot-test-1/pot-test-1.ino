#include <Control_Surface.h>

BEGIN_CS_NAMESPACE

/*
 * @brief   A class for momentary buttons and switches that send MIDI events.
 *
 * The button is debounced, and the internal pull-up resistor is enabled.
 *
 * @see     NoteButton
 * @see     Button
 */
class MyCCButton : public MIDIOutputElement {
 public:
  /*
   * @brief   Create a new MyCCButton object on the given pin, with the 
   *          given address and value.
   * 
   * @param   pin
   *          The digital input pin to read from.  
   *          The internal pull-up resistor will be enabled.
   * @param   address
   *          The MIDI address to send to.
   * @param   down_value
   *          The MIDI value to send [0, 127].
   * @param   up_value
   *          The MIDI value to send [0, 127].
      */
  MyCCButton(pin_t pin, const MIDIAddress &address, uint8_t down_value, uint8_t up_value)
    : button(pin), address(address), down_value(down_value), up_value(up_value) {}

 public:
  // Initialize: enable the pull-up resistor for the button
  // This method is called once by `Control_Surface.begin()`.
  void begin() final override { button.begin(); }

  // Update: read the button and send MIDI messages when appropriate.
  // This method is called continuously by `Control_Surface.loop()`.
  void update() final override {
    AH::Button::State state = button.update();               // Read the button
    if (state == AH::Button::Falling) {                      // if pressed
      Control_Surface.sendCC(address, down_value);
    } else if (state == AH::Button::Rising) {                // if released
      Control_Surface.sendCC(address, up_value); 
    }
  }

 private:
  AH::Button button;
  const MIDIAddress address;
  uint8_t down_value;
  uint8_t up_value;
};


/*
 * @brief   A class for momentary buttons and switches that send MIDI events.
 *
 * The button is debounced, and the internal pull-up resistor is enabled.
 *
 * @see     NoteButton
 * @see     Button
 */
class MyPCButton : public MIDIOutputElement {
 public:
  /*
   * @brief   Create a new MyPCButton object on the given pin, with the 
   *          given address and value.
   * 
   * @param   pin
   *          The digital input pin to read from.  
   *          The internal pull-up resistor will be enabled.
   * @param   address
   *          The MIDI address to send to.
   * @param   down_value
   *          The MIDI value to send [0, 127].
   * @param   up_value
   *          The MIDI value to send [0, 127].
   */
  MyPCButton(pin_t pin, const MIDIAddress &down_address, const MIDIAddress &up_address )
    : button(pin), down_address(down_address), up_address(up_address) {}

 public:
  // Initialize: enable the pull-up resistor for the button
  // This method is called once by `Control_Surface.begin()`.
  void begin() final override { button.begin(); }

  // Update: read the button and send MIDI messages when appropriate.
  // This method is called continuously by `Control_Surface.loop()`.
  void update() final override {
    AH::Button::State state = button.update();               // Read the button
    if (state == AH::Button::Falling) {                      // if pressed
      Control_Surface.sendPC(down_address);
    } else if (state == AH::Button::Rising) {                // if released
      Control_Surface.sendPC(up_address); 
    }
  }

 private:
  AH::Button button;
  const MIDIAddress down_address;
  const MIDIAddress up_address;
};

END_CS_NAMESPACE

USBMIDI_Interface usbmidi;
// Create a hardware midi serial output using the TX pin of the Arduino.
HardwareSerialMIDI_Interface serialmidi = {Serial1, MIDI_BAUD};

// Create a MIDI pipe factory to connect the MIDI interfaces to Control Surface
BidirectionalMIDI_PipeFactory<2> pipes;

// Create a new instance of the class `CCPotentiometer`, called `potentiometer`,
// on pin A0, that sends MIDI messages with controller 7 (channel volume)
// on channel 1.
CCPotentiometer potentiometer = {
  A0, {0x1F, CHANNEL_1}
};

// The filtered value read when potentiometer is at the 0% position
constexpr analog_t minimumValue = 255;
// The filtered value read when potentiometer is at the 100% position
constexpr analog_t maximumValue = 9300;

// A mapping function to eliminate the dead zones of the potentiometer:
// Some potentiometers don't output a perfect zero signal when you move them to
// the zero position, they will still output a value of 1 or 2, and the same
// goes for the maximum position.
analog_t mappingFunction(analog_t raw) {
    // make sure that the analog value is between the minimum and maximum
    raw = constrain(raw, minimumValue, maximumValue);
    // map the value from [minimumValue, maximumValue] to [0, 16383]
    return map(raw, minimumValue, maximumValue, 0, 16383);
    // Note: 16383 = 2¹⁴ - 1 (the maximum value that can be represented by
    // a 14-bit unsigned number
}

// Instantiate a MyCCButton object
MyCCButton ptt_button = {
  2,                       // Push button on pin 2
  {0x00, CHANNEL_2}, // Address on Channel
  0x00, // on button down - unmute Audio Input 1
  0x7F // on button up - mute Audio Input 1
};

CCButton mute_strip_1 = {
  4,
  {0x00, CHANNEL_2} // mute Audio Input 1
};

MyPCButton all_mute = {
  3,
  {0x01, CHANNEL_1}, // snapshot 2
  {0x02, CHANNEL_1}  // snapshot 3
};

void setup() {
  // Manually connect the MIDI interfaces to Control Surface
  Control_Surface | pipes | usbmidi;
  Control_Surface | pipes | serialmidi;
  // Add the mapping function to the potentiometer
  potentiometer.map(mappingFunction);
  // Initialize everything
  Control_Surface.begin();
  Serial.begin(115200);
}

void loop() {
  // Update the Control Surface (check whether the potentiometer's
  // input has changed since last time, if so, send the new value over MIDI).
  Control_Surface.loop();
  // Use this to find minimumValue and maximumValue: it prints the raw value
  // of the potentiometer, without the mapping function
//   Serial.println(potentiometer.getRawValue());
}
