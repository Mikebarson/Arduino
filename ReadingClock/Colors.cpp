#include <Arduino.h>
#include "Colors.h"

Color::Color() { }

Color::Color(byte red, byte green, byte blue, const char *name)
  : red(red), green(green), blue(blue), name(name)
{
}

static Color allColors[] =
{
  Color(0xFF, 0xFF, 0xFF, "White"),
  Color(0xFF, 0, 0, "Red"),
  Color(0xFF, 0xFF, 0, "Yellow"),
  Color(0, 0xFF, 0, "Green"),
  Color(0, 0xFF, 0xFF, "Cyan"),
  Color(0, 0, 0xFF, "Blue"),
  Color(0xFF, 0, 0xFF, "Purple"),
};

int Colors::NumColors()
{
  return sizeof(allColors) / sizeof(*allColors);
}

Color Colors::GetColor(int i)
{
  return allColors[i];
}

int Colors::GetColorIndex(byte red, byte green, byte blue)
{
  Color color;
  for (int i = 0; i < NumColors(); ++i)
  {
    color = allColors[i];
    if (color.red == red &&
        color.green == green &&
        color.blue == blue)
    {
      return i;
    }
  }
  
  return -1;
}

