#ifndef _encoder_h
#define _encoder_h

class Encoder
{
  public:
    Encoder(int pinA, int pinB);
    
    void Init();
    int ReadDelta();
    
  private:
    int pinA, pinB;
    volatile byte state;
};

#endif // _encoder_h

