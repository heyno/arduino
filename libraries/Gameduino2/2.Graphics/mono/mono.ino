#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

#include "mono_assets.h"

void setup()
{
  GD.begin();
  LOAD_ASSETS();
}

void loop()
{
  GD.ClearColorRGB(0x375e03);
  GD.Clear();
  GD.Begin(BITMAPS);
  GD.ColorRGB(0x68b203);
  GD.BitmapSize(NEAREST, REPEAT, REPEAT, GD.w, GD.h);
  GD.Vertex2f(0, 0);
  GD.swap();
}
