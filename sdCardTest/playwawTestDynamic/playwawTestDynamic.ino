#include <SimpleSDAudio.h>

#define BUTTON_PIN_PAUSE 2      // Define the pin for the pause button
#define BUTTON_PIN_NEXT 3        // Define the pin for the next button
#define DEBOUNCE_DELAY 50        // Debounce delay in milliseconds

#define MAX_FILES 3             // Maximum number of files to hold at a time
#define MAX_FILENAME_LENGTH 13    // Maximum length for each file name

char audioFiles[MAX_FILES][MAX_FILENAME_LENGTH]; // 2D array to hold the audio file names
int audioFileCount = 0;      // Number of files found
int currentFileIndex = 0;    // Index of the currently playing file
bool isPaused = false;

// Global Variables For Button Reading & Debouncing
int btnState = LOW;
int lastBtnState = LOW;
int lastDebounceTime = 0;

int btnState2 = LOW;
int lastBtnState2 = LOW;
int lastDebounceTime2 = 0;

char lastLoadedFile[MAX_FILENAME_LENGTH]; // Store the last loaded file
bool foundLastFile = false;   // Flag to check if the last file is found

void setup() {
  Serial.begin(9600);
  Serial.println("1. Initializing...");

  // Set up button pins
  pinMode(BUTTON_PIN_PAUSE, INPUT);
  pinMode(BUTTON_PIN_NEXT, INPUT);
  
  SdPlay.setSDCSPin(10);
  Serial.println("2. Setting up audio...");

  if (!SdPlay.init(SSDA_MODE_HALFRATE | SSDA_MODE_MONO | SSDA_MODE_AUTOWORKER)) {
    Serial.println("Failed to initialize audio!");
    Serial.println(SdPlay.getLastError());
    while(1);
  }

  loadAudioFiles(); // Load initial audio files
}

void loop() {
  handleButtonPause();
  handleButtonNext();

  // Play next file if not paused and current file has stopped
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
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
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
  if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
    if (reading != btnState2) {
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
    SdPlay.play();  // Resume playback
  }
}

void playNextFile() {
  if (audioFileCount > 0) {
    // Ensure the index wraps around correctly
    // Load new files after reaching the end of the current list
    if (currentFileIndex >= audioFileCount) {
      loadNewFilesAfterLast(); // Load new files after last played
      if (audioFileCount > 0) { // Check if we have new files loaded
        currentFileIndex = 0; // Start from the beginning of the new files
      }
    }
    currentFileIndex %= audioFileCount;

    Serial.print("Playing: ");
    Serial.println(audioFiles[currentFileIndex]);

    if (!SdPlay.setFile(audioFiles[currentFileIndex])) {
      Serial.println("Failed to open audio file!");
      return;
    }

    SdPlay.play();
    isPaused = false;  // Reset pause state for new file

    currentFileIndex++; // Increment the file index for the next song

    
  } else {
    Serial.println("No audio files available to play!");
  }
}

void loadAudioFiles() {
  audioFileCount = 0; // Reset file count before loading new files
  Serial.println("Scanning for audio files...");

  // Scan the directory for audio files
  SdPlay.dir(&dir_callback);

  if (audioFileCount == 0) {
    Serial.println("No audio files found!");
  } else {
    // Update the last loaded file after scanning
    updateLastLoadedFile();
  }
}

void loadNewFilesAfterLast() {
  Serial.println("Loading new audio files after last loaded file...");

  // Reset audioFileCount to allow new files to be loaded
  int previousCount = audioFileCount;
  audioFileCount = 0;  // Reset the count to load new files

  // Scan for new files after the last loaded file
  foundLastFile = false; // Reset the flag before scanning
  SdPlay.dir(dir_callbackAfterLast); // Use the new callback function
  
  // If we loaded new files, store the last loaded file
  if (audioFileCount > previousCount) {
    updateLastLoadedFile();
  } else {
    Serial.println("No new audio files found after last loaded file!");
  }
}

void updateLastLoadedFile() {
  // Update the last loaded file only if we have files to update
  if (audioFileCount > 0) {
    strncpy(lastLoadedFile, audioFiles[audioFileCount - 1], MAX_FILENAME_LENGTH - 1);
    lastLoadedFile[MAX_FILENAME_LENGTH - 1] = '\0'; // Null-terminate the string
    Serial.print("Last loaded file updated to: ");
    Serial.println(lastLoadedFile);
  }
}

void dir_callback(char *buf) {
  int len = strlen(buf);
  if (len > 4 && 
      (strcasecmp(buf + len - 4, ".mp3") == 0 || 
       strcasecmp(buf + len - 4, ".wav") == 0 || 
       strcasecmp(buf + len - 4, ".ahm") == 0)) {

    // Check if the file is already present in the audioFiles array
    bool alreadyExists = false;
    for (int i = 0; i < audioFileCount; i++) {
      if (strcasecmp(audioFiles[i], buf) == 0) {
        alreadyExists = true; // File already exists
        Serial.print("File already exists: ");
        Serial.println(buf);
        break;
      }
    }

    // If the file does not already exist and audio file count is less than max files, add the file
    if (!alreadyExists && audioFileCount < MAX_FILES) {
      strncpy(audioFiles[audioFileCount], buf, MAX_FILENAME_LENGTH - 1);
      audioFiles[audioFileCount][MAX_FILENAME_LENGTH - 1] = '\0'; // Null-terminate the string
      audioFileCount++;
      Serial.print("Added audio file: ");
      Serial.println(buf);

      // Update the last loaded file whenever a new one is added
      updateLastLoadedFile();
    } else if (alreadyExists) {
      Serial.print("File already exists, not adding: ");
      Serial.println(buf);
    } else {
      Serial.println("Maximum number of audio files reached!");
      Serial.println(buf);
    }
  }
}

void dir_callbackAfterLast(char *buf) {
  // If we find the last loaded file, mark it
  if (strcasecmp(buf, lastLoadedFile) == 0) {
    foundLastFile = true; // Last loaded file found
    return; // Skip the last loaded file
  }

  // Only call dir_callback for new files if the last loaded file has been found
  if (foundLastFile) {
    dir_callback(buf); // Call the original callback for new files only
  }
}
