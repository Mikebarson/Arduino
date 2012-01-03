#include <Arduino.h>
#include "Colors.h"

// This rigamarole is necessary to get the color name strings int program storage rather than RAM.
class ColorNames
{
  public:
    enum Names
    {
      White,
      Red,
      Yellow,
      Green,
      Cyan,
      Blue,
      Purple,
    };

    static PGM_P GetName(int colorName)
    {
      switch (colorName)
      {
        case White:
          return PSTR("White");
        case Red:
          return PSTR("Red");
        case Yellow:
          return PSTR("Yellow");
        case Green:
          return PSTR("Green");
        case Cyan:
          return PSTR("Cyan");
        case Blue:
          return PSTR("Blue");
        case Purple:
          return PSTR("Purple");
      }
    }
};

static const Color allColors[] =
{
  Color(0xFF, 0xFF, 0xFF, ColorNames::GetName(ColorNames::White)),
  Color(0xFF, 0, 0, ColorNames::GetName(ColorNames::Red)),
  Color(0xFF, 0xFF, 0, ColorNames::GetName(ColorNames::Yellow)),
  Color(0, 0xFF, 0, ColorNames::GetName(ColorNames::Green)),
  Color(0, 0xFF, 0xFF, ColorNames::GetName(ColorNames::Cyan)),
  Color(0, 0, 0xFF, ColorNames::GetName(ColorNames::Blue)),
  Color(0xFF, 0, 0xFF, ColorNames::GetName(ColorNames::Purple)),
};

Color::Color(byte red, byte green, byte blue, PGM_P name)
  : red(red), green(green), blue(blue), name(name)
{
}

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
  for (int i = 0; i < NumColors(); ++i)
  {
    const Color &color = allColors[i];
    if (color.red == red &&
        color.green == green &&
        color.blue == blue)
    {
      return i;
    }
  }
  
  return -1;
}

