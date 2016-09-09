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
  //Serial.println("playing");
  if (arduboy.pressed(LEFT_BUTTON)) (cam.x > 0) ? cam.x-- : 0;
  if (arduboy.pressed(RIGHT_BUTTON)) (cam.x < 1023) ? cam.x++ : 895;
  if (arduboy.pressed(UP_BUTTON)) (cam.y > 0) ? cam.y-- : 0;
  if (arduboy.pressed(DOWN_BUTTON)) (cam.y < 1023) ? cam.y++ : 895;
  drawTiles();
  checkInputs();
  drawPlayer();
  checkCollisions();
};

void stateGamePause()
{
  if (arduboy.justPressed(A_BUTTON)) gameState = STATE_GAME_PLAYING;
};

void stateGameOver()
{
  
};



#endif
