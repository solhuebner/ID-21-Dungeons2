#ifndef LEVEL_H
#define LEVEL_H

#include <Arduino.h>
#include "Arglib.h"
#include "globals.h"

extern Arduboy arduboy;

#define TOTAL_ROOMS   7
#define LEVEL_CHUNK_W 16
#define LEVEL_CHUNK_H 16
#define CHUNK_W       4
#define CHUNK_H       4
#define TILE_W        16
#define TILE_H        16
#define ROOM_W        3
#define ROOM_H        3
#define LEVEL_W       LEVEL_CHUNK_W * CHUNK_W *TILE_W
#define LEVEL_H       LEVEL_CHUNK_H * CHUNK_H *TILE_H
#define CHUNK_PW      CHUNK_W * TILE_W
#define CHUNK_PH      CHUNK_H * TILE_H
#define _BV2(bit) ~(1 << (bit))
#define sign(v) (v > 0) - (v < 0)


byte getChunkID(const byte * levelarray, const int &rx, const int &ry);
bool getChunkBit(const byte * levelarray, const int &rx, const int &ry);
byte getTileInChunk(const byte * chunkArray, const byte &chunk, const byte &cx, const byte &cy);
int r_to_c(const int x);

byte getTileID(const byte * levelarray, const int &rx, const int &ry)
{
  //Serial.println("get tile id");
  byte chunk = getChunkID(levelarray, rx, ry);
  return getTileInChunk(chunkSet, chunk, r_to_c(rx), r_to_c(ry));
}

bool getSolid(const byte * levelarray, const int &rx, const int &ry)
{
  return getTileID(levelarray, rx, ry) <= 12;
}

byte getChunkID(const byte * levelarray, const int &rx, const int &ry)
{
  //Serial.println("get chunk id");
  // Not solid
  if (!getChunkBit(levelarray, rx, ry))
  {
    byte b = 16;// + ((rx + ry) % 5);
    Serial.print("Chunk ID: ");
    Serial.println(b);
    return b;
  }
  else
  {
    byte b = 0;
    b |= getChunkBit(levelarray, rx + CHUNK_PW, ry);  // right of current
    b |= getChunkBit(levelarray, rx, ry - CHUNK_PH) << 1;  // above
    b |= getChunkBit(levelarray, rx - CHUNK_PW, ry) << 2;  // left
    b |= getChunkBit(levelarray, rx, ry + CHUNK_PH) << 3;  // below

    Serial.print("Chunk ID: ");
    Serial.println(b);
    return b;
  }
}

bool getChunkBit(const byte * levelarray, const int &rx, const int &ry)
{
  if (rx < 0 || rx >= LEVEL_W
  || ry < 0 || ry >= LEVEL_H)
    return 1;
  byte x = rx / LEVEL_CHUNK_W / CHUNK_W / 8;    // 8 is number of chunks per byte
  byte y = ry / CHUNK_PW * 2;           // 2 is number of bytes per row
  byte i = (rx / LEVEL_CHUNK_W / CHUNK_W) % 8;

  return ((levelarray[x + y] & _BV(i)) > 0);
}

byte getTileInChunk(const byte * chunkArray, const byte &chunkID, const byte &cx, const byte &cy)
{
  /*Serial.print("Chunk ID: ");
  Serial.print(chunkID);
  Serial.print(" cx: ");
  Serial.print(cx);
  Serial.print(" cy: ");
  Serial.println(cy);*/
  byte nibble = pgm_read_byte(chunkArray + (chunkID * 8) + (cx / 2) + (cy * 2));
  byte i = cx % 2;
  nibble = (nibble >> (i * 4)) & 0X0F;
  return nibble;
}

int r_to_c(const int x)
{
  int xx = ((x % 64) / 16);
  //Serial.print("r_to_c: ");
  //Serial.print(x);
  //Serial.print(" -> ");
  //Serial.println(xx);
  return xx;
}

void setBit(byte  &b, const byte &i, const bool &v)
{
  if (v) b = b | _BV(i);
  else b = b & _BV2(i);
}

void setBitXY(byte * bytearray, const int &x, const int &y, const byte &w, const bool &v)
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
    for (byte a = 0; a <= abs(x2 - x1); a++)
    {
        setBitXY(levelarray, x1 + a * sign(x2 - x1), y1, LEVEL_CHUNK_W, 0);
    }
    for (byte b = 0; b <= abs(y2 - y1); b++)
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

void drawTiles()
{
  //Serial.println("draw tiles:");
  for (int x = (cam.x >> 4); x <= (cam.x >> 4) + 8; ++x)
  {
     for (int y = (cam.y >> 4); y <= (cam.y >> 4) + 4; ++y)
     {
        byte tile = getTileID(levelArray, x << 4, y << 4);
        //Serial.print("tile: ");
        //Serial.println(tile);
        sprites.drawErase((x << 4) - cam.x, (y << 4) - cam.y, bm_tileset, tile);
     }
  }
}

#endif
