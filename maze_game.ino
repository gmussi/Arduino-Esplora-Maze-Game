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

/***********************************
   Variables used during maze
   selection screen
 **********************************/

byte page = 0;
int selected = 0;
byte selectedRow = 0;
byte selectedCol = 0;

/***********************************
   Variables storing the state
   of the current maze game
 **********************************/
char maze[961];
byte entryX;
byte entryY;
byte exitX;
byte exitY;
byte playerX;
byte playerY;
long startTime;

void setup() {
  Serial.begin(9600);

  // initialize SD card reader
  if (!SD.begin(SD_CS)) {
    while (1);
  }

  // initialize the dispay
  EsploraTFT.begin();
  EsploraTFT.setTextSize(1);

  // display logo
  //displayIntro();

  // check how many maze files there are
  countMazes();

  // select maze screen
  displaySelectionPage();
}

void loadMaze(byte num) {
  String filename = "mazerun/maze-" + String(num);

  File mazeFile = SD.open(filename + ".txt");

  int i = 0;
  while (mazeFile.available()) {
    maze[i++] = (char) mazeFile.read();
  }

  mazeFile.close();
}

void initMaze() {
  boolean entryFound = false;
  for (int i = 0; i < 31; i++) {
    for (int j = 0; j < 31; j++) {
      if (maze[(i * 31) + j] == PATH) {
        if (i == 0 || i == 30 || j == 0 || j == 30) {
          if (entryFound) {
            exitX = i;
            exitY = j;

          } else {
            entryX = i;
            entryY = j;
            entryFound = true;
          }
        }

        drawPath(i, j);
      } else {
        drawWall(i, j);
      }
    }
  }

  playerX = entryX;
  playerY = entryY;
}

void drawPlayer(byte x, byte y) {
  EsploraTFT.stroke(255, 0, 0);
  EsploraTFT.fill(255, 0, 0);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

void drawPath(byte x, byte y) {
  EsploraTFT.stroke(200, 200, 200);
  EsploraTFT.fill(200, 200, 200);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

void drawWall(byte x, byte y) {
  EsploraTFT.stroke(44, 44, 44);
  EsploraTFT.fill(125, 125, 125);
  EsploraTFT.rect(x * 5, y * 4, 5, 4);
}

void movePlayerLeft() {
  // can go left?
  if (maze[((playerX - 1) * 31) + playerY] == PATH) {
    drawPath(playerX, playerY);
    drawPlayer(--playerX, playerY);
  }
}

void movePlayerRight() {
  // can go right?
  if (maze[((playerX + 1) * 31) + playerY] == PATH) {
    drawPath(playerX, playerY);
    drawPlayer(++playerX, playerY);
  }
}

void movePlayerDown() {
  // can go down?
  if (maze[(playerX * 31) + (playerY - 1)] == PATH) {
    drawPath(playerX, playerY);
    drawPlayer(playerX, --playerY);
  }
}

void movePlayerUp() {
  // can go up?
  if (maze[(playerX * 31) + (playerY + 1)] == PATH) {
    drawPath(playerX, playerY);
    drawPlayer(playerX, ++playerY);
  }
}

boolean checkVictory() {
  return playerX == exitX && playerY == exitY;
}

void finnishMaze() {
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

  int secs = (millis() - startTime) / 1000;
  String text = "Completed in " + String(secs) + " secs.";
  int len = text.length();
  char arr[len];
  text.toCharArray(arr, len);
  EsploraTFT.text(arr, 18, 75);
}

void displayIntro() {
  PImage logo = EsploraTFT.loadImage("mazerun/logo.bmp");
  EsploraTFT.image(logo, 0, 0);
}

void drawMazeNum(int row, int col, int mazeNum) {
  int x = col * 38 + 9;
  int y = row * 29 + 34;

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

  char cstr[3];
  itoa(mazeNum, cstr, 10);
  EsploraTFT.text(cstr, x + 5, y + 5);
}

void displaySelectionPage() {
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

      drawMazeNum(row, col, mazeNum);
    }
  }

  screen = SCREEN_SELECTION;
}

void countMazes() {
  mazeCount = 0;
  File folder = SD.open("mazerun/");

  File entry;
  while (entry = folder.openNextFile()) {
    if (String(entry.name()).startsWith(F("MAZE-"))) {
      mazeCount++;
    }
    entry.close();
  }
}

void selectTileUp() {

}

void selectTileDown() {

}

void selectTileRight() {
  boolean lastCol = selectedCol == 3;
  boolean lastRow = selectedRow == 2;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  selected++;
  if (selected == mazeCount) { //
    selected = 0;
    page = 0;
    selectedCol = 0;
    selectedRow = 0;

    displaySelectionPage();
  } else if (lastCol && lastRow) { // check if the tile selected is the last
    selectedCol = 0;
    selectedRow = 0;
    page++;

    displaySelectionPage();
  } else if (lastCol) {
    selectedCol = 0;
    selectedRow++;
    drawMazeNum(lastSelectedRow, lastSelectedCol, lastSelected);
    drawMazeNum(selectedRow, selectedCol, selected);
  } else {
    selectedCol++;
    drawMazeNum(lastSelectedRow, lastSelectedCol, lastSelected);
    drawMazeNum(selectedRow, selectedCol, selected);
  }
}

void selectTileLeft() {
  boolean firstCol = selectedCol == 0;
  boolean firstRow = selectedRow == 0;

  byte lastSelectedRow = selectedRow;
  byte lastSelectedCol = selectedCol;
  int lastSelected = selected;

  selected--;
  if (selected == -1) {
    selected = mazeCount - 1;
    page = mazeCount / 12;

    int thisPageCount = mazeCount - 1 - (page * 12);

    selectedRow = thisPageCount / 4;
    selectedCol = thisPageCount - (selectedRow * 4);

    displaySelectionPage();
  } else if (firstCol && firstRow) { // check if the tile selected is the first
    selectedCol = 3;
    selectedRow = 2;
    page--;

    displaySelectionPage();
  } else if (firstCol) {
    selectedCol = 3;
    selectedRow--;
    drawMazeNum(lastSelectedRow, lastSelectedCol, lastSelected);
    drawMazeNum(selectedRow, selectedCol, selected);
  } else {
    selectedCol--;
    drawMazeNum(lastSelectedRow, lastSelectedCol, lastSelected);
    drawMazeNum(selectedRow, selectedCol, selected);
  }
}

void openSelectedTile() {
  loadMaze(selected);
  initMaze();
  drawPlayer(entryX, entryY);
  screen = SCREEN_MAZE;

  startTime = millis();
}

void loop() {
  int joyX = Esplora.readJoystickX(); // read value of joystick
  int joyY = Esplora.readJoystickY(); // read value of joystick

  if (screen == SCREEN_SELECTION) {
    if (Esplora.readButton(SWITCH_DOWN) == LOW) {
      openSelectedTile();
    } else if (joyX > 256) { // go left
      selectTileLeft();
      delay(TILE_MOVE_DELAY);
    } else if (joyX < -256) { // go right
      selectTileRight();
      delay(TILE_MOVE_DELAY);
    } else if (joyY < -256) { // go down
      selectTileDown();
      delay(TILE_MOVE_DELAY);
    } else if (joyY > 256) { // go up
      selectTileUp();
      delay(TILE_MOVE_DELAY);
    }
  } else if (screen == SCREEN_MAZE) {
    boolean moved = true;
    if (joyX > 256) { // go left
      movePlayerLeft();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyX < -256) { // go right
      movePlayerRight();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY < -256) { // go down
      movePlayerDown();
      delay(PLAYER_MOVE_DELAY);
    } else if (joyY > 256) { // go up
      movePlayerUp();
      delay(PLAYER_MOVE_DELAY);
    } else {
      moved = false;
    }

    if (moved && checkVictory()) {
      finnishMaze();
    }
  } else if (screen == SCREEN_VICTORY) {
      if (Esplora.readButton(SWITCH_DOWN) == LOW) {
        displaySelectionPage();  
      }
  }
}
