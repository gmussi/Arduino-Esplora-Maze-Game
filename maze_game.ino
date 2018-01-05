/**********************************************************************************
 * =======================================
 *    TODOS
 *    
 *    create splash screen
 *    replace X and Y by ROW and COL
 *    use UP and DOWN on the maze selection screen
 *    put a "maze selection" title on the maze selection page
 *    upload .zip to github and paste link here
 *    create github and arduino project hub page
 *    
 *  
 *  =====================================
 *  
 * This is a Maze Game I developed as a learning project for the Arduino
 * platform. This code is developed specifically for the Arduino Esplora with LCD,
 * but could be easily adapted to any other Arduino, by replacing the calls to the
 * "Esplora" and "EsploraTFT" libraries.
 * The maze are stored and loaded from an SD card and consist of a 961 char sequence 
 * of 'f' (wall) and 't' (path) which form a 31x31 maze.. When the player selects a 
 * map and reaches its end, a victory screen is shown with the time taken by the player 
 * to complete the map.
 * 
 * Author: Guilherme Mussi <https://github.com/gmussi>
 * 
 * Github Project Page: https://github.com/gmussi/Arduino-Esplora-Maze-Game
 * Arduino Project Hub: 
 * 
 ********************************************************************************** 
 * 
 * INSTRUCTIONS
 * 
 * To run the game, you will need the following:
 * 
 * Arduino Esplora - https://store.arduino.cc/arduino-esplora
 * Arduino LCD Screen - https://store.arduino.cc/arduino-lcd-screen
 * SD Card with maps downloaded - 
 * 
 * The first step is to unzip the maps.zip file in the root folder of the SD card.
 * Once the Sketch is uploaded to Arduino Esplora, you will be prompted with a maze
 * selection screen, where you can choose one of the available maps to play.
 * After the map is selected, simply use the Joystick to reach the end of the map as
 * fast as possible, and see your score at the end.
 * 
 ********************************************************************************** 
 * 
 * MAZE GENERATION AND NEXT STEPS
 * 
 * The mazes were generated in Java using a simple recursive algorithm. 
 * It would probably be interesting to generate the maps in the game, to generate a kind
 * of "endless maze" feel. Another plus would be not needing the SD card and maze 
 * selection screen.
 * Other ideas:
 * 1. Store on SD card best time for each map
 * 2. Block a map until previous map has been completed
 * 3. Start maze selection screen on first uncompleted map
 * 4. Show a HIGH SCORE screen
 * 
 ********************************************************************************** 
 */
 
#include <Esplora.h>
#include <SPI.h>
#include <SD.h>
#include <TFT.h>

#define SD_CS    8  // Chip select line for SD card in Esplora

/***********************************
 * Constants used in the game
 **********************************/

const int PLAYER_MOVE_DELAY = 100; // delay to wait after every move, so that player don't move too fast
const int TILE_MOVE_DELAY = 200; // delay to wait before mving to the next tile
const char WALL = 'f';
const char PATH = 't';
const byte SCREEN_SELECTION = 1; // start screen where player selects a maze
const byte SCREEN_MAZE = 2; // main screen of the game: maze
const byte SCREEN_VICTORY = 3; // when the user successfully finishes a maze

/***********************************
 * Variables storing information
 * for the whole application
 **********************************/

int mazeCount; // number of mazes found in SD card
byte screen; // stores the current screen

/***********************************
 * Variables used during maze
 * selection screen
 **********************************/

byte page = 0; // current page being displayed
int selected = 0; // curent maze number selected
byte selectedRow = 0; // remember the row of the selected maze
byte selectedCol = 0; // remember the column of the selected maze

/***********************************
 * Variables storing the state
 * of the current maze game
 **********************************/
char maze[961]; // stores the entire maze
byte entryX; // stores the coordinate X of the maze entry
byte entryY; // stores the coordinate Y of the maze entry
byte exitX; // stores the coordinate X of the maze exit
byte exitY; // stores the coordinate Y of the maze exit
byte playerX; // coordinate X of the player's current location
byte playerY; // coordinate Y of the player's current location
long startTime; // stores when the current maze was started

void setup() {
  // initialize the dispay
  EsploraTFT.begin();

  // display splash screen while things load
  displaySplashScreen();

  // initialize SD card reader
  if (!SD.begin(SD_CS)) {
    while (1);
  }
  
  // check how many maze files there are
  countMazes();

  // display select maze screen
  displaySelectionPage();
}

/**
 * Loads a maze from the SD card.
 * 
 * Each maze file must consist of a 961 character 
 * sequence of 't's (path) and 'f's (wall).
 * 
 * The maze files should be named as follows:
 * /mazerun/maze-0.txt
 * /mazerun/maze-1.txt
 * ...
 * 
 * There is no imit on the amount of mazes. 
 */
void loadMaze(byte num) {
  // open maze file from SD card
  String filename = "mazerun/maze-" + String(num);
  File mazeFile = SD.open(filename + ".txt");

  // place the maze in memory
  int i = 0;
  while (mazeFile.available()) {
    maze[i++] = (char) mazeFile.read();
  }

  // after file is read, free resources
  mazeFile.close();
}

/**
 * Draws the maze on the screen.
 * The first 't' path found on the edge of the maze is
 * marked as the entry of the maze. The second 't' is the exit.
 */
void renderMaze() {
  boolean entryFound = false;
  
  for (int i = 0; i < 31; i++) {
    for (int j = 0; j < 31; j++) {
      if (maze[(i * 31) + j] == PATH) {
        if (i == 0 || i == 30 || j == 0 || j == 30) {
          if (entryFound) { // second path on the edge is the exit
            exitX = i;
            exitY = j;
          } else { // first path on the edge is the entry
            entryX = i;
            entryY = j;
            entryFound = true;
          }
        }

        renderPath(i, j);
      } else {
        renderWall(i, j);
      }
    }
  }

  // place player in the entrance of the maze
  playerX = entryX;
  playerY = entryY;
}

/**
 * Render the player on the specific coordinates
 */
void renderPlayer(byte x, byte y) {
  EsploraTFT.stroke(255, 0, 0);
  EsploraTFT.fill(255, 0, 0);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

/**
 * Render a path on the specific coordinates
 */
void renderPath(byte x, byte y) {
  EsploraTFT.stroke(200, 200, 200);
  EsploraTFT.fill(200, 200, 200);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

/**
 * Render a wall on the specific coordinates
 */
void renderWall(byte x, byte y) {
  EsploraTFT.stroke(44, 44, 44);
  EsploraTFT.fill(125, 125, 125);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

/**
 * Move the player one tile to the left, only in case
 * the movemet is possible (left tile is PATH, not WALL)
 */
void movePlayerLeft() {
  if (maze[((playerX - 1) * 31) + playerY] == PATH) { // is tile to the left a path?
    renderPath(playerX, playerY); // render a path where player was
    renderPlayer(--playerX, playerY); // render player in the new location
  }
}

/**
 * Move the player one tile to the right, only in case
 * the movement is possible (right tile is PATH, not WALL)
 */
void movePlayerRight() {
  if (maze[((playerX + 1) * 31) + playerY] == PATH) { // is tile to the right a PATH?
    renderPath(playerX, playerY); // render a path where player was
    renderPlayer(++playerX, playerY); // render player in the new location
  }
}

/**
 * Move the player one tile downwards, only in case
 * the movement is possible (down tile is PATH, not WALL)
 */
void movePlayerDown() {
  if (maze[(playerX * 31) + (playerY - 1)] == PATH) { // is tile downwards a PATH?
    renderPath(playerX, playerY); // render a path where player was
    renderPlayer(playerX, --playerY); // render player in the new location
  }
}

/**
 * Move the player one tile upwards, only in case
 * the movement is possible (upper tile is PATH, not WALL)
 */
void movePlayerUp() {
  if (maze[(playerX * 31) + (playerY + 1)] == PATH) { // is tile upwards a PATH?
    renderPath(playerX, playerY); // render a path where player was
    renderPlayer(playerX, ++playerY); // render player in the new location
  }
}

/**
 * Checks if player is in the exit
 */
boolean checkVictory() {
  return playerX == exitX && playerY == exitY;
}

/**
 * Display the victory screen.
 * 
 * The victory screen appears on top of the 
 * maze, and shows how many seconds it took
 * for the player to find the exit.
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
 * Displays a splash screen.
 * It should be quick to render so that application
 * loads faster.
 */
void displaySplashScreen() {
  
}

/**
 * Renders a selection tile for the selection 
 * screen on the specified row and column
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
 * Display the selection page, for the player
 * to select a specific maze to play.
 * This only draws the current page. The player
 * can use the joystick to scroll through the pages.
 */
void displaySelectionPage() {
  // TODO : place title
  EsploraTFT.setTextSize(1);
  EsploraTFT.background(0, 0, 0);
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
}

/**
 * Counts how many maze files there are on the SD card.
 * 
 * The maze files are located in /mazerun and named like:
 * /mazerun/maze-0.txt
 * /mazerun/maze-1.txt
 * ...
 */
void countMazes() {
  mazeCount = 0;
  File folder = SD.open("mazerun/"); // open folder

  File entry;
  while (entry = folder.openNextFile()) { // go through all files and folder
    if (String(entry.name()).startsWith(F("MAZE-"))) { // check if it is a maze file
      mazeCount++;
    }
    entry.close(); // free resources
  }
}

void selectTileUp() {

}

void selectTileDown() {

}

/**
 * Select one maze tile to the right.
 * If it is the last tile on a row, jups to the next row.
 * If it is the last tile on a page, jumps to the next page.
 * If it is the last tile on the last page, jumps to the first tile
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
 * Select one maze tile to the left.
 * If it is the first tile on a row, jups to the previous row.
 * If it is the first tile on a page, jumps to the previous page.
 * If it is the first tile on the first page, jumps to the last tile of the last page
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
 * Loads the maze selected in the selection screen.
 * First, the file is loaded from an SD card and into the memory.
 * Then, the maze and the player are rendered.
 * Finally, the timer that will be displayed in the Victory screen is started.
 */
void openSelectedTile() {
  loadMaze(selected); // load from SD card
  renderMaze(); // render the maze on the screen
  renderPlayer(entryX, entryY); // render the player on the screen
  screen = SCREEN_MAZE; // transition screen to accept different inputs

  startTime = millis(); // start victory timer
}

/**
 * Main loop of the application, that reads for the player inputs, 
 * and apply them, depending on the current screen the player is on.
 */
void loop() {
  int joyX = Esplora.readJoystickX(); // read value of joystick
  int joyY = Esplora.readJoystickY(); // read value of joystick

  if (screen == SCREEN_SELECTION) { // inputs for selection screen
    if (Esplora.readButton(SWITCH_DOWN) == LOW) { // bottom button
      openSelectedTile();
    } else if (joyX > 256) { // joystick left
      selectTileLeft();
      delay(TILE_MOVE_DELAY);
    } else if (joyX < -256) { // joystick right
      selectTileRight();
      delay(TILE_MOVE_DELAY);
    } else if (joyY < -256) { // joystick down
      selectTileDown();
      delay(TILE_MOVE_DELAY);
    } else if (joyY > 256) { // joystick up
      selectTileUp();
      delay(TILE_MOVE_DELAY);
    }
  } else if (screen == SCREEN_MAZE) { // inputs for main screen
    boolean moved = true;
    if (joyX > 256) { // joystick left
      movePlayerLeft();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyX < -256) { // joystick right
      movePlayerRight();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY < -256) { // joystick down
      movePlayerDown();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY > 256) { // joystick up
      movePlayerUp();
      delay(PLAYER_MOVE_DELAY);
    } else {
      moved = false;
    }

    if (moved && checkVictory()) { // if the player moved, check if he won the game
      displayVictoryScreen(); 
    }
  } else if (screen == SCREEN_VICTORY) {
      if (Esplora.readButton(SWITCH_DOWN) == LOW) { // bottom button
        displaySelectionPage();  
      } 
  }
}
