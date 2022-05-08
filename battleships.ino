#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <stdio.h>
#include <TimerOne.h>

LCDWIKI_SPI mylcd(ST7735S, A5, A3, -1, A2, A4, A1, A3); //software spi,model,cs,cd,miso,mosi,reset,clk,led

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

const int numRows = 4; //nr de randuri ale matricii de butoane
const int numCols = 4; //nr de coloane ale matricii de butoane
const int debounceTime = 20; // nr de milisecunde de asteptare


char battleMap[70][70] = {{'-', '-', '-', '-', '-', '-', '-', '-', '-', '\n'}, //harta 3x4 de joc
  {'|', 'X', '|', 'X', '|', 'X', '|', 'X', '|', '\n'},
  {'-', '-', '-', '-', '-', '-', '-', '-', '-', '\n'},
  {'|', 'X', '|', 'X', '|', 'X', '|', 'X', '|', '\n'},
  {'-', '-', '-', '-', '-', '-', '-', '-', '-', '\n'},
  {'|', 'X', '|', 'X', '|', 'X', '|', 'X', '|', '\n'},
  {'-', '-', '-', '-', '-', '-', '-', '-', '-', '\n'},
};

//Se definesc pinii atasati randurilor si coloanelor ale matricii de butoane, aranjati in ordinea logica
const int rowPins[numRows] = {5, 4, 3, 2}; //pinii pentru randuri
const int colPins[numCols] = {6, 7, 8, 9}; //pinii pentru coloane

//LUT pentru identificarea tastei de la intersectia unui rand cu o coloana
const int keymap[numRows][numCols] = {
  {1, 2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14, 15, 16}
};

int lives = 3;
int shipsFound = 0;
int noShips = 4;  //numarul de corabii care sa fie introduse in joc
volatile int counter = 0;

//matricea de intregi pentru plasarea aleatoare ca corăbiilor
int numberMap[3][4] = {0};
int answers[13] = {0};

void setup() {

  Serial.begin(9600);

  //initializarea piniilor butoanelor
  for (int row = 0; row < numRows; row++) {
    pinMode(rowPins[row], INPUT);
    digitalWrite(rowPins[row], HIGH);
  }

  for (int column = 0; column < numCols; column++) {
    pinMode(colPins[column], OUTPUT);
    digitalWrite(colPins[column], HIGH);
  }

  //initializarea ecranului lcd
  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK);

  //desenarea hartii de joc
  //drawMap(0);

  resetGame();

}

void loop() {

  int key = getKey(); //preluarea butonului apasat
  if (key != 0) {

    Serial.print("Got key ");
    Serial.println(key);

    if (lives > 0) //daca au mai ramas incercari
      drawMap(key);
    else {
      Serial.println("You lost!");
      //curatarea ecranului si scrierea mesajului
      mylcd.Init_LCD();
      mylcd.Fill_Screen(BLACK);
      mylcd.Print_String("You lost!:(", 0, 18);
      delay(1000);
      Timer1.restart();
      resetGame();
    }
  }

  char buffer[200];

  //counter = millis() / 1000;
  Timer1.attachInterrupt(countAdd);
  snprintf(buffer, 100, "Lives:%d %d", lives, counter);

  //countAdd();
  mylcd.Print_String(buffer, 0, 130);



  checkIfWin();
}

//-------Metode Implementate-------//

void countAdd() {
  counter++;
}

void resetGame() {

  lives = 3;
  counter = 0;
  shipsFound = 0;

  Timer1.initialize(1000000);

  //pregătirea corăbiilor
  resetMaps();
  randomizeShips(noShips);

  //afisarea rezultatului in serial monitor pentru verificare
  Serial.print("The hidden ships: ");
  Serial.println(" ");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      Serial.print(numberMap[i][j]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }

  //desenarea hartii de joc
  drawMap(0);
}

void resetMaps() {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 4; j++) {
      numberMap[i][j] = 0;
      changeXAt(i, j);
    }
}

void randomizeShips(int noShips) {
  int randRow = 0;
  int randCol = 0;

  for (int i = 0; i < noShips; i++) {
    randRow = random(0, 3);
    randCol = random(0, 4);
    while (numberMap[randRow][randCol] == 1) {
      randRow = random(0, 3);
      randCol = random(0, 4);
    }
    numberMap[randRow][randCol] = 1;
  }
}

int alreadyAnswered(int key) {
  for (int i = 0; i < noShips; i++) {
    if (key == answers[i])
      return 1;
  }
  return 0;
}

void drawMapOnLCD() {
  int rowDist = 18;
  for (int i = 0; i < 7; i++) {
    mylcd.Print_String(battleMap[i], 0, rowDist);
    rowDist += 15;
  }
}

void checkIfWin() {
  if (shipsFound == noShips) {
    Serial.println("Victory!");

    mylcd.Init_LCD();
    mylcd.Fill_Screen(BLACK);
    mylcd.Print_String("You won!:)", 0, 18);
    Timer1.restart();
    delay(1000);
    resetGame();
  }
}

void drawMap(int key) {

  mylcd.Set_Text_Mode(0);
  mylcd.Fill_Screen(0x0000);
  mylcd.Set_Text_colour(BLUE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(2);

  if (key > 0)

    switch (key) {

      case 0: break;
      case 1: checkIfShip(0, 0, 1); break;
      case 2: checkIfShip(0, 1, 2); break;
      case 3: checkIfShip(0, 2, 3); break;
      case 4: checkIfShip(0, 3, 4); break;
      case 5: checkIfShip(1, 0, 5); break;
      case 6: checkIfShip(1, 1, 6); break;
      case 7: checkIfShip(1, 2, 7); break;
      case 8: checkIfShip(1, 3, 8); break;
      case 9: checkIfShip(2, 0, 9); break;
      case 10: checkIfShip(2, 1, 10); break;
      case 11: checkIfShip(2, 2, 11); break;
      case 12: checkIfShip(2, 3, 12); break;

      case 13: //butonul de reset
        Serial.print("Resetting!");
        Serial.println(" ");
        resetGame();
        break;
      case 14: break;
      case 15: break;
      case 16:  break;
    }




  drawMapOnLCD();

}

void changeXAt(int i, int j) {
  battleMap[2 * i + 1][2 * j + 1] = 'X';
}

void checkIfShip(int i, int j, int key) {
  if (numberMap[i][j] == 1) {
    battleMap[2 * i + 1][2 * j + 1] = 'O';
    if (alreadyAnswered(key) == 0) {
      answers[shipsFound] = key;
      shipsFound++;
      Serial.println("Already answered: ");
      for (int i = 0; i < shipsFound; i++)
      {
        Serial.print(answers[i]);
        Serial.print(" ");
      }
      Serial.println(" ");
    }
  }
  else {
    lives--;
  }
}

int getKey() {
  int key = 0;

  for (int column = 0; column < numCols; column++) {
    digitalWrite(colPins[column], LOW);
    for (int row = 0; row < numRows; row++) {
      if (digitalRead(rowPins[row]) == LOW) {
        delay(debounceTime);
        while (digitalRead(rowPins[row]) == LOW);
        key = keymap[row][column];
      }
    }
    digitalWrite(colPins[column], HIGH);
  }

  return key;
}
