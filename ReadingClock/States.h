#ifndef _states_h
#define _states_h

class States
{
  public:
    enum Values
    {
      idle,
      timerRunning,
      timerRunningAndExpired,
      timerPaused,
      timerStopped,
      menu,
    };
};

#endif // _states_h

