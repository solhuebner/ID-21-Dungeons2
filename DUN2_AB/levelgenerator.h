#ifndef LEVEL_H
#define LEVEL_H

#include <Arduino.h>
#include "globals.h"
#include "Arglib.h"

#define TOTAL_ROOMS   7
#define LEVEL_CHUNK_W 16
#define LEVEL_CHUNK_H 16
#define ROOM_W        3
#define ROOM_H        3

#define _BV2(bit) ~(1 << (bit))
#define sign(v) (v > 0) - (v < 0)

void setBit(byte  &b, byte i, bool v)
{
  if (v) b = b | _BV(i);
  else b = b & _BV2(i);
}

void setBitXY(byte * bytearray, int x, int y, byte w, bool v)
{
  if (x < 0 || y < 0) return;

  setBit(bytearray[(x/8) + y *(w / 8)], x % 8, v);
}

void hallwaysGenerate(byte * levelarray, Rect * rooms)
{
  for (byte i = 0; i < TOTAL_ROOMS - 1; ++i)
  {
    // Connect rooms via 1 bit wide hallways in sequential order
    int x1 = rooms[i].x;
    int y1 = rooms[i].y;
    int x2 = rooms[i + 1].x;
    int y2 = rooms[i + 1].y;
    for (var a = 0; a <= abs(x2 - x1); a++)
    {
        setBitXY(levelarray, x1 + a * sign(x2 - x1), y1, LEVEL_CHUNK_W, 0);
    }
    for (var b = 0; b <= abs(y2 - y1); b++)
    {
        setBitXY(levelarray, x2, y1 + b * sign(y2 - y1), LEVEL_CHUNK_W, 0);
    }
  }
}

void levelGenerate(byte * levelarray, long int levelseed)
{
  // Clear level array (all solid)
  for (byte i = 0; i < LEVEL_CHUNK_W * LEVEL_CHUNK_H / 8; ++i)
  {
    levelarray[i] = 0xFF;
  }
  // Set seed to generate same level sequence each play through
  randomSeed(levelseed);

  Rect * rooms = (Rect*)malloc(sizeof(Rect) * TOTAL_ROOMS);

  // Randomly place rooms in level
  for (byte i = 0; i < TOTAL_ROOMS; ++i)
  {
    rooms[i].x = random(LEVEL_CHUNK_W - ROOM_W);
    rooms[i].y = random(LEVEL_CHUNK_H - ROOM_H);
    rooms[i].width = ROOM_W;
    rooms[i].height = ROOM_H;
  }

  // Remove collisions between rooms
  bool col = true;
  byte tries = 255;
  while (col && tries > 0)
  {
    col = false;
    for (byte a = 0; a < TOTAL_ROOMS; ++a)
    {
      for (byte b = 0; b < TOTAL_ROOMS; ++b)
      {
         if (a != b && arduboy.collide(rooms[a], rooms[b]))
         {
            col = true;
            // Move away x
            if (rooms[a].x <= rooms[b].x && rooms[a].x > 0) --rooms[a].x;
            else if (rooms[a].x >= rooms[b].x && rooms[a].x < LEVEL_CHUNK_W - ROOM_W - 1) ++rooms[a].x;
            // Move away y
            if (rooms[a].y <= rooms[b].y && rooms[a].y > 0) --rooms[a].y;
            else if (rooms[a].y >= rooms[b].y && rooms[a].y < LEVEL_CHUNK_H - ROOM_H - 1) ++rooms[a].y;
         }
      }
    }

    --tries;
  }

  // Imprint rooms into level array
  for (byte a = 0; a < TOTAL_ROOMS; ++a)
  {
    for (byte x = 0; x < ROOM_W; ++x)
    {
      for (byte y = 0; y < ROOM_H; ++y)
      {
        setBitXY(levelarray, rooms[a].x + x, rooms[a].y + y, LEVEL_CHUNK_W, 0);
      }
    }
  }

  // Create connecting hallways between rooms
  hallwaysGenerate(levelarray, rooms);

  free(rooms);
}

#endif
