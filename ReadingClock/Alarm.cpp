#include <Arduino.h>
#include "Alarm.h"

struct AlarmState
{
  bool state;
  int duration;
};

static const AlarmState alarmStates[] = { { 1, 250}, { 0, 250 }, { 1, 250 }, { 0, 250 }, { 1, 250 }, { 0, 1000 } };
static const int numAlarmStates = sizeof(alarmStates) / sizeof(alarmStates[0]);

Alarm::Alarm(void (*beeperCallback)(bool))
  : _beeperCallback(beeperCallback), _isOn(false), _currentStateIndex(0)
{
}

void Alarm::Update(uint32_t deltaMilliseconds)
{
  if (!_isOn)
  {
    return;
  }
  
  _millisecondsSinceLastStateChange += deltaMilliseconds;
  
  if (_millisecondsSinceLastStateChange > alarmStates[_currentStateIndex].duration)
  {
    _currentStateIndex = (_currentStateIndex + 1) % numAlarmStates;
    UpdateCurrentState();
  }
}

void Alarm::TurnOn()
{
  SetState(true);
}

void Alarm::TurnOff()
{
  SetState(false);
}

void Alarm::SetState(bool on)
{
  _isOn = on;
  _currentStateIndex = 0;
  UpdateCurrentState();
}

bool Alarm::IsOn()
{
  return _isOn;
}

void Alarm::UpdateCurrentState()
{
  AlarmState alarmState = alarmStates[_currentStateIndex];
  (*_beeperCallback)(_isOn && alarmState.state);
  _millisecondsSinceLastStateChange = 0;
}

