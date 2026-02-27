#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

// -------- LED SETTINGS --------
#define LED_PIN 2
#define NUM_LEDS 30
#define BRIGHTNESS 80
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

int currentMode = 0;
unsigned long beatTimer = 0;
int beatStep = 0;

// -------- WIFI SETTINGS --------
const char* WIFI_SSID = "Pixel_4593";
const char* WIFI_PASS = "jgmtwfKO#25";

WebServer server(80);


// ----------- HTML PAGE -----------------
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>DJ Beat Pad</title>

<style>
body {
  margin: 0;
  background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
  font-family: 'Segoe UI', sans-serif;
  color: white;
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
}

/* Main App Card */
.app-container {
  background: rgba(255,255,255,0.05);
  backdrop-filter: blur(15px);
  border-radius: 25px;
  padding: 30px;
  width: 420px;
  box-shadow: 0 0 40px rgba(0,0,0,0.6);
  text-align: center;
}

h2 {
  margin-bottom: 25px;
  font-weight: 600;
  letter-spacing: 1px;
}

/* Grid */
.pad-container {
  display: grid;
  grid-template-columns: repeat(3, 110px);
  gap: 20px;
  justify-content: center;
}

/* Pads */
.pad {
  width: 110px;
  height: 110px;
  border-radius: 20px;
  border: none;
  font-weight: bold;
  font-size: 15px;
  cursor: pointer;
  transition: all 0.2s ease;
  box-shadow: 0 8px 20px rgba(0,0,0,0.4);
}

.pad:hover {
  transform: translateY(-5px) scale(1.05);
  box-shadow: 0 0 20px #39ff14;
}

.pad:active {
  transform: scale(0.95);
}

/* Presets */
.preset-container {
  margin-top: 35px;
}

.preset {
  width: 100%;
  margin-top: 10px;
  padding: 12px;
  border-radius: 15px;
  border: none;
  font-weight: bold;
  cursor: pointer;
  background: rgba(255,255,255,0.08);
  color: white;
  transition: 0.2s ease;
}

.preset:hover {
  background: rgba(255,255,255,0.15);
  box-shadow: 0 0 15px #00ffff;
}

.preset.active {
  background: #ff00ff;
  box-shadow: 0 0 20px #ff00ff;
}

/* Colors */
.red{background:#ff4d4d;}
.blue{background:#4da6ff;}
.green{background:#4dff88;}
.yellow{background:#ffcc00;}
.purple{background:#cc66ff;}
.orange{background:#ff884d;}

.house{background:#ff0066;}
.trap{background:#00ccff;}
.techno{background:#00ff99;}

</style>
</head>

<body>

<div class="app-container">

<h2>🎮 DJ Beat Pad</h2>

<div class="pad-container">
  <button class="pad red" onclick="playPad(1)">Kick</button>
  <button class="pad blue" onclick="playPad(2)">Snare</button>
  <button class="pad green" onclick="playPad(3)">HiHat</button>
  <button class="pad yellow" onclick="playPad(4)">Clap</button>
  <button class="pad purple" onclick="playPad(5)">Drop</button>
  <button class="pad orange" onclick="playPad(6)">Bass</button>
</div>

<div class="preset-container">
  <button class="preset house" onclick="setMode(1)">Disco</button>
  <button class="preset trap" onclick="setMode(2)">Trap</button>
  <button class="preset techno" onclick="setMode(3)">Techno</button>
  <button class="preset" style="background:#ff2e2e;" onclick="setMode(0)">Stop</button>
</div>

</div>

<script>

// ---- SOUND FILES ----
const sounds = {
  1: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Kick.wav"),
  2: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Snare.wav"),
  3: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/HiHat.wav"),
  4: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Clap.wav"),
  5: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Drop.wav"),
  6: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Bass.mp3")
};

const presetSounds = {
  1: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Disco_Mode.mp3"),
  2: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Trap_Mode.wav"),
  3: new Audio("https://cdn.jsdelivr.net/gh/KirstenO09/Beat_Pad_Sounds/Techno_Mode.wav")
};

// ---- PLAY PAD ----
function playPad(pad){
  fetch('/pad?i=' + pad);

  if(sounds[pad]){
    sounds[pad].currentTime = 0;
    sounds[pad].play();
  }
}

// ---- SET MODE ----
function setMode(mode){
  fetch('/mode?m=' + mode);

  Object.values(presetSounds).forEach(sound => {
    sound.pause();
    sound.currentTime = 0;
  });

  document.querySelectorAll('.preset').forEach(btn => {
    btn.classList.remove('active');
  });

  if(mode !== 0 && presetSounds[mode]){
    presetSounds[mode].loop = true;
    presetSounds[mode].play();
  }

}

</script>

</body>
</html>
)rawliteral";


// -------- RIPPLE SETTINGS --------
unsigned long previousMillis = 0;
const int interval = 40;   

bool rippleActive = false;
int rippleCenter = 0;
int rippleStep = 0;
CRGB rippleColor;

// 6 evenly spaced centers for 30 LEDs
int sectionCenters[6] = {2, 7, 12, 17, 22, 27};

void startRipple(int center, CRGB color) {
  rippleCenter = center;
  rippleColor = color;
  rippleStep = 0;
  rippleActive = true;
}

void updateRipple() {
  if (!rippleActive) return;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    
    fadeToBlackBy(leds, NUM_LEDS, 20);  

    int left = rippleCenter - rippleStep;
    int right = rippleCenter + rippleStep;

    if (left >= 0)
      leds[left] = rippleColor;

    if (right < NUM_LEDS)
      leds[right] = rippleColor;

    FastLED.show();

    rippleStep++;

    if (left < 0 && right >= NUM_LEDS) {
      rippleActive = false;
    }
  }
}


// -------- BEAT ENGINE --------
void runBeatEngine() {

  if (currentMode == 0) return;

  unsigned long now = millis();
  //if (now - beatTimer < 300) return;  // tempo speed
  int tempo = 300;  // default speed
  
  if (currentMode == 1) {
  tempo = 250;   // Disco tempo
  }


  if (currentMode == 2) {
  tempo = 230;   // fast strobe trap energy
  }

  if (currentMode == 3) {
    tempo = 250;    // techno faster
  }

  if (now - beatTimer < tempo) return;

  beatTimer = now;

  switch(currentMode) {

    // -------- HOUSE (Disco Congo Line) --------
    case 1:
    {
      static int offset = 0;

      CRGB discoColors[4] = {
        CRGB::Red,
        CRGB::Blue,
        CRGB::Green,
        CRGB::Purple
      };

      for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = discoColors[(i + offset) % 4];
      }

      FastLED.show();

      offset++;
      if(offset >= 4) offset = 0;

      return;  // skip ripple engine for this mode
    }


    // -------- TRAP (Flash + Beat Synced) --------
    case 2:
    {
      static int step = 0;

      // --- KICK (Bass Ripple Pulse) ---
      if (step == 0 || step == 4) {
        startRipple(NUM_LEDS / 2, CRGB::Red);
      }

      // --- SNARE (Full Strip Flash) ---
      else if (step == 2 || step == 6) {
        fill_solid(leds, NUM_LEDS, CHSV(random(0,255), 255, 255));
        //fill_solid(leds, NUM_LEDS, CRGB::White);
        FastLED.show();
      }

      // --- HI-HATS (Quick Sparkle) ---
      else {
        for (int i = 0; i < NUM_LEDS; i++) {
          if (random(0,100) < 40) {
            leds[i] = CRGB::White;
          }
        }
        FastLED.show();
      }

      // --- HI-HAT ROLL BURST ---
      if (step == 5) {
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CHSV(random(0,255), 255, 255);
        }
        FastLED.show();
      }

      step++;
      if (step >= 8) step = 0;

      return;
    }
        

    // -------- TECHNO (120 BPM G-Minor Drive) --------
    case 3:
    {
      static int step = 0;

      // --- Kick (Deep Purple Ripple Center) ---
      if (step % 2 == 0) {
        startRipple(NUM_LEDS / 2, CRGB(120, 0, 255));
      }

      // --- Off-beat Hi-Hat Spark ---
      else {
        for (int i = 0; i < NUM_LEDS; i++) {
          if (random(0,100) < 20) {
            leds[i] = CRGB(0,255,180);
          }
        }
        FastLED.show();
      }

      // --- End-of-bar Accent ---
      if (step == 7) {
        fill_solid(leds, NUM_LEDS, CHSV(160,255,255));
        FastLED.show();
      }

      step++;
      if (step >= 8) step = 0;

      return;
    }
  }
}


// -------- HANDLE PAD REQUEST --------
void handlePad() {

  int pad = server.arg("i").toInt();
  CRGB color = CRGB::White;

  switch(pad) {
    case 1: color = CRGB::Red; break;       
    case 2: color = CRGB::Blue; break;      
    case 3: color = CRGB::Green; break;     
    case 4: color = CRGB::Yellow; break;    
    case 5: color = CRGB(150,0,255); break; 
    case 6: color = CRGB(255,100,0); break; 
  }

  // Start ripple in its section
  if (pad >= 1 && pad <= 6) {
    startRipple(sectionCenters[pad - 1], color);
  }

  server.send(200, "text/plain", "OK");
}


void handleMode() {
  currentMode = server.arg("m").toInt();
  beatStep = 0;
  server.send(200, "text/plain", "OK");
}

void handleRoot() {
  server.send_P(200, "text/html", webpage);
}


// -------- SETUP --------
void setup() {

  Serial.begin(115200);

  // ---- LED INITIALIZATION (keep yours exactly as in sketch) ----
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(80);
  FastLED.clear();
  FastLED.show();

  // ---- WIFI CONNECTION ----
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ---- START WEB SERVER ----
  server.on("/", handleRoot);
  server.on("/pad", handlePad);
  server.on("/mode", handleMode);
  server.begin();
}

// -------- LOOP --------
void loop() {
  server.handleClient();
  updateRipple();   
  runBeatEngine();
}