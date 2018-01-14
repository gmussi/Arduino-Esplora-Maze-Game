/**********************************************************************************
   This is a Maze Game I developed as a learning project for the Arduino
   platform. This code is developed specifically for the Arduino Esplora with LCD,
   but could be easily adapted to any other Arduino, by replacing the calls to the
   "Esplora" and "EsploraTFT" libraries.
   The maze are stored and loaded from an SD card and consist of a 1024 char sequence
   of 'f' (wall) and 't' (path) which form a 64x64 maze. At any given time, only a 32x32 
   part of the maze is shown. When the player reaches the edge of the screen, the screen scrolls
   to the next / previous part of the maze. When the player reaches its end, a victory screen is 
   shown with the time taken by the player.
   By pressing the top button, an overview with the entire maze is shown.
   
   Author: Guilherme Mussi <https://github.com/gmussi>

   Github Project Page: https://github.com/gmussi/Arduino-Esplora-Maze-Game
   Arduino Project Hub:

 **********************************************************************************

   INSTRUCTIONS

   To run the game, you will need the following:

   Arduino Esplora - https://store.arduino.cc/arduino-esplora
   Arduino LCD Screen - https://store.arduino.cc/arduino-lcd-screen
   SD Card with maps downloaded -

   The first step is to unzip the maps.zip file in the root folder of the SD card.
   Once the Sketch is uploaded to Arduino Esplora, you will be prompted with a maze
   selection screen, where you can choose one of the available maps to play.
   After the map is selected, simply use the Joystick to reach the end of the map as
   fast as possible, and see your score at the end.

 **********************************************************************************

   MAZE GENERATION AND NEXT STEPS

   The mazes were generated in Java using a simple recursive algorithm.
   It would probably be interesting to generate the maps in the game, to generate a kind
   of "endless maze" feel. Another plus would be not needing the SD card and maze
   selection screen.
   Another idea would be to create bigger mazes then it fits on the screen, and then
   make the screen scrollable.

 **********************************************************************************
*/

#include <Esplora.h>
#include <SPI.h>
#include <SD.h>
#include <TFT.h>

#define SD_CS    8  // Chip select line for SD card in Esplora

/***********************************
   Constants used in the game
 **********************************/

const int PLAYER_MOVE_DELAY = 100; // delay to wait after every move, so that player don't move too fast
const int TILE_MOVE_DELAY = 200; // delay to wait before mving to the next tile
const char WALL = 'f';
const char PATH = 't';
const byte SCREEN_SELECTION = 1; // start screen where player selects a maze
const byte SCREEN_MAZE = 2; // main screen of the game: maze
const byte SCREEN_VICTORY = 3; // when the user successfully finishes a maze

/***********************************
   Variables storing information
   for the whole application
 **********************************/

int mazeCount; // number of mazes found in SD card
byte screen; // stores the current screen
boolean previewOpen = false; // stores state of preview window

/***********************************
   Variables used during maze
   selection screen
 **********************************/

byte page = 0; // current page being displayed
int selected = 0; // curent maze number selected
byte selectedRow = 0; // remember the row of the selected maze
byte selectedCol = 0; // remember the column of the selected maze

/***********************************
   Variables storing the state
   of the current maze game
 **********************************/
char maze[1024]; // stores the entire maze
byte exitRow; // stores the coordinate X of the maze exit
byte exitCol; // stores the coordinate Y of the maze exit
byte exitQuadrant; // stores the quadrant of the maze exit
byte quadrant; // stores the quadrant of the maze
byte playerRow; // coordinate X of the player's current location
byte playerCol; // coordinate Y of the player's current location
long startTime; // stores when the current maze was started

void setup() {
  // initialize the dispay
  EsploraTFT.begin();

  // initialize SD card reader
  if (!SD.begin(8)) {
    while (1);
  }

  // check how many maze files there are
  countMazes();

  // display select maze screen
  displaySelectionPage();
}

/**
   Loads a maze from the SD card.

   Each maze file must consist of a 961 character
   sequence of 't's (path) and 'f's (wall).

   The maze files should be named as follows:
   /mazerun/maze-0.txt
   /mazerun/maze-1.txt
   ...

   There is no imit on the amount of mazes.
*/
void loadMaze() {
  // open maze file from SD card
  String filename = "MAZERUN/BIGM-" + String(selected);
  filename += ".TXT";

  File mazeFile = SD.open(filename);

  playerRow = mazeFile.parseInt();
  playerCol = mazeFile.parseInt();

  exitRow = mazeFile.parseInt();
  exitCol = mazeFile.parseInt();

  if (exitRow < 32 && exitCol < 32) {
    exitQuadrant = 0;
  } else if (exitRow < 32 && exitCol >= 32) {
    exitQuadrant = 1;
    exitCol -= 31;
  } else if (exitRow >= 32 && exitCol < 32) {
    exitQuadrant = 2;
    exitRow -= 31;
  } else {
    exitQuadrant = 3;
    exitRow -= 31;
    exitCol -= 31;
  }

  if (playerRow < 32 && playerCol < 32) {
    quadrant = 0;
  } else if (playerRow < 32 && playerCol >= 32) {
    quadrant = 1;
    playerCol -= 31;
  } else if (playerRow >= 32 && playerCol < 32) {
    quadrant = 2;
    playerRow -= 31;
  } else {
    quadrant = 3;
    playerRow -= 31;
    playerCol -= 31;
  }

  // do the same as loadQuadrant here, for performance improvement
  for (int i = 0; i <= quadrant; i++) { // find correct place in the buffer
    mazeFile.find('!');
  }

  for (int i = 0; i < 1024; i++) {
    maze[i] = (char) mazeFile.read();
  }

  // after file is read, free resources
  mazeFile.close();
}

void previewMaze2() {
  previewOpen = true;

   // draws frame
  EsploraTFT.stroke(185, 86, 53);
  EsploraTFT.noFill();
  EsploraTFT.rect(46, 30 , 67, 67);

  EsploraTFT.stroke(185, 122, 87);
  EsploraTFT.rect(47, 31 , 65, 65);

  // open maze file from SD card
  String filename = "MAZERUN/BIGM-" + String(selected);
  filename += ".TXT";

  File mazeFile = SD.open(filename);

  // draw first quadrant
  mazeFile.find('!');
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(48 + col, 32 + row);
    }
  }
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(79 + col, 32 + row);
    }
  }
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(48 + col, 63 + row);
    }
  }
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(79 + col, 63 + row);
    }
  }

   mazeFile.close();
}

/**
   Display a small window with the entirety of the maze, 
   to serve as a heping hand for the player.
*/
void previewMaze() {
  previewOpen = true;

  // draws frame around the maze
  EsploraTFT.stroke(185, 86, 53);
  EsploraTFT.noFill();
  EsploraTFT.rect(46, 30 , 67, 67);

  EsploraTFT.stroke(185, 122, 87);
  EsploraTFT.rect(47, 31 , 65, 65);

  // open maze file from SD card
  String filename = "MAZERUN/BIGM-" + String(selected);
  filename += ".TXT";

  File mazeFile = SD.open(filename);

  // draw first quadrant
  mazeFile.find('!');
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(48 + col, 32 + row);
    }
  }

  // draw second quadrant
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(79 + col, 32 + row);
    }
  }

  // draw third quadrant
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(48 + col, 63 + row);
    }
  }
  // draw fourth quadrant
  mazeFile.read();
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      char c = (char) mazeFile.read();
      if (c == PATH) {
        EsploraTFT.stroke(200, 200, 200);
      } else {
        EsploraTFT.stroke(44, 44, 44);
      }
      EsploraTFT.point(79 + col, 63 + row);
    }
  }

  // close file to free resources
  mazeFile.close();
}

/**
 * Loads a specific quadrant from the SD Card.
 * This action is done every time the player switches 
 * from one quadrant to the next.
 */
void loadQuadrant() {
  // open maze file
  String filename = "MAZERUN/BIGM-" + String(selected);
  filename += ".TXT";

  File mazeFile = SD.open(filename);

  for (int i = 0; i <= quadrant; i++) { // find correct place in the buffer
    mazeFile.find('!');
  }

  // place the maze in memory
  for (int i = 0; i < 1024; i++) {
    maze[i] = (char) mazeFile.read();
  }

  // free resources
  mazeFile.close();
}

/**
   Draws the maze on the screen.
   The first 't' path found on the edge of the maze is
   marked as the entry of the maze. The second 't' is the exit.
*/
void renderMaze() {
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      if (maze[(row * 32) + col] == PATH) {
        renderPath(col, row);
      } else {
        renderWall(col, row);
      }
    }
  }

  previewOpen = false;
}

/**
   Render the player on the specific coordinates
*/
void renderPlayer() {
  EsploraTFT.stroke(255, 0, 0);
  EsploraTFT.fill(255, 0, 0);
  EsploraTFT.rect(playerCol * 5, playerRow * 4, 5, 4);
}

/**
   Render a path on the specific coordinates
*/
void renderPath(byte col, byte row) {
  EsploraTFT.stroke(200, 200, 200);
  EsploraTFT.fill(200, 200, 200);
  EsploraTFT.rect(col * 5, row * 4, 5, 4);
}

/**
   Render a wall on the specific coordinates
*/
void renderWall(byte col, byte row) {
  EsploraTFT.stroke(44, 44, 44);
  EsploraTFT.fill(125, 125, 125);
  EsploraTFT.rect(col * 5, row * 4, 5, 4);
}

/**
   Move the player one tile to the left, only in case
   the movemet is possible (left tile is PATH, not WALL)
*/
void movePlayerLeft() {
  if (maze[(playerRow * 32) + (playerCol - 1)] == PATH) { // is tile to the left a path?
    renderPath(playerCol, playerRow); // render a path where player was
    playerCol--;
    renderPlayer(); // render player in the new location
  }
}

/**
   Move the player one tile to the right, only in case
   the movement is possible (right tile is PATH, not WALL)
*/
void movePlayerRight() {
  if (maze[(playerRow * 32) + (playerCol + 1)] == PATH) { // is tile to the right a PATH?
    renderPath(playerCol, playerRow); // render a path where player was
    playerCol++;
    renderPlayer(); // render player in the new location
  }
}

/**
   Move the player one tile downwards, only in case
   the movement is possible (down tile is PATH, not WALL)
*/
void movePlayerDown() {
  if (maze[((playerRow + 1) * 32) + playerCol] == PATH) { // is tile downwards a PATH?
    renderPath(playerCol, playerRow); // render a path where player was
    playerRow++;
    renderPlayer(); // render player in the new location
  }
}

/**
   Move the player one tile upwards, only in case
   the movement is possible (upper tile is PATH, not WALL)
*/
void movePlayerUp() {
  if (maze[((playerRow - 1) * 32) + playerCol] == PATH) { // is tile upwards a PATH?
    renderPath(playerCol, playerRow); // render a path where player was
    playerRow--;
    renderPlayer(); // render player in the new location
  }
}

/**
   Checks if player is in the exit
*/
boolean checkVictory() {
  return playerCol == exitCol && playerRow == exitRow && quadrant == exitQuadrant;
}

/**
   Display the victory screen.

   The victory screen appears on top of the
   maze, and shows how many seconds it took
   for the player to find the exit.
*/
void displayVictoryScreen() {
  screen = SCREEN_VICTORY;

  // Display "Victory" sign
  EsploraTFT.stroke(185, 86, 53);
  EsploraTFT.noFill();
  EsploraTFT.rect(12, 12 , 136, 42);
  EsploraTFT.rect(12, 70 , 136, 18);

  EsploraTFT.stroke(185, 122, 87);
  EsploraTFT.fill(52, 49, 40);
  EsploraTFT.rect(13, 13, 134, 40);
  EsploraTFT.rect(13, 71, 134, 16);

  EsploraTFT.setTextSize(3);
  EsploraTFT.stroke(255, 255, 255);
  EsploraTFT.text("Victory", 18, 21);

  // Display time spent
  EsploraTFT.setTextSize(1);
  EsploraTFT.stroke(255, 255, 255);

  int secs = (millis() - startTime) / 1000; // time spent
  String text = "Completed in " + String(secs) + " secs."; // text to display
  int len = text.length();
  char arr[len];
  text.toCharArray(arr, len); // convert from String to char array
  EsploraTFT.text(arr, 18, 75);
}

/**
   Renders a selection tile for the selection
   screen on the specified row and column
*/
void renderMazeTile(int row, int col, int mazeNum) {
  int x = col * 38 + 9; // calculate X axis on the screen
  int y = row * 29 + 34; // calculate Y axis on the screen

  // draws frame
  EsploraTFT.stroke(185, 86, 53);
  EsploraTFT.noFill();
  EsploraTFT.rect(x, y , 29, 18);

  if (selected == mazeNum) {
    EsploraTFT.stroke(255, 201, 14);
  } else {
    EsploraTFT.stroke(185, 122, 87);
  }

  EsploraTFT.fill(52, 49, 40);
  EsploraTFT.rect(x + 1, y + 1, 27, 16);

  // write maze number
  char cstr[3];
  itoa(mazeNum, cstr, 10);
  EsploraTFT.text(cstr, x + 5, y + 5);
}

/**
   Display the selection page, for the player
   to select a specific maze to play.
   This only draws the current page. The player
   can use the joystick to scroll through the pages.
*/
void displaySelectionPage() {
  EsploraTFT.background(0, 0, 0);

  EsploraTFT.setTextSize(2);
  EsploraTFT.stroke(255, 255, 255);
  EsploraTFT.text("Select Maze", 14, 8);

  EsploraTFT.setTextSize(1);
  EsploraTFT.stroke(185, 122, 87);
  EsploraTFT.noFill();
  int mazeNum;

  int limit = max(page * 12, mazeCount);
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 4; col++) {
      mazeNum = (page * 12) + (row * 4) + col;
      if (mazeNum >= limit) {
        break;
      }

      renderMazeTile(row, col, mazeNum);
    }
  }

  screen = SCREEN_SELECTION;
  previewOpen = false;
}

/**
   Counts how many maze files there are on the SD card.

   The maze files are located in /mazerun and named like:
   /mazerun/maze-0.txt
   /mazerun/maze-1.txt
   ...
*/
void countMazes() {
  mazeCount = 0;

  File root = SD.open("MAZERUN"); // open folder

  while (true) {
    File entry = root.openNextFile();

    if (!entry) {
      break;
    }

    if (!entry.isDirectory() && String(entry.name()).startsWith(F("BIGM-"))) {
      mazeCount++;
    }

    entry.close();
  }

  root.close();
}

/**
  Select one maze tile to the top
  if the tile selected is in the first row of the first page, jumps to the last page
  If the tile selected is in the first row of the page, jumps to the previous page
*/
void selectTileUp() {
  boolean firstRow = selectedRow == 0;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  if (page == 0 && firstRow) {
    int rows = (mazeCount - 1) / 4;

    selected += rows * 4;
    page = mazeCount / 12;

    selectedRow = rows - (page * 3);

    if (selected >= mazeCount) {
      selected -= 4;
      selectedRow--;
    }

    displaySelectionPage();
  } else if (firstRow) {
    // jumps to first row of the next page
    page--;
    selectedRow = 2;
    selected -= 4;

    displaySelectionPage();
  } else {
    selectedRow--;
    selected -= 4;

    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  }
}

/**
   Select one maze tile to the bottom
   If the tile selected is in the last row of the page, jumps to the next page
   If the tile selected is in the last row of the last page, jumps to first row of the last page
*/
void selectTileDown() {
  int pages = mazeCount / 12;
  boolean lastPage = page == pages;
  boolean lastRow = selectedRow == 2;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  if (lastPage && selected + 4 >= mazeCount) {
    // jumps to first row of the first page
    selected = selectedCol;
    selectedRow = 0;
    page = 0;

    displaySelectionPage();
  } else if (lastRow) {
    // jumps to first row of the next page
    page++;
    selectedRow = 0;
    selected += 4;

    displaySelectionPage();
  } else {
    // jumps to next row
    selectedRow++;
    selected += 4;

    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  }
}

/**
   Select one maze tile to the right.
   If it is the last tile on a row, jups to the next row.
   If it is the last tile on a page, jumps to the next page.
   If it is the last tile on the last page, jumps to the first tile
*/
void selectTileRight() {
  boolean lastCol = selectedCol == 3;
  boolean lastRow = selectedRow == 2;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  selected++;
  if (selected == mazeCount) { // last tile of the last page?
    // jumps to first page
    selected = 0;
    page = 0;
    selectedCol = 0;
    selectedRow = 0;

    displaySelectionPage();
  } else if (lastCol && lastRow) { // last tile of the current page?
    // jumps to next page
    selectedCol = 0;
    selectedRow = 0;
    page++;

    displaySelectionPage();
  } else if (lastCol) { // last tile of this row?
    // jumps to next first tile of row
    selectedCol = 0;
    selectedRow++;
    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  } else {
    // jumps one tile to the right
    selectedCol++;
    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  }
}

/**
   Select one maze tile to the left.
   If it is the first tile on a row, jups to the previous row.
   If it is the first tile on a page, jumps to the previous page.
   If it is the first tile on the first page, jumps to the last tile of the last page
*/
void selectTileLeft() {
  boolean firstCol = selectedCol == 0;
  boolean firstRow = selectedRow == 0;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  selected--;
  if (selected == -1) { // first tile of first page?
    // jumps to last tile of the last page
    selected = mazeCount - 1;
    page = mazeCount / 12;

    int thisPageCount = mazeCount - 1 - (page * 12);

    selectedRow = thisPageCount / 4;
    selectedCol = thisPageCount - (selectedRow * 4);

    displaySelectionPage();
  } else if (firstCol && firstRow) { // first tile of the current page?
    // jumps to last tile of the previous page
    selectedCol = 3;
    selectedRow = 2;
    page--;

    displaySelectionPage();
  } else if (firstCol) { // first tile of the row?
    // jumps to last tile of the previous row
    selectedCol = 3;
    selectedRow--;
    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  } else {
    // jumps one tile to the left
    selectedCol--;
    renderMazeTile(lastSelectedRow, lastSelectedCol, lastSelected);
    renderMazeTile(selectedRow, selectedCol, selected);
  }
}

/**
   Loads the maze selected in the selection screen.
   First, the file is loaded from an SD card and into the memory.
   Then, the maze and the player are rendered.
   Finally, the timer that will be displayed in the Victory screen is started.
*/
void openSelectedTile() {
  loadMaze(); // load from SD card
  renderMaze(); // render the maze on the screen
  renderPlayer(); // render the player on the screen
  screen = SCREEN_MAZE; // transition screen to accept different inputs

  startTime = millis(); // start victory timer
}

/**
   Main loop of the application, that reads for the player inputs,
   and apply them, depending on the current screen the player is on.
*/
void loop() {
  int joyX = Esplora.readJoystickX(); // read value of joystick
  int joyY = Esplora.readJoystickY(); // read value of joystick

  if (screen == SCREEN_SELECTION) { // inputs for selection screen
    if (Esplora.readButton(SWITCH_DOWN) == LOW) { // bottom button
      openSelectedTile(); // open tile selected
    } else if (Esplora.readButton(SWITCH_UP) == LOW) {
      if (previewOpen) {
        displaySelectionPage();
      } else {
        previewMaze();
      }
    } else if (joyX > 256) { // joystick left
      selectTileLeft();
      delay(TILE_MOVE_DELAY);
    } else if (joyX < -256) { // joystick right
      selectTileRight();
      delay(TILE_MOVE_DELAY);
    } else if (joyY < -256) { // joystick up
      selectTileUp();
      delay(TILE_MOVE_DELAY);
    } else if (joyY > 256) { // joystick down
      selectTileDown();
      delay(TILE_MOVE_DELAY);
    }
  } else if (screen == SCREEN_MAZE) { // inputs for main screen
    byte lastCol = playerCol;
    byte lastRow = playerRow;

    if (Esplora.readButton(SWITCH_RIGHT) == LOW) { // right buttom
      displaySelectionPage();
    } else if (Esplora.readButton(SWITCH_UP) == LOW) {
      if (previewOpen) {
        renderMaze();
        renderPlayer();
      } else {
        previewMaze2();
      }
    } else if (joyX > 256) { // joystick left
      movePlayerLeft();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyX < -256) { // joystick right
      movePlayerRight();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY < -256) { // joystick up
      movePlayerUp();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY > 256) { // joystick down
      movePlayerDown();
      delay(PLAYER_MOVE_DELAY);
    } else {
      return;
    }

    if (checkVictory()) { // check if he won the game
      displayVictoryScreen(); // and display victory screen
    }

    // TODO code below could probably use a refactor to be more readable
    if (playerCol == 31 && quadrant == 0 && (playerCol != lastCol || joyX < -256)) { // player is on the right edge of quadrant 0
      quadrant = 1; // show quadrant 1
      playerCol = 0;
    } else if (playerRow == 31 && quadrant == 0 && (playerRow != lastRow || joyY > 256)) { // player is on bottom edge of quadrant 0
      quadrant = 2;
      playerRow = 0;
    } else if (playerCol == 0 && quadrant == 1 && (playerCol != lastCol || joyX > 256)) {  // player is on left edge of quadrant 1
      quadrant = 0; // show quadrant 0
      playerCol = 31;
    } else if (playerRow == 31 && quadrant == 1 && (playerRow != lastRow || joyY > 256)) { // player is on bottom edge of quadrant 1
      quadrant = 3;
      playerRow = 0;
    } else if (playerRow == 0 && quadrant == 2 && (playerRow != lastRow || joyY < -256)) { // player is on top edge of quadrant 2
      quadrant = 0;
      playerRow = 31;
    } else if (playerCol == 31 && quadrant == 2 && (playerCol != lastCol || joyX < -256)) { // player is on right edge of quadran 2
      quadrant = 3;
      playerCol = 0;
    } else if (playerCol == 0 && quadrant == 3 && (playerCol != lastCol || joyX > 256)) { // player is on left edge of quadrant 3
      quadrant = 2;
      playerCol = 31;
    } else if (playerRow == 0 && quadrant == 3 && (playerRow != lastRow || joyY < -256)) { // player is on top edge of quadrant 3
      quadrant = 1;
      playerRow = 31;
    } else { // don't do anything
      return;
    }

    loadQuadrant();
    renderMaze();
    renderPlayer();

  } else if (screen == SCREEN_VICTORY) {
    if (Esplora.readButton(SWITCH_DOWN) == LOW) { // bottom button
      displaySelectionPage();

      selectTileRight();
    }
  }
}
