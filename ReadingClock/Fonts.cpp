#include <Arduino.h>
#include "Fonts.h"

// The way the fonts are defined, you can only include them once in an app, so we encapsulate them.
#include <utility/font_clR4x6.h>
#include <utility/font_clR6x8.h>

#include "globals.h"

void Fonts::SelectFont(Fonts::FontSize fontSize)
{
  switch (fontSize)
  {
    case Regular:
      glcd.setFont(font_clR6x8);
      break;
      
    case Small:
      glcd.setFont(font_clR4x6);
  }
}

