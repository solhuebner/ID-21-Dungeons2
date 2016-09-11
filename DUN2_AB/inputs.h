#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include "globals.h"
#include "player.h"
#include "levelgenerator.h"

void checkInputs()
{
  player.walking = false;
  if (arduboy.pressed(DOWN_BUTTON))
  {
    player.direction = FACING_SOUTH;
    if (!getSolid(levelArray, player.x + 8, player.y + 16)) player.y++;
    player.walking = true;
  }
  if (arduboy.pressed(LEFT_BUTTON))
  {
    player.direction = FACING_WEST;
    if (!getSolid(levelArray, player.x - 1, player.y + 8)) player.x--;
    player.walking = true;
  }
  if (arduboy.pressed(UP_BUTTON))
  {
    player.direction = FACING_NORTH;
    if (!getSolid(levelArray, player.x + 8, player.y - 1)) player.y--;
    player.walking = true;
  }
  if (arduboy.pressed(RIGHT_BUTTON))
  {
    player.direction = FACING_EAST;
    if (!getSolid(levelArray, player.x + 16, player.y + 8)) player.x++;
    player.walking = true;
  }

  // update camera
  cam.x = player.x - 56;
  cam.y = player.y - 24;

  if (arduboy.justPressed(A_BUTTON)) gameState = STATE_GAME_PAUSE;
  if (arduboy.justPressed(B_BUTTON)) arduboy.audio.tone(880, 20);
}


#endif
