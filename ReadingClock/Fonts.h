#ifndef _fonts_h
#define _fonts_h

class Fonts
{
  public:
    enum FontSize
    {
      Regular,
      Small,
    };
    
    static void SelectFont(FontSize fontSize);
};

#endif // _fonts_h

