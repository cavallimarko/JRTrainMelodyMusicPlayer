#include <SimpleSDAudio.h>

#define MAX_AFM_FILES 14
#define BUTTON_PIN_PAUSE 2  // Define the pin for the pause button
#define BUTTON_PIN_NEXT 3  // Define the pin for the next button
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

char* afmFiles[MAX_AFM_FILES];
int afmFileCount = 0;
int currentFileIndex = 0;
bool isPaused = false;

// Global Variables For Button Reading & Debouncing

int btnState = LOW;
int lastBtnState = LOW;
int lastDebounceTime = 0;

int btnState2 = LOW;
int lastBtnState2 = LOW;
int lastDebounceTime2 = 0;

int debounceDelay = 200;


void setup() {
  Serial.begin(9600);
  Serial.println("1. Initializing...");
  
  // Set up button pin
  pinMode(BUTTON_PIN_PAUSE, INPUT);
  pinMode(BUTTON_PIN_NEXT, INPUT);
  
  SdPlay.setSDCSPin(10);
  Serial.println("2. Setting up audio...");
  
  if (!SdPlay.init(SSDA_MODE_HALFRATE | SSDA_MODE_MONO | SSDA_MODE_AUTOWORKER)) {
    Serial.println("Failed to initialize audio!");
    Serial.println(SdPlay.getLastError());
    while(1);
  }

  Serial.println("Scanning for AFM files...");
  SdPlay.dir(&dir_callback);

  //Serial.println("Found AFM files:");
  //for(int i = 0; i < afmFileCount; i++) {
  //  Serial.print(i + 1);
  //  Serial.print(": ");
  //  Serial.println(afmFiles[i]);
  //}
  
  //if (afmFileCount == 0) {
  //  Serial.println("No AFM files found!");
  //  while(1);
  //}
}

void loop() {
  handleButtonPause();
  handleButtonNext();
  if (!isPaused && SdPlay.isStopped()) {
    playNextFile();
  }
  
  delay(10);  // Small delay to prevent tight looping
}

void handleButtonPause() {
  int reading = digitalRead(BUTTON_PIN_PAUSE);
  if (reading != lastBtnState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != btnState) {
      btnState = reading;
      if (btnState == HIGH) {
        togglePause();
      }
    }
  }
  lastBtnState = reading;
  
}
void handleButtonNext() {
  int reading = digitalRead(BUTTON_PIN_NEXT);
  if (reading != lastBtnState2) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading != btnState) {
      btnState2 = reading;
      if (btnState2 == HIGH) {
        playNextFile();
      }
    }
  }
  lastBtnState2 = reading;
  
}
void togglePause() {

  isPaused = !isPaused;
  if (isPaused) {
    Serial.println("Playback paused");
    SdPlay.pause();
  } else {
    Serial.println("Playback resumed");
    SdPlay.pause();
  }
}

void playNextFile() {
  if (afmFileCount > 0) {
    Serial.print("Playing file ");
    Serial.print(currentFileIndex + 1);
    Serial.print(" of ");
    Serial.print(afmFileCount);
    Serial.print(": ");
    Serial.println(afmFiles[currentFileIndex]);
    
    if (!SdPlay.setFile(afmFiles[currentFileIndex])) {
      Serial.println("Failed to open audio file!");
      return;
    }
    
    SdPlay.play();
    isPaused = false;  // Reset pause state for new file
    
    currentFileIndex = (currentFileIndex + 1) % afmFileCount;
  }
}

void dir_callback(char *buf) {
  int len = strlen(buf);
  if (len > 4 && 
      (strcasecmp(buf + len - 4, ".AFM") == 0 || 
       strcasecmp(buf + len - 4, ".ahm") == 0)) {
    
    if (afmFileCount < MAX_AFM_FILES) {
      char* filename = (char*)malloc(len + 1);
      strcpy(filename, buf);
      afmFiles[afmFileCount++] = filename;
      
      Serial.print("Added AFM file: ");
      Serial.println(buf);
    } else {
      Serial.println("Maximum number of AFM files reached!");
    }
  }
}
