

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Preferences.h>
#include <time.h>
#include <sys/time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>


const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* TZ_IST    = "IST-5:30";          

const uint8_t  DEFAULT_ALARM_H = 7;         
const uint8_t  DEFAULT_ALARM_M = 30;
const uint8_t  EVENING_HOUR    = 21;         
const uint32_t RITUAL_MS       = 30000;      
const uint32_t HOLD_MS         = 2000;
const uint32_t SNOOZE_SEC      = 300;        
const uint32_t RING_MAX_MS     = 120000;     
const uint8_t  MAX_AUTO_RETRIES = 3;         
const uint8_t  TFT_ROTATION    = 1;          // use 3


const uint8_t PIN_SW1 = D0;
const uint8_t PIN_SW2 = D1;
const uint8_t PIN_SW3 = D2;
const uint8_t PIN_SW4 = D3;
const uint8_t PIN_DC  = D4;
const uint8_t PIN_CS  = D5;
const uint8_t PIN_BL  = D6;
const uint8_t PIN_BUZ = D7;
const uint8_t PIN_RST = D8;
const uint8_t PIN_SCK = D9;
const uint8_t PIN_MOSI = D10;


const char* const AFF[] = {
  "I am calm, capable, and ready.",
  "I deserve good things today.",
  "My effort matters, even when it is small.",
  "I trust myself to figure it out.",
  "I am growing, one day at a time.",
  "I choose progress over perfection.",
  "I am enough exactly as I am.",
  "My mind is clear and my heart is steady.",
  "I welcome today with an open mind.",
  "I handle challenges with grace.",
  "I am worthy of rest and of success.",
  "Good things are on their way to me.",
  "I am focused and I finish what I start.",
  "I am proud of how far I have come.",
  "I release what I cannot control.",
  "My energy is steady and strong.",
  "I speak to myself with kindness.",
  "I am brave enough to begin.",
  "Every day I become more confident.",
  "I attract people who lift me up.",
  "I make time for what matters.",
  "I am allowed to take up space.",
  "My best is good enough today.",
  "I learn something from every day.",
  "I am patient with my own pace.",
  "I have everything I need to start.",
  "I bring warmth to everything I do.",
  "I am becoming who I want to be.",
  "Today is full of quiet possibilities.",
  "I am grateful for this fresh start."
};
const char* const MAN[] = {
  "Today I choose calm over worry.",
  "Today I take one small step first.",
  "Today I do my best work with a clear mind.",
  "Today I choose to be kind to myself.",
  "Today I welcome good news and new ideas.",
  "Today I choose focus over distraction.",
  "Today I notice what is going well.",
  "Today I choose courage over comfort.",
  "Today I attract opportunities that fit me.",
  "Today I finish what matters most.",
  "Today I move my body and feel strong.",
  "Today I find joy in ordinary moments.",
  "Today I speak up with confidence.",
  "Today I choose patience when things slow down.",
  "Today I make room for what I love.",
  "Today I trust the process.",
  "Today I learn and grow a little.",
  "Today I choose to be present.",
  "Today I celebrate small wins.",
  "Today I choose peace and let the rest go."
};
const char* const EVE_AFF[] = {
  "You did enough today.",
  "I let go of today and welcome rest.",
  "I am proud of what I handled today.",
  "Rest is part of my progress.",
  "I forgive myself for what went unfinished.",
  "My day is complete. I can breathe out.",
  "I am safe, calm, and ready to rest.",
  "I did my best with what I had today.",
  "I am grateful for the good in today.",
  "I release the worries I carried today.",
  "I deserve a peaceful night.",
  "Tomorrow is a fresh page for me."
};
const char* const EVE_MAN[] = {
  "Tonight I choose deep, restful sleep.",
  "Tomorrow I wake up refreshed and clear.",
  "Tonight I let my mind quiet down.",
  "Tomorrow I welcome new possibilities.",
  "Tonight I think of one good thing from today.",
  "Tomorrow I take it one step at a time.",
  "Tonight I choose peace.",
  "Tomorrow I begin with calm.",
  "Tonight I rest so tomorrow I can shine.",
  "Tomorrow I bring my best energy."
};
#define COUNT(a) ((uint32_t)(sizeof(a) / sizeof((a)[0])))


#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
const uint16_t C_BG     = RGB565(10, 10, 28);
const uint16_t C_TIME   = RGB565(255, 214, 224);
const uint16_t C_DATE   = RGB565(180, 190, 235);
const uint16_t C_AFF    = RGB565(255, 236, 153);
const uint16_t C_MAN    = RGB565(170, 230, 210);
const uint16_t C_DIM    = RGB565(125, 125, 160);
const uint16_t C_ACCENT = RGB565(255, 150, 175);
const uint16_t C_WHITE  = RGB565(255, 255, 255);

#define SCR_W 320
#define SCR_H 240


const uint8_t ST_CLOCK = 0, ST_RINGING = 1, ST_RITUAL = 2, ST_SET_ALARM = 3, ST_SET_CLOCK = 4;
const uint8_t RK_MORNING = 0, RK_EVENING = 1, RK_MANUAL = 2;
const uint8_t EV_NONE = 0, EV_TAP = 1, EV_HOLD = 2;

Adafruit_ILI9341 tft(&SPI, PIN_DC, PIN_CS, PIN_RST);
Preferences prefs;

uint8_t  state = ST_CLOCK;
uint8_t  ritualKind = RK_MORNING;
uint8_t  alarmH = DEFAULT_ALARM_H, alarmM = DEFAULT_ALARM_M;
bool     alarmOn = true;
bool     nightMode = false;
bool     needRedraw = true;
int      lastMin = -1;
int      browseOffset = 0;
uint32_t firedKey = 0, eveningDay = 0;
time_t   snoozeAt = 0;
uint8_t  autoRetries = 0;
uint32_t ringStart = 0, ritualUntil = 0, lockUntil = 0, lastInput = 0;
int      ringLevel = -1;


int editH, editM, setField;
bool editOn;
int scH, scM, scD, scMo, scY, scF;



void pwmAttach(uint8_t pin, uint8_t ch, uint32_t freq, uint8_t bits) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(pin, freq, bits);
#else
  ledcSetup(ch, freq, bits);
  ledcAttachPin(pin, ch);
#endif
}
void setBacklight(uint8_t v) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_BL, v);
#else
  ledcWrite(0, v);
#endif
}
void toneSet(uint16_t f) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWriteTone(PIN_BUZ, f);
#else
  ledcWriteTone(2, f);    
#endif
}

struct Note { uint16_t f; uint16_t ms; };
const Note M_SOFT[]  = {{523,140},{0,80},{659,140},{0,1400}};
const Note M_MED[]   = {{523,150},{0,60},{659,150},{0,60},{784,220},{0,900}};
const Note M_LOUD[]  = {{2093,120},{0,50},{2349,120},{0,50},{2637,200},{0,300}};
const Note M_CHIME[] = {{659,350},{0,300},{880,550},{0,200}};
const Note M_BEEP[]  = {{880,90},{0,10}};

const Note* mel = nullptr;
uint8_t melN = 0, melI = 0;
bool melLoop = false, melPlaying = false;
uint32_t melNext = 0;

void startMelody(const Note* seq, uint8_t n, bool loop) {
  mel = seq; melN = n; melI = 0; melLoop = loop; melPlaying = true; melNext = 0;
}
void stopMelody() { melPlaying = false; toneSet(0); }
void updateMelody() {
  if (!melPlaying || millis() < melNext) return;
  if (melI >= melN) {
    if (melLoop) melI = 0; else { stopMelody(); return; }
  }
  toneSet(mel[melI].f);
  melNext = millis() + mel[melI].ms;
  melI++;
}


struct Btn {
  uint8_t pin; bool raw, down, holdDone, repDone; uint32_t chg, t0, lastRep;
  explicit Btn(uint8_t p) : pin(p), raw(false), down(false), holdDone(false),
                            repDone(false), chg(0), t0(0), lastRep(0) {}
};
Btn btn[4] = { Btn(PIN_SW1), Btn(PIN_SW2), Btn(PIN_SW3), Btn(PIN_SW4) };
uint8_t ev[4];

void pollButtons(bool repeatOn) {
  uint32_t now = millis();
  for (int i = 0; i < 4; i++) {
    Btn &b = btn[i];
    ev[i] = EV_NONE;
    bool r = (digitalRead(b.pin) == LOW);
    if (r != b.raw) { b.raw = r; b.chg = now; }
    if (now - b.chg > 25 && r != b.down) {            // debounced change
      b.down = r;
      if (r) { b.t0 = now; b.holdDone = false; b.repDone = false; b.lastRep = now; }
      else if (!b.holdDone && !b.repDone) ev[i] = EV_TAP;   // tap fires on release
    }
    if (b.down) {
      if (!b.holdDone && !b.repDone && now - b.t0 >= HOLD_MS) { b.holdDone = true; ev[i] = EV_HOLD; }
      if (repeatOn && i >= 2 && !b.holdDone && now - b.t0 >= 500 && now - b.lastRep >= 120) {
        b.repDone = true; b.lastRep = now; ev[i] = EV_TAP;   // auto-repeat for +/-
      }
    }
  }
}


bool timeIsValid() { return time(nullptr) > 1700000000; }
uint32_t dayNum() { return (uint32_t)((time(nullptr) + 19800) / 86400); }   // IST day count
void fmt12(char* b, size_t n, int h, int m) {
  int h12 = h % 12; if (!h12) h12 = 12;
  snprintf(b, n, "%d:%02d %s", h12, m, h < 12 ? "AM" : "PM");
}


void centerText(const char* s, int y, uint8_t size, uint16_t col) {
  tft.setTextSize(size);
  tft.setTextColor(col);
  int w = strlen(s) * 6 * size;
  tft.setCursor((SCR_W - w) / 2, y);
  tft.print(s);
}

int drawWrapped(const char* text, int y, uint8_t size, uint16_t col) {
  const int cw = 6 * size, lh = 9 * size;
  const int maxC = (SCR_W - 20) / cw;
  String line, word;
  tft.setTextSize(size);
  tft.setTextColor(col);
  auto flush = [&]() {
    int x = (SCR_W - (int)line.length() * cw) / 2;
    tft.setCursor(x, y);
    tft.print(line);
    y += lh;
    line = "";
  };
  for (const char* p = text;; p++) {
    char c = *p;
    if (c == ' ' || c == 0) {
      if (word.length()) {
        if (line.length() && (int)(line.length() + 1 + word.length()) > maxC) flush();
        if (line.length()) line += ' ';
        line += word;
        word = "";
      }
      if (c == 0) break;
    } else {
      word += c;
    }
  }
  if (line.length()) flush();
  return y;
}

void drawField(int x, int y, uint8_t size, const char* s, bool active) {
  tft.setTextSize(size);
  tft.setTextColor(active ? C_ACCENT : C_WHITE);
  tft.setCursor(x, y);
  tft.print(s);
  if (active) tft.fillRect(x, y + 8 * size + 3, strlen(s) * 6 * size, 3, C_ACCENT);
}

void applyBacklight() {
  setBacklight((state == ST_CLOCK && nightMode) ? 20 : 255);
}
void setState(uint8_t s) { state = s; applyBacklight(); }
void returnToClock() {
  stopMelody();
  setState(ST_CLOCK);
  needRedraw = true;
  lastMin = -1;
}


void drawClock(const tm& t) {
  tft.fillScreen(C_BG);
  char buf[32];
  strftime(buf, sizeof buf, "%a, %d %b %Y", &t);
  centerText(buf, 12, 2, C_DATE);

  int h12 = t.tm_hour % 12; if (!h12) h12 = 12;
  snprintf(buf, sizeof buf, "%d:%02d", h12, t.tm_min);
  const int size = 8;
  int w = strlen(buf) * 6 * size;
  int total = w + 6 + 36;
  int x0 = (SCR_W - total) / 2;
  tft.setTextSize(size);
  tft.setTextColor(C_TIME);
  tft.setCursor(x0, 44);
  tft.print(buf);
  tft.setTextSize(3);
  tft.setTextColor(C_ACCENT);
  tft.setCursor(x0 + w + 6, 84);
  tft.print(t.tm_hour < 12 ? "AM" : "PM");

  tft.drawFastHLine(40, 122, 240, C_DIM);

  uint32_t dn = dayNum();
  bool evening = (t.tm_hour >= EVENING_HOUR || t.tm_hour < 4);
  const char* line = evening ? EVE_AFF[dn % COUNT(EVE_AFF)] : AFF[dn % COUNT(AFF)];
  drawWrapped(line, 134, 2, C_AFF);

  char ab[28];
  if (alarmOn) {
    char a12[12];
    fmt12(a12, sizeof a12, alarmH, alarmM);
    snprintf(ab, sizeof ab, "Alarm %s", a12);
  } else {
    strcpy(ab, "Alarm off");
  }
  centerText(ab, 214, 2, C_DIM);
}

void drawRinging() {
  tft.fillScreen(C_BG);
  centerText("WAKE UP", 24, 4, C_ACCENT);
  char a12[12];
  fmt12(a12, sizeof a12, alarmH, alarmM);
  centerText(a12, 84, 5, C_TIME);
  centerText("Tap: snooze 5 min", 160, 2, C_DATE);
  centerText("Hold: stop", 190, 2, C_DATE);
}

void drawRitual(uint8_t kind, int off) {
  uint32_t dn = dayNum();
  const char *aff, *man, *head, *l1, *l2;
  if (kind == RK_EVENING) {
    aff = EVE_AFF[dn % COUNT(EVE_AFF)];
    man = EVE_MAN[(dn + 5) % COUNT(EVE_MAN)];
    head = "Good evening";
    l1 = "Tonight's affirmation";
    l2 = "Tonight's manifestation";
  } else {
    aff = AFF[(dn + off) % COUNT(AFF)];
    man = MAN[(dn + 7 + off) % COUNT(MAN)];
    head = (kind == RK_MORNING) ? "Good morning" : "Today's words";
    l1 = "Today's affirmation";
    l2 = "Today's manifestation";
  }
  time_t now = time(nullptr);
  tm t;
  localtime_r(&now, &t);
  char d[32];
  strftime(d, sizeof d, "%A, %d %B", &t);

  tft.fillScreen(C_BG);
  centerText(head, 8, 3, C_ACCENT);
  centerText(d, 38, 2, C_DATE);
  centerText(l1, 66, 1, C_DIM);
  int y = drawWrapped(aff, 78, 2, C_AFF);
  centerText(l2, y + 12, 1, C_DIM);
  drawWrapped(man, y + 24, 2, C_MAN);
}

void drawSetAlarm(bool full) {
  if (full) {
    tft.fillScreen(C_BG);
    centerText("Set alarm", 10, 3, C_ACCENT);
    centerText("SW2 next    SW3 +    SW4 -", 196, 1, C_DIM);
    centerText("Hold SW1 to save", 212, 1, C_DIM);
  }
  tft.fillRect(0, 50, SCR_W, 140, C_BG);
  int h12 = editH % 12; if (!h12) h12 = 12;
  char hh[4], mm[4];
  snprintf(hh, sizeof hh, "%02d", h12);
  snprintf(mm, sizeof mm, "%02d", editM);
  drawField(70, 62, 6, hh, setField == 0);
  tft.setTextSize(6); tft.setTextColor(C_WHITE);
  tft.setCursor(142, 62); tft.print(":");
  drawField(178, 62, 6, mm, setField == 1);
  tft.setTextSize(3); tft.setTextColor(C_DATE);
  tft.setCursor(258, 86); tft.print(editH < 12 ? "AM" : "PM");
  const char* st = editOn ? "ALARM ON" : "ALARM OFF";
  drawField((SCR_W - (int)strlen(st) * 24) / 2, 140, 4, st, setField == 2);
}

void drawSetClock(bool full) {
  if (full) {
    tft.fillScreen(C_BG);
    centerText("Set the time", 10, 3, C_ACCENT);
    centerText("No WiFi time yet. Set it by hand,", 40, 1, C_DIM);
    centerText("or wait: WiFi will fix it when it connects.", 52, 1, C_DIM);
    centerText("SW2 next    SW3 +    SW4 -", 196, 1, C_DIM);
    centerText("Hold SW1 to save", 212, 1, C_DIM);
  }
  tft.fillRect(0, 68, SCR_W, 120, C_BG);
  char b[8];
  snprintf(b, sizeof b, "%02d", scH);  drawField(85, 76, 5, b, scF == 0);
  tft.setTextSize(5); tft.setTextColor(C_WHITE); tft.setCursor(145, 76); tft.print(":");
  snprintf(b, sizeof b, "%02d", scM);  drawField(175, 76, 5, b, scF == 1);
  snprintf(b, sizeof b, "%02d", scD);  drawField(70, 136, 3, b, scF == 2);
  tft.setTextSize(3); tft.setTextColor(C_WHITE); tft.setCursor(106, 136); tft.print("/");
  snprintf(b, sizeof b, "%02d", scMo); drawField(124, 136, 3, b, scF == 3);
  tft.setTextSize(3); tft.setTextColor(C_WHITE); tft.setCursor(160, 136); tft.print("/");
  snprintf(b, sizeof b, "%04d", scY);  drawField(178, 136, 3, b, scF == 4);
}


void startRinging() {
  setState(ST_RINGING);
  ringStart = millis();
  ringLevel = -1;
  drawRinging();
}

void showRitual(uint8_t kind) {
  ritualKind = kind;
  ritualUntil = millis() + RITUAL_MS;
  lockUntil = millis() + 500;                
  setState(ST_RITUAL);
  drawRitual(kind, browseOffset);
  if (kind == RK_EVENING) startMelody(M_CHIME, COUNT(M_CHIME), false);
}

void enterSetAlarm() {
  editH = alarmH; editM = alarmM; editOn = alarmOn; setField = 0;
  lastInput = millis();
  lockUntil = millis() + 300;
  setState(ST_SET_ALARM);
  drawSetAlarm(true);
}

void enterSetClock() {
  const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char mon[4] = {__DATE__[0], __DATE__[1], __DATE__[2], 0};
  const char* p = strstr(months, mon);
  scMo = p ? (int)((p - months) / 3) + 1 : 1;
  scD = atoi(__DATE__ + 4);
  scY = atoi(__DATE__ + 7);
  scH = 12; scM = 0; scF = 0;
  lockUntil = millis() + 300;
  setState(ST_SET_CLOCK);
  drawSetClock(true);
}

void handleClock() {
  if (!timeIsValid()) { enterSetClock(); return; }
  time_t now = time(nullptr);
  tm t;
  localtime_r(&now, &t);

  if (needRedraw || t.tm_min != lastMin) {
    drawClock(t);
    lastMin = t.tm_min;
    needRedraw = false;
  }

  uint32_t dn = dayNum();
  uint32_t key = dn * 1440UL + t.tm_hour * 60 + t.tm_min;
  if (alarmOn && t.tm_hour == alarmH && t.tm_min == alarmM && key != firedKey) {
    firedKey = key; autoRetries = 0; startRinging(); return;
  }
  if (snoozeAt && now >= snoozeAt) { snoozeAt = 0; startRinging(); return; }
  if (t.tm_hour >= EVENING_HOUR && t.tm_hour < EVENING_HOUR + 2 && eveningDay != dn) {
    eveningDay = dn; showRitual(RK_EVENING); return;
  }

  if (ev[0] == EV_TAP)       { browseOffset = 0; showRitual(RK_MANUAL); }
  else if (ev[2] == EV_TAP)  { browseOffset++;   showRitual(RK_MANUAL); }
  else if (ev[1] == EV_HOLD) { enterSetAlarm(); }
  else if (ev[3] == EV_TAP)  { nightMode = !nightMode; applyBacklight(); }
}

void handleRinging() {
  uint32_t el = millis() - ringStart;
  int lvl = el < 20000 ? 0 : (el < 60000 ? 1 : 2);
  if (lvl != ringLevel) {
    ringLevel = lvl;
    if (lvl == 0)      startMelody(M_SOFT, COUNT(M_SOFT), true);
    else if (lvl == 1) startMelody(M_MED,  COUNT(M_MED),  true);
    else               startMelody(M_LOUD, COUNT(M_LOUD), true);
  }
  time_t now = time(nullptr);
  if (ev[0] == EV_TAP) {                       // snooze
    stopMelody();
    snoozeAt = now + SNOOZE_SEC;
    autoRetries = 0;
    returnToClock();
  } else if (ev[0] == EV_HOLD) {               // stop, then show morning words
    stopMelody();
    snoozeAt = 0;
    autoRetries = 0;
    browseOffset = 0;
    showRitual(RK_MORNING);
  } else if (el > RING_MAX_MS) {               // nobody woke up: try again later
    stopMelody();
    if (autoRetries < MAX_AUTO_RETRIES) { autoRetries++; snoozeAt = now + SNOOZE_SEC; }
    returnToClock();
  }
}

void handleRitual() {
  if (millis() > ritualUntil) { returnToClock(); return; }
  if (ritualKind == RK_MANUAL && ev[2] == EV_TAP) {      // SW3 = next affirmation
    browseOffset++;
    ritualUntil = millis() + RITUAL_MS;
    drawRitual(RK_MANUAL, browseOffset);
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (ev[i] != EV_NONE) { returnToClock(); return; }
  }
}

void handleSetAlarm() {
  bool changed = false;
  if (ev[1] == EV_TAP) { setField = (setField + 1) % 3; changed = true; }
  int dir = 0;
  if (ev[2] == EV_TAP) dir = +1;
  if (ev[3] == EV_TAP) dir = -1;
  if (dir) {
    if (setField == 0)      editH = (editH + 24 + dir) % 24;
    else if (setField == 1) editM = (editM + 60 + dir) % 60;
    else                    editOn = !editOn;
    changed = true;
  }
  if (ev[0] == EV_HOLD) {                       // save
    alarmH = editH; alarmM = editM; alarmOn = editOn;
    prefs.putUChar("h", alarmH);
    prefs.putUChar("m", alarmM);
    prefs.putBool("on", alarmOn);
    snoozeAt = 0; firedKey = 0;
    startMelody(M_BEEP, COUNT(M_BEEP), false);
    returnToClock();
    return;
  }
  if (changed) { lastInput = millis(); drawSetAlarm(false); }
  if (millis() - lastInput > 60000) returnToClock();     
}

void handleSetClock() {
  if (timeIsValid()) { returnToClock(); return; }        
  bool changed = false;
  if (ev[1] == EV_TAP) { scF = (scF + 1) % 5; changed = true; }
  int dir = 0;
  if (ev[2] == EV_TAP) dir = +1;
  if (ev[3] == EV_TAP) dir = -1;
  if (dir) {
    switch (scF) {
      case 0: scH  = (scH + 24 + dir) % 24; break;
      case 1: scM  = (scM + 60 + dir) % 60; break;
      case 2: scD  = (scD - 1 + 31 + dir) % 31 + 1; break;
      case 3: scMo = (scMo - 1 + 12 + dir) % 12 + 1; break;
      case 4: scY  = constrain(scY + dir, 2025, 2099); break;
    }
    changed = true;
  }
  if (ev[0] == EV_HOLD) {                                 // save the time
    tm t = {};
    t.tm_year = scY - 1900; t.tm_mon = scMo - 1; t.tm_mday = scD;
    t.tm_hour = scH; t.tm_min = scM; t.tm_sec = 0; t.tm_isdst = 0;
    timeval tv = { mktime(&t), 0 };
    settimeofday(&tv, nullptr);
    startMelody(M_BEEP, COUNT(M_BEEP), false);
    returnToClock();
    return;
  }
  if (changed) drawSetClock(false);
}


void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) pinMode(btn[i].pin, INPUT_PULLUP);

  pwmAttach(PIN_BL, 0, 5000, 8);
  setBacklight(255);
  pwmAttach(PIN_BUZ, 2, 2000, 10);
  toneSet(0);

  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);     
  tft.begin();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(C_BG);
  centerText("Hello :)", 70, 4, C_ACCENT);
  centerText("Connecting to WiFi...", 150, 2, C_DATE);

  prefs.begin("alarm", false);
  alarmH  = prefs.getUChar("h", DEFAULT_ALARM_H);
  alarmM  = prefs.getUChar("m", DEFAULT_ALARM_M);
  alarmOn = prefs.getBool("on", true);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  configTzTime(TZ_IST, "pool.ntp.org", "time.google.com");

  uint32_t t0 = millis();
  while (!timeIsValid() && millis() - t0 < 12000) delay(250);

  if (timeIsValid()) {
    startMelody(M_BEEP, COUNT(M_BEEP), false);
    returnToClock();
  } else {
    enterSetClock();       
  }
}


void loop() {
  pollButtons(state == ST_SET_ALARM || state == ST_SET_CLOCK);
  updateMelody();
  if (millis() < lockUntil) {
    for (int i = 0; i < 4; i++) ev[i] = EV_NONE;
  }
  switch (state) {
    case ST_CLOCK:     handleClock();    break;
    case ST_RINGING:   handleRinging();  break;
    case ST_RITUAL:    handleRitual();   break;
    case ST_SET_ALARM: handleSetAlarm(); break;
    case ST_SET_CLOCK: handleSetClock(); break;
  }
  delay(5);
}
