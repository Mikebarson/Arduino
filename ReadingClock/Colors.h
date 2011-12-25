#ifndef _colors_h
#define _colors_h

class Color
{
  public:
    Color();
    Color(byte red, byte green, byte blue, const char * name);
    
    byte red, green, blue;
    const char * name;
};

class Colors
{
  public:
    static int NumColors();
    static Color GetColor(int i);
    
    static int GetColorIndex(byte red, byte green, byte blue);
};

#endif // _colors_h
