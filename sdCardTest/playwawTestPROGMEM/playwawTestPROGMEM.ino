#include <SimpleSDAudio.h>
#include <avr/pgmspace.h> // Include for PROGMEM
#include <SD.h> // Ensure the SD library is included

#define BUTTON_PIN_PAUSE 2
#define BUTTON_PIN_NEXT 3
#define DEBOUNCE_DELAY 50

#define MAX_FILES 5             // Maximum number of files to hold at a time
#define MAX_FILENAME_LENGTH 13   // Maximum length for each file name

// Store the file names in PROGMEM using const char pointers
const char audioFilesPointers[] PROGMEM =
    "Airly.AHM\0Astro_B2.AHM\0Astro_B3.AHM\0Astro_B4.AHM\0Astro_Bo.AHM\0Azamino2.AHM\0Azamino_.AHM\0Babble.AHM\0Babblin2.AHM\0Babbling.AHM\0Beautifu.AHM\0Bell_A.AHM\0Bell_B.AHM\0Cappucci.AHM\0Cielo_Es.AHM\0Commuter.AHM\0Coral_Re.AHM\0Cosmos_V.AHM\0Dance_On.AHM\0DDG_Fin2.AHM\0DDG_Fin3.AHM\0DDG_Fin4.AHM\0DDG_Fina.AHM\0DDG_iOS2.AHM\0DDG_iOS3.AHM\0DDG_iOS4.AHM\0DDG_iOS5.AHM\0DDG_iOS6.AHM\0DDG_iOS7.AHM\0DDG_iOS8.AHM\0DDG_iOS9.AHM\0DDG_iOS_.AHM\0DDG_nDS_.AHM\0DDG_SS_A.AHM\0Distant_.AHM\0Eau_Conr.AHM\0Ebisu.AHM\0Ebisu_MI.AHM\0Ebisu_v2.AHM\0Ebisu_v3.AHM\0Ebisu_ve.AHM\0Farewell.AHM\0First_Sp.AHM\0Flowers_.AHM\0Flower_S.AHM\0Fraying_.AHM\0Gota_del.AHM\0Harajuk2.AHM\0Harajuku.AHM\0Holiday2.AHM\0Holiday_.AHM\0Joban_2.AHM\0Joban_3-.AHM\0Kids_Sta.AHM\0Light_an.AHM\0Maihama2.AHM\0Maihama3.AHM\0Maihama4.AHM\0Maihama5.AHM\0Maihama6.AHM\0Maihama_.AHM\0Marina.AHM\0Mellow_T.AHM\0Melody.AHM\0Moonston.AHM\0Morning_.AHM\0New_Seas.AHM\0Old_Musi.AHM\0Osaka_L2.AHM\0Osaka_L3.AHM\0Osaka_L4.AHM\0Osaka_Lo.AHM\0Railroad.AHM\0Rainy_S2.AHM\0Rainy_St.AHM\0Sakura_2.AHM\0Sakura_3.AHM\0Sakura_4.AHM\0Sakura_5.AHM\0Sakura_6.AHM\0Sakura_S.AHM\0See_You_.AHM\0Seiryu.AHM\0SF-1.AHM\0SF-10-31.AHM\0SF-10-60.AHM\0SF-22-14.AHM\0SF-22-29.AHM\0SF-3.AHM\0SF_10-38.AHM\0SF_10-43.AHM\0SF_22-14.AHM\0SF_22-29.AHM\0SH-1-1.AHM\0SH-1.AHM\0SH-2-1.AHM\0SH-2.AHM\0SH-3-1.AHM\0SH-3.AHM\0SH-4.AHM\0SH-5-1.AHM\0SH-5-2.AHM\0SH-5.AHM\0SH-6.AHM\0SH-7.AHM\0SH-8-1.AHM\0SH-8.AHM\0SH-9-1.AHM\0Spring.AHM\0Spring_2.AHM\0Spring_B.AHM\0Station2.AHM\0Station3.AHM\0Station_.AHM\0Sunny_Is.AHM\0Sunny_S2.AHM\0Sunny_Sp.AHM\0Sunrise.AHM\0Susuki_2.AHM\0Susuki_H.AHM\0Sweet_Ca.AHM\0Syncopat.AHM\0The_Comi.AHM\0The_Othe.AHM\0The_Soun.AHM\0The_Wher.AHM\0Time_of_.AHM\0TR1.AHM\0TR11.AHM\0TR12.AHM\0TR2.AHM\0TR3.AHM\0TR4.AHM\0TR5_Song.AHM\0TR6_Viva.AHM\0TR7_Jesu.AHM\0TR8_Blue.AHM\0Twilight.AHM\0Twinklin.AHM\0Under_th.AHM\0Verde_R2.AHM\0Verde_Ra.AHM\0Waiting_.AHM\0Waking_2.AHM\0Waking_o.AHM\0Walnut_P.AHM\0Waterfro.AHM\0Water_Cr.AHM\0";

// Number of audio files
const int audioFileCount = sizeof(audioFilesPointers) / sizeof(audioFilesPointers[0]); 

int currentFileIndex = 0; // Index of the currently playing file
bool isPaused = false;

// Global Variables For Button Reading & Debouncing
int btnState = LOW;
int lastBtnState = LOW;
int lastDebounceTime = 0;

int btnState2 = LOW;
int lastBtnState2 = LOW;
int lastDebounceTime2 = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("1. Initializing...");

    pinMode(BUTTON_PIN_PAUSE, INPUT);
    pinMode(BUTTON_PIN_NEXT, INPUT);

    SdPlay.setSDCSPin(10);
    Serial.println("2. Setting up audio...");

    if (!SdPlay.init(SSDA_MODE_HALFRATE | SSDA_MODE_MONO | SSDA_MODE_AUTOWORKER)) {
        Serial.println("Failed to initialize audio!");
        Serial.println(SdPlay.getLastError());
        while (1);
    }

    Serial.print("Loading audio files... Found: ");
    Serial.println(audioFileCount); // Print the count of loaded audio files
}

void loop() {
    handleButtonPause();
    handleButtonNext();

    if (!isPaused && SdPlay.isStopped()) {
        playNextFile();
    }

    delay(10);
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
        char buf[MAX_FILENAME_LENGTH]; // Buffer for the filename

        // Read filename from program memory
        // Find the start of the current file
        const char* fileStart = (const char*)pgm_read_word(&(audioFilesPointers[currentFileIndex * (MAX_FILENAME_LENGTH + 1)]));
        strcpy_P(buf, fileStart);
        Serial.print("Playing: ");
        Serial.println(buf);

        if (!SdPlay.setFile(buf)) {
            Serial.println("Failed to open audio file!");
            return;
        }

        SdPlay.play();
        isPaused = false;

        // Increment the current file index and wrap around
        currentFileIndex = (currentFileIndex + 1) % audioFileCount; 
    } else {
        Serial.println("No audio files available to play!");
    }
}
