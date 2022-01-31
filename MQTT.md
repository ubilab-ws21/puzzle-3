The MQQT-Specification for Puzzle 3:

| Topic | Description | Payload | Reset-Value | Subscriber | Publisher |
| :--:	| :---------: | :------:| :---------: |:--------: | :--------:|
| 3/gamecontrol/antenna| Control the radio: start first puzzle & receive updates on it | on & off, inactive, active & solved | off | radio, operator | radio, operator | 
| 3/gamecontrol/map | Control the radio: start the second puzzle & receive updates on it | on & off, inactive, active & solved | off | operator, radio | radio, operator |
| 3/gamecontrol/touchgame | Control the radio: start the third puzzle & receive updates on it | on & off, inactive, active & solved | off | operator, radio | radio, operator |
| 3/audiocontrol/roomsolved | Control the radio: play final message when the room is solved | on & off | off | radio | operator |

We reduced the amount of MQTT-Topics since the hint-system is controlled by us directly: Therefore it is not required to send all informations to the operator, which are not really helpful in terms of gamecontrol.


MQTT-Topics for the hint-system
| Topic | Description | Payload | Reset-Value | Subscriber | Publisher |
| :--:	| :---------: | :------:| :---------: |:--------: | :--------:|
| game/puzzle3/antenna | Hint-System: Unlock the hint for the first game | active_1 & inactive | inactive | hint-system | radio |
| game/puzzle3/map | Hint-System: Unlock the hints for the second game | active_1, active_2 & inactive | inactive | hint-system | radio |
| game/puzzle3/touchgame | Hint-System: Unlock the hints for the third game | active_1, active_2, active_3, active_4 & inactive | hint-system | radio |



