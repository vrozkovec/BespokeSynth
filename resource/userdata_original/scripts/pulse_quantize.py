#quantize a controller button to the next beat, blinking the pad while it is "armed"
#requires a midicontroller named "midicontroller" -- it must have an OUTPUT device set too,
#  not only an input, or the LED feedback (send_note/send_cc) goes nowhere.
#route this script's note output into a "notetopulse" module to get a pulse on the beat.

import midicontroller

m = midicontroller.get("midicontroller")
m.add_script_listener(me.me())   #deliver on_midi() to this script

BUTTON = 36      #control number of the pad to capture (turn on DEBUG to confirm)
INTERVAL = 4     #4 = next beat (quarter), 1 = next bar, 8 = 8th, 16 = 16th
DEBUG = True     #prints every incoming midi message to the script console

#--- LED self-test: uncomment ONE of these, hit run, and watch the pad. ---
#--- if it does NOT light, the problem is the controller/output, not the script. ---
#m.send_note(BUTTON, 127, True)      #for a note pad   (type=0 in the debug line)
#m.send_cc(BUTTON, 127)              #for a cc pad     (type=1 in the debug line)

armed = False
btn_type = 0      #captured from the pad: 0 = note, 1 = cc
btn_channel = -1  #captured from the pad

def on_midi(messageType, control, value, channel):
   global armed, btn_type, btn_channel
   if DEBUG:
      me.output("midi  type=" + str(messageType) + "  ctrl=" + str(control) + "  val=" + str(value) + "  ch=" + str(channel))
   #trigger on ANY message from the pad (works for momentary AND latching/toggle pads)
   if control == BUTTON and not armed:
      btn_type = messageType
      btn_channel = channel
      armed = True
      blink()
      me.schedule_call(bespoke.time_until_subdivision(INTERVAL), "fire()")

def led(level):
   #send the LED back the exact same way the pad talks to us (note vs cc, same channel)
   if btn_type == 1:
      m.send_cc(BUTTON, level, btn_channel)
   else:
      m.send_note(BUTTON, level, True, btn_channel)

def blink():
   if armed:
      led(127 if bespoke.get_step(8) % 2 == 0 else 0)   #8th-note blink
      me.schedule_call(1.0 / 16, "blink()")             #re-evaluate every 16th

def fire():
   global armed
   armed = False
   led(0)                            #LED off
   me.play_note(60, 127, 1.0 / 16)  #-> notetopulse -> downstream pulse
