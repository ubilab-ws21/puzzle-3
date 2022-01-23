# Puzzle 3

## Contents
- [Puzzle overview](#puzzle-overview)
- [Repo Structure](#repo-structure)
- [Fitting the Puzzle into the story](#fitting-the-puzzle)
- [Puzzle description](#puzzle-description)
	- [Puzzle Part 1](#Puzzle-1)
	- [Puzzle Part 2](#Puzzle-2)
	- [Puzzle Part 3](#Puzzle-3)
- [MQTT Topics/Specification](#MQTT-Spec)
- [Documents](#documents)

## Puzzle overview

The central role of our puzzle plays a "radio".
The radio consists of a touchscreen display, a speaker, a module to play sound from an sd card, three encoders for tuning as well as one for the volume. The devices are all connected to a ESP32 WROOM Development kit. 
Final aim of the puzzle is to retrieve the so-called "green code" which is needed in a combination to restart the final server, i.e. to succeed in playining the Escape Room. 
It is to be discussed if the radio will maybe deliver more information, e.g. hints for the next puzzle (Morse-Code sound to open the server room door). 
The green code will eventually be displayed on the radio screen, once all the radio-related puzzles have been solved. The puzzles to be solved include the correct positioning of the three encoders which results in a change of the radio station.
The new channel also comes with a new radio display screen. Solving a word game now leads to the final green code. 

## Repo Structure

This repository is structured in the following way:
- datasheets contains all interesting datasheets
- designs contains eagle designs
- code contains the code
- notes cotains all notes from lectures and meetings

## Fitting the Puzzle into the story

The radio belongs to the hobby funker and former power distribution worker Ferdi. 
He has been fired a few years ago due to automatisation and digitalization of the distribution center. Ever since he has been fired, the work in the control room had been done autonomously. Ferdi has always predicted that the full autonoumous room can be dangerous in the future because nobody knows how to fix things if something goes wrong. 

## Puzzle Description 

### Puzzle Part 1 

There is an antenna hidden in the room. When the radio turns on, there is nothing but noise to hear. The players need to find the antenna and plug it into a socket one can find at the radio. This will trigger a button. Also, the correct positioning of the antenna makes a difference which is observed by an encoder insider the radio case. When both is fine, there will be no noise anymore and an emergency broadcast can be heard nicely.


### Puzzle Part 2
A world map is placed in the room. There are three colored pins placed on three diffeents countries pinned on the map. Meanwhile, the screen of the radio shows a three slider bars in the same colors, where the values of each bar can be changed with a correspondingly colored encoder.
Both the radio and the world map belong to the former control room worker, i.e. his name is printed on both the radio case and the world map.
The emergency broadcast plays repeatedly. 
The players must look at the pins on the map, extract the difference of the locations to the greenwich mean time (=delta) and turn the correspondingly colored encoders so that the sliding bars have the deltas as values. When this is done, the Emergency broadcast stops, and instead the voice of the former control room worker, a hobby radio operator, arises. Also the screen changed. The puzzle is solved.

### Puzzle Part 3
In the recording, the former control room worker repeatedly says that he has forseen this and that going away from manual control to autonoumously controlled distribution was a bad choice. 
The screen shows a word puzzle, where it is not clear yet how it looks exactly. From the voice from the radio guy it becomes clear how to solve the puzzle.
After the puzzle was solved, the screen changes again and shows the code which is needed for the next puzzle of the next group (code for opening the door via light bulbs)

## MQTT Topics/Specification

Topic: 3/gamecontrol

Description: Control the radio: which puzzle state is currently in play

Payload: Enum(Gamecontrol)

Reset-Value: Idle

Enum Values:

- Idle: Radio puzzle is waiting in Idle mode to be started
	Operator: set the third puzzle in idle mode
	radio: receive to stay in idle mode

- antenna: antenna game is started
	Operator: start the first part of the puzzle
	radio: receive to start the puzzle with antenna game
- antennaFinished: antenna game is finished
	Operator: receive updates on the gamestatus
	radio: update once the game is finished

- map: map game is played
	Operator: start the second part of the puzzle
	radio: receive to start the second part of the puzzle
- mapFinished: map game is finished
	Operator: receive updates on the gamestatus
	radio: update once the game is finished

- touchgame: touchgame is played
	Operator: start the third part of the puzzle
	radio: receive to start the third part of the puzzle
- touchgameFinished: touchgame is finished
	Operator: receive updates on the game status
	radio: update once the game is finished

- finished: puzzle 3 is solved
	Operator: set the third puzzle in finished mode
	radio: receive to get in finished mode

Topic: 3/antenna/plugStatus

Description: gives information about the antenna

Payload: boolean

Reset-Value: false
	Radio: update if antenna is plugged in
	Operator: receive updates on the antenna
	
Topic: 3/antenna/orientation

Description: gives information about the antenna orientation, should the operator know about the randomisation of the required postion?

Payload: int[0-23] or boolean
	Radio: update if antenna is set up in the correct direction
	Operator: receive updates on the antenna orientation

	
Topic: 3/map/knob1

Description: Control/Check the first knob

Playload: boolean

Reset-Value: false
	Radio: update if the the first knob/color reaches/leaves the correct postion
	Operator: receive updates on the first knob

Topic: 3/map/knob2

Description: Control/Check the second knob

Playload: boolean

Reset-Value: false
	Radio: update if the the second knob/color reaches/leaves the correct postion
	Operator: receive updates on the second knob

Topic: 3/map/knob3

Description: Control/Check the third knob

Playload: boolean

Reset-Value: false
	Radio: update if the the third knob/color reaches/leaves the correct postion
	Operator: receive updates on the third knob

Topic: 3/touchgame

Description: needs some discussion what should be updated to the operator


## Documents
The datasheets can be found in the datasheets directory

## Notes
The board with the ICs, placed on the bottom of the radio case inside, should be 9cm x 13cm long.
Software game: We only have one rectangle. By placing the blue rectangle on a letter and touching it, the rectangle and the letter will go to the next free location in the password space.


Next Tasks - 23.01
- Alles zusammen bauen (außer Platine)
- Teile Anmalen 
- Encoder an Plexiglas festmachen (Verkabelung dahin) 
- Driver Board an case anbringen
- Landkarte in Auftrag geben 
- Displayhousing fix montieren
- DisplayDriver Anbindung. (Pins Umlöten, Anbindung konstruieren)
- plantine planen und Grundform schneiden
- geteilte pull-ups und Wiederstände testen.
- Audios neu aufnehmen
