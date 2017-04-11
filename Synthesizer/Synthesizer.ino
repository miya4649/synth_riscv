/*
  Copyright (c) 2017, miya
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Pin
unsigned char LED[] = {6,3,5};
const int AUDIO_L = 9;
const int AUDIO_R = 10;

// 123458,123466
const int RANDOM_SEED = 123466;
const int STATE_ATTACK = 0;
const int STATE_DECAY = 1;
const int STATE_SUSTAIN = 2;
const int STATE_RELEASE = 3;
const int STATE_SLEEP = 4;
const int WAVE_BUFFER_BITS = 8;
const int INT_BITS = 32;
const int FIXED_BITS = 14;
const int FIXED_BITS_ENV = 8;
const int WAVE_ADDR_SHIFT = (INT_BITS - WAVE_BUFFER_BITS);
const int WAVE_ADDR_SHIFT_M = (WAVE_ADDR_SHIFT - FIXED_BITS);
const int FIXED_SCALE = (1 << FIXED_BITS);
const int FIXED_SCALE_M1 = (FIXED_SCALE - 1);
const int WAVE_BUFFER_SIZE = (1 << WAVE_BUFFER_BITS);
const int WAVE_BUFFER_SIZE_M1 = (WAVE_BUFFER_SIZE - 1);
const int OSCS = 4;
const int REVERB_BUFFER_SIZE = 0x1000;
const int TEMPO = (1 << 12);
const int OUT_BITS = 7;
const int OUT_SHIFT = (FIXED_BITS - OUT_BITS);
const int OUT_SHIFT_M1 = (OUT_SHIFT - 1);
const int OUT_OFFSET = (1 << OUT_BITS);
const int SEQ_LENGTH = 16;
const int SAMPLE_US = 40;

const int CONST03 = (1 << FIXED_BITS << FIXED_BITS_ENV);
const int CONST04 = (FIXED_SCALE >> 0);
const int CONST05 = (CONST04 / OSCS);
const int REVERB_VOLUME = (CONST04 >> 2);

typedef struct
{
  int envelopeLevelA;
  int envelopeLevelS;
  int envelopeDiffA;
  int envelopeDiffD;
  int envelopeDiffR;
  int modPatch0;
  int modPatch1;
  int modLevel0;
  int modLevel1;
  int levelL;
  int levelR;
  int levelRev;

  int state;
  int count;
  int currentLevel;
  int pitch;
  int velocity;
  int mod0;
  int mod1;
  int outData;
  int outWaveL;
  int outWaveR;
  int outRevL;
  int outRevR;
  boolean mixOut;
  boolean noteOn;
  boolean noteOnSave;
  boolean goToggle;
  boolean outDoneToggle;
} params_t;

typedef struct
{
  unsigned char note;
  unsigned char oct;
} seqData_t;

short waveData[] = {
  0,402,803,1205,1605,2005,2403,2800,
  3196,3589,3980,4369,4755,5139,5519,5896,
  6269,6639,7004,7365,7722,8075,8422,8764,
  9101,9433,9759,10079,10393,10700,11002,11296,
  11584,11865,12139,12405,12664,12915,13158,13394,
  13621,13841,14052,14254,14448,14633,14810,14977,
  15135,15285,15425,15556,15677,15789,15892,15984,
  16068,16141,16205,16259,16304,16338,16363,16378,
  16383,16378,16363,16338,16304,16259,16205,16141,
  16068,15984,15892,15789,15677,15556,15425,15285,
  15135,14977,14810,14633,14448,14254,14052,13841,
  13621,13394,13158,12915,12664,12405,12139,11865,
  11584,11296,11002,10700,10393,10079,9759,9433,
  9101,8764,8422,8075,7722,7365,7004,6639,
  6269,5896,5519,5139,4755,4369,3980,3589,
  3196,2800,2403,2005,1605,1205,803,402,
  0,-403,-804,-1206,-1606,-2006,-2404,-2801,
  -3197,-3590,-3981,-4370,-4756,-5140,-5520,-5897,
  -6270,-6640,-7005,-7366,-7723,-8076,-8423,-8765,
  -9102,-9434,-9760,-10080,-10394,-10701,-11003,-11297,
  -11585,-11866,-12140,-12406,-12665,-12916,-13159,-13395,
  -13622,-13842,-14053,-14255,-14449,-14634,-14811,-14978,
  -15136,-15286,-15426,-15557,-15678,-15790,-15893,-15985,
  -16069,-16142,-16206,-16260,-16305,-16339,-16364,-16379,
  -16383,-16379,-16364,-16339,-16305,-16260,-16206,-16142,
  -16069,-15985,-15893,-15790,-15678,-15557,-15426,-15286,
  -15136,-14978,-14811,-14634,-14449,-14255,-14053,-13842,
  -13622,-13395,-13159,-12916,-12665,-12406,-12140,-11866,
  -11585,-11297,-11003,-10701,-10394,-10080,-9760,-9434,
  -9102,-8765,-8423,-8076,-7723,-7366,-7005,-6640,
  -6270,-5897,-5520,-5140,-4756,-4370,-3981,-3590,
  -3197,-2801,-2404,-2006,-1606,-1206,-804,-403,
};

int scaleTable[] = {
  153791,162936,172624,182889,193764,205286,217493,230426,
  244128,258644,274024,290319,307582,325872,345249,365779,
};

const int CHORD_LENGTH = 5;
const int CHORDS = 6;
unsigned char chordData[CHORDS][8] = {
  {0,2,4,7,11,0,0,0},
  {2,4,5,7,11,0,0,0},
  {0,4,7,9,11,0,0,0},
  {0,4,5,7,9,0,0,0},
  {0,2,4,5,9,0,0,0},
  {2,5,7,9,11,0,0,0},
};

params_t params[OSCS];
seqData_t seqData[OSCS][SEQ_LENGTH];

signed char reverbBufferL[REVERB_BUFFER_SIZE];
signed char reverbBufferR[REVERB_BUFFER_SIZE];

int32_t timerNext;
int counter;
int seqCounter;
int barCounter;
int deleteCounter;
int chord;
int note;
int sc;
int reverbCounter;
int reverbAddrL;
int reverbAddrR;

void synth()
{
  // patch
  for (int i = 0; i < OSCS; i++)
  {
    params[i].mod0 = params[params[i].modPatch0].outData;
    params[i].mod1 = params[params[i].modPatch1].outData;
  }

  for (int i = 0; i < OSCS; i++)
  {
    // envelope generator
    if ((params[i].noteOn == true) && (params[i].noteOnSave != params[i].noteOn))
    {
      params[i].state = STATE_ATTACK;
    }
    if ((params[i].noteOn == false) && (params[i].noteOnSave != params[i].noteOn))
    {
      params[i].state = STATE_RELEASE;
    }
    params[i].noteOnSave = params[i].noteOn;
  
    if (params[i].state == STATE_SLEEP)
    {
      params[i].count = 0;
    }

    int limitValue = 0;
    int valueDiff = 0;
    boolean limitGt = false;
  
    if (params[i].state == STATE_ATTACK)
    {
      limitValue = params[i].envelopeLevelA;
      valueDiff = params[i].envelopeDiffA;
      limitGt = true;
    }
    else if (params[i].state == STATE_DECAY)
    {
      limitValue = params[i].envelopeLevelS;
      valueDiff = params[i].envelopeDiffD;
      limitGt = false;
    }
    else if (params[i].state == STATE_RELEASE)
    {
      limitValue = 0;
      valueDiff = params[i].envelopeDiffR;
      limitGt = false;
    }
  
    params[i].currentLevel += valueDiff;
  
    if (((limitGt == true) && (params[i].currentLevel > limitValue)) ||
        ((limitGt == false) && (params[i].currentLevel < limitValue)))
    {
      params[i].currentLevel = limitValue;
      params[i].state++;
    }

    int waveAddr = (unsigned int)(params[i].count +
                                  (params[i].mod0 * params[i].modLevel0) +
                                  (params[i].mod1 * params[i].modLevel1)) >> WAVE_ADDR_SHIFT_M;

    // fetch wave data
    int waveAddrF = waveAddr >> FIXED_BITS;
    int waveAddrR = (waveAddrF + 1) & WAVE_BUFFER_SIZE_M1;    
    int oscOutF = waveData[waveAddrF];
    int oscOutR = waveData[waveAddrR];
    int waveAddrM = waveAddr & FIXED_SCALE_M1;
    int oscOut = ((oscOutF * (FIXED_SCALE - waveAddrM)) >> FIXED_BITS) +
      ((oscOutR * waveAddrM) >> FIXED_BITS);
    params[i].outData = (oscOut * (params[i].currentLevel >> FIXED_BITS_ENV)) >> FIXED_BITS;
    params[i].count += params[i].pitch;

    // mix
    if (params[i].mixOut == 0)
    {
      params[i].outWaveL = 0;
      params[i].outWaveR = 0;
      params[i].outRevL = 0;
      params[i].outRevR = 0;
    }
    else
    {
      params[i].outWaveL = (params[i].outData * params[i].levelL) >> FIXED_BITS;
      params[i].outWaveR = (params[i].outData * params[i].levelR) >> FIXED_BITS;
      params[i].outRevL = (params[i].outWaveL * params[i].levelRev) >> FIXED_BITS;
      params[i].outRevR = (params[i].outWaveR * params[i].levelRev) >> FIXED_BITS;
    }
  }
}

void setup()
{
  pinMode(AUDIO_R, OUTPUT);
  pinMode(AUDIO_L, OUTPUT);

  //Serial.begin(115200);

  randomSeed(RANDOM_SEED);

  for (int i = 0; i < OSCS; i++)
  {
    params[i].state = STATE_SLEEP;
    params[i].envelopeLevelA = CONST03;
    params[i].envelopeLevelS = 0;
    params[i].envelopeDiffA = CONST03 >> 3;
    params[i].envelopeDiffD = (0 - CONST03) >> 15;
    params[i].envelopeDiffR = (0 - CONST03) >> 13;
    params[i].levelL = CONST05;
    params[i].levelR = CONST05;
    params[i].levelRev = REVERB_VOLUME;
    params[i].mixOut = true;
    params[i].modPatch0 = i;
    params[i].modPatch1 = i;
    params[i].modLevel0 = FIXED_SCALE * random(4);
    params[i].modLevel1 = 0;
  }
  params[0].levelL = CONST05 >> 1;
  params[0].levelR = CONST05;
  params[1].levelL = CONST05;
  params[1].levelR = CONST05 >> 1;

  for (int i = 0; i < 3; i++)
  {
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], HIGH);
  }

  delay(500);
  counter = 0;
  seqCounter = 0;
  barCounter = 0;
  deleteCounter = 0;
  chord = 0;
  note = 0;
  sc = 0;
  reverbAddrL = 0;
  reverbAddrR = 0;
  reverbCounter = 0;
  timerNext = micros();
}

void loop()
{
  // sequencer
  counter++;
  if (counter > TEMPO)
  {
    counter = 0;
    if (seqCounter >= SEQ_LENGTH)
    {
      seqCounter = 0;
      if ((barCounter & 3) == 0)
      {
        for (int i = 0; i < OSCS; i++)
        {
          params[i].envelopeDiffA = CONST03 >> (random(11) + 2);
          params[i].modLevel0 = FIXED_SCALE * random(4);
        }
        if (chord == 5)
        {
          chord = random(3);
        }
        else
        {
          chord = random(CHORDS);
        }
        deleteCounter = random(16);
      }
      for (int i = 0; i < deleteCounter; i++)
      {
        seqData[random(OSCS)][random(SEQ_LENGTH)].oct = 0;
      }
      for (int i = 0; i < 4; i++)
      {
        int ch = random(OSCS);
        seqData[ch][random(SEQ_LENGTH)].note = random(CHORD_LENGTH);
        seqData[ch][random(SEQ_LENGTH)].oct = random(3) + 6;
      }
      barCounter++;
    }
    for (int i = 0; i < OSCS; i++)
    {
      if (seqData[i][seqCounter].oct != 0)
      {
        params[i].noteOn = true;
        int n = chordData[chord][seqData[i][seqCounter].note];
        params[i].pitch = scaleTable[n] << seqData[i][seqCounter].oct;
      }
      else
      {
        params[i].noteOn = false;
      }
    }
    seqCounter++;
  }

  synth();

  // mixing
  int mixL = 0;
  int mixR = 0;
  int mixRevL = 0;
  int mixRevR = 0;
  for (int i = 0; i < OSCS; i++)
  {
    mixL += params[i].outWaveL;
    mixR += params[i].outWaveR;
    mixRevL += params[i].outRevL;
    mixRevR += params[i].outRevR;
  }

  // reverb
  if ((reverbCounter & 1) == 0)
  {
    int reverbL = (int)reverbBufferR[reverbAddrR] << OUT_SHIFT_M1;
    int reverbR = (int)reverbBufferL[reverbAddrL] << OUT_SHIFT_M1;
    reverbL += mixRevR;
    reverbR += mixRevL;
    reverbL >>= OUT_SHIFT;
    reverbR >>= OUT_SHIFT;
    reverbBufferL[reverbAddrL] = reverbL;
    reverbBufferR[reverbAddrR] = reverbR;
    reverbAddrL++;
    if (reverbAddrL > 0xfff)
    {
      reverbAddrL = 0;
    }
    reverbAddrR++;
    if (reverbAddrR > 0xf51)
    {
      reverbAddrR = 0;
    }
  }
  reverbCounter++;
  int outL = (mixL >> OUT_SHIFT) + reverbBufferL[reverbAddrL] + OUT_OFFSET;
  int outR = (mixR >> OUT_SHIFT) + reverbBufferR[reverbAddrR] + OUT_OFFSET;

  while (true)
  {
    int32_t timer = micros();
    if ((timer - timerNext) > 0)
    {
      timerNext += SAMPLE_US;
      break;
    }
  }

  // output
  analogWrite(AUDIO_L, outL);
  analogWrite(AUDIO_R, outR);
  for (int i = 0; i < 3; i++)
  {
    if (params[i].noteOn)
    {
      digitalWrite(LED[i], LOW);
    }
    else
    {
      digitalWrite(LED[i], HIGH);
    }
  }
}
