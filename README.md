# Arduino Esplora Maze Runner Game

[Arduino Project Hub Page]()

## Introduction

This project is a Maze game for Arduino. I developed specifically for the Arduino Esplora, but can be deployed in any Arduino by replacing the Esplora and EsporaTFT libraries by their less generic counterparts.
The mazes are 961 character sequences of 't's (path) and 'f's (wall), which form a 31x31 grid. There should be two, and only two path blocks on the edge of the maze, which will be the Entry and Exit of the maze.
It is possible to load an infinite number of maze files (I provide a zip file with up to 500 randomly generated maps).
The game also keeps a high score (time taken to complete) for each maze.

## Hardware

The only hardware used in this project were:
* Arduino Esplora: https://store.arduino.cc/arduino-esplora

* Ardiono LCD Screen: https://store.arduino.cc/arduino-lcd-screen
* Micro SD card

## Installation

1. Unzip one of the maze pack files in the root of a micro SD card
2. Compile the Sketch with the Arduino IDE and upload it to your Arduino Esplora.

## Usage

### Map selection

[Maze selection screen]()
Use the joystick to select a maze and press the bottom button to start:

### Maze game

[Maze game screen]()
Use the joystick to run through the maze and reach the exit

### Victory screen

[Victory screen]()
Click the bottom button to dismiss and go back to the maze selection screen.


## Feedback?

I appreciate feedback of any kind. Just give a message or post in issues. Thanks!
