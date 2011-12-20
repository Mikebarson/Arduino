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

