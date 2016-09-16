#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "globals.h"
#include "inputs.h"
#include "player.h"
#include "enemies.h"
#include "elements.h"
#include "levels.h"
#include "levelgenerator.h"

void stateMenuPlay()
//void stateGamePrepareLevel()
{
  player.score = 0; 
  gameState = STATE_GAME_NEXT_LEVEL;
  //Serial.print("play");
};


void stateGameNextLevel()
{
  level++;
  gameState = STATE_GAME_PLAYING;
};


void stateGamePlaying()
{
  drawTiles();
  checkInputs();
  drawPlayer();
  checkCollisions();

  if (arduboy.justPressed(B_BUTTON))
  {
    // Testing
    level++;
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("---Level ");
    Serial.print(level);
    Serial.println("---");
    levelGenerate(levelArray, level);
    for (int y = 0; y <= 16; ++y)
    {
      for (int x = 0; x <= 16; ++x)
      {
        Serial.print((getChunkBit(levelArray, x, y)) ? "|X|" : "   ");
      }
      Serial.println(" ");
    }
  }
};

void stateGamePause()
{
  if (arduboy.justPressed(A_BUTTON)) gameState = STATE_GAME_PLAYING;
};

void stateGameOver()
{
  
};



#endif
