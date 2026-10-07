#pragma once

// 2 debounced buttons from a single Analog pin
class ABTN
{
  public:
    enum tButton {eNone, eSet, eSel};

    void Init(int Pin, int ThresholdSet, int ThresholdSel);
    tButton Pressed();
    tButton Down();
    
  private:
    int m_iPin;
    int m_iThresholdSet;
    int m_iThresholdSel;
    int m_iPrevReading;
    int m_iPrevState;
    unsigned long m_iTransitionTimeMS;
};

extern ABTN btns;

