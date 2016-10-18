#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#include "globals.h"

struct Players
{
  public:
    int x;
    int y;
    boolean walking;
    byte direction;
    byte frame;
    unsigned long score; 
};

Players player = { .x = 20, .y = 20, .walking = false, .direction = FACING_SOUTH, .frame = 0 };

void drawPlayer()
{
  if (arduboy.everyXFrames(6) && player.walking) player.frame++;
  if (player.frame > 3 ) player.frame = 0;
  sprites.drawPlusMask(player.x - cam.x -1, player.y - cam.y -1, playerMask_plus_mask, player.direction);
  sprites.drawSelfMasked(player.x - cam.x, player.y - cam.y, playerSprite, player.frame + 4 * player.direction);
}

#endif
