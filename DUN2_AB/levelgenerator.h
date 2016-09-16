#ifndef LEVEL_H
#define LEVEL_H

#include <Arduino.h>
#include "Arglib.h"
#include "globals.h"

extern Arduboy arduboy;

#define TOTAL_ROOMS   9
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
//#define sign(v) (v > 0) - (v < 0)


byte getChunkID(const byte * levelarray, const int &rx, const int &ry);
bool getChunkBit(const byte * levelarray, const int &rx, const int &ry);
byte getTileInChunk(const byte * chunkArray, const byte &chunk, const byte &cx, const byte &cy);
int r_to_c(const int x);

int sign(int v)
{
  return (v > 0) - (v < 0);
}

byte getTileID(const byte * levelarray, const int &rx, const int &ry, const int variance = 0)
{
  //Serial.println("get tile id");
  int tx = rx / 64;
  int ty = ry / 64;
  byte chunk = getChunkID(levelarray, tx, ty);
  byte tile = getTileInChunk(chunkSet, chunk, r_to_c(rx), r_to_c(ry));
  
  if (tile == 15) tile += variance % 5;
  return tile;
}

bool getSolid(const byte * levelarray, const int &rx, const int &ry)
{
  return getTileID(levelarray, rx, ry) <= 11;
}

byte getChunkID(const byte * levelarray, const int &lx, const int &ly)
{
  //Serial.println("get chunk id");
  // Not solid
  if (getChunkBit(levelarray, lx, ly))
  {
    byte b = 16;
    //Serial.print("Chunk ID: ");
    //Serial.println(b);
    return b;
  }
  else
  {
    byte b = 0;
    b |= getChunkBit(levelarray, lx + 1, ly);  // right of current
    b |= getChunkBit(levelarray, lx, ly - 1) << 1;  // above
    b |= getChunkBit(levelarray, lx - 1, ly) << 2;  // left
    b |= getChunkBit(levelarray, lx, ly + 1) << 3;  // below

    if (b == 0)
    {
      b |= getChunkBit(levelarray, lx + 1, ly - 1);  // top right
      b |= getChunkBit(levelarray, lx - 1, ly - 1) << 1;  // top left
      b |= getChunkBit(levelarray, lx - 1, ly + 1) << 2;  // left bottom
      b |= getChunkBit(levelarray, lx + 1, ly + 1) << 3;  // below right
      if (b > 0) b += 16;
    }
    return b;
  }
}

bool getChunkBit(const byte * levelarray, const int &lx, const int &ly)
{
  if (lx < 0 || lx >= LEVEL_CHUNK_W
  || ly < 0 || ly >= LEVEL_CHUNK_H)
    return 1;
    
  byte x = lx / 8;    // 8 is number of chunks per byte
  byte y = ly * 2;           // 2 is number of bytes per row
  byte i = lx % 8;

  byte b = (levelarray[x + y] & _BV(i));
  //Serial.print("Chunk Bit: ");
  //Serial.println(b > 0);
  return (b > 0);
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
  nibble = (nibble >> (i * 4)) & 0x0F;
  return nibble;
}

int r_to_c(const int x)
{
  byte xx = ((x % 64) / 16);
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
    int x1 = rooms[i].x + 1;
    int y1 = rooms[i].y + 1;
    int x2 = rooms[i + 1].x + 1;
    int y2 = rooms[i + 1].y + 1;
    int xdir = sign(x2 - x1);
    int ydir = sign(y2 - y1);
    for (int a = x1; a != x2; a += xdir)
    {
        setBitXY(levelarray, a, y1, LEVEL_CHUNK_W, 0);
    }
    for (int b = y1; b != y2; b += ydir)
    {
        setBitXY(levelarray, x2, b, LEVEL_CHUNK_W, 0);
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
            if (rooms[a].x < rooms[b].x && rooms[a].x > 0) --rooms[a].x;
            else if (rooms[a].x >= rooms[b].x && rooms[a].x < LEVEL_CHUNK_W - ROOM_W - 1) ++rooms[a].x;
            // Move away y
            if (rooms[a].y < rooms[b].y && rooms[a].y > 0) --rooms[a].y;
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

  // Object stuff
  player.x = (rooms[0].x + 1) << 6;
  player.y = (rooms[0].y + 1) << 6;

  free(rooms);
}

void drawTiles()
{
  //Serial.println("draw tiles:");
  int cx = (cam.x >> 4) << 4;
  int cy = (cam.y >> 4) << 4;
  for (int x = 0; x <= 128; x += 16)
  {
     for (int y = 0; y <= 64; y += 16)
     {
        int ccx = cx + x;
        int ccy = cy + y;
        randomSeed((cam.x + x)/32 + (cam.y + y)/16);
        byte tile = getTileID(levelArray, cam.x + x, cam.y + y, random(5));
        //Serial.print("tile: ");
        //Serial.println(tile);
        sprites.drawOverwrite(ccx - cam.x, ccy - cam.y, bm_tileset, tile);
     }
  }
}

#endif
