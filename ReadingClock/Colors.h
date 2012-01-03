#ifndef _colors_h
#define _colors_h

class Color
{
  public:
    Color(byte red, byte green, byte blue, PGM_P name);
    
    byte red, green, blue;
    PGM_P name;
};

class Colors
{
  public:
    static int NumColors();
    static Color GetColor(int i);
    
    static int GetColorIndex(byte red, byte green, byte blue);
};

#endif // _colors_h

