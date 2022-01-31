### Hints to be display for Puzzle 3 including the MQTT-Spec


## Game 1: Antenna
Hint 1: What do you do if a radio does have a bad connection?
MQTT-Message: game/puzzle3/antenna : {"method": "trigger", "state": "active_1"}

## Game 2: Map
Hint 1: The room operator usually spend his holiday at three locations
MQTT-Message: game/puzzle3/map : {"method": "trigger", "state": "active_1"}

Hint 2: These locations are in different time zones
MQTT-Message: game/puzzle3/map : {"method": "trigger", "state": "active_2"}

## Game 3: Touchgame
Hint 1: Touch the letters to build a word.
MQTT-Message: game/puzzle3/touchgame : {"method": "trigger", "state": "active_1"}

Hint 2: Listen carefully.
MQTT-Message: game/puzzle3/touchgame : {"method": "trigger", "state": "active_2"}

Hint 3: Not automatic.
MQTT-Message: game/puzzle3/touchgame : {"method": "trigger", "state": "active_3"}

Hint 4: Manual.
MQTT-Message: game/puzzle3/touchgame : {"method": "trigger", "state": "active_4"}
