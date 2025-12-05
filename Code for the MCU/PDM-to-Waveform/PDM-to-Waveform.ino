#include <Wire.h>       // I2C for OLED
#include <U8g2lib.h>    // OLED library
#include <PDM.h>        // MEMS microphone library

// Initialize OLED for I2C
// Using hardware I2C on ESP32-C3: SDA=IO8, SCL=IO9
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /*SDA=*/ 8, /*SCL=*/ 9);

// Audio buffer
#define AUDIO_BUFFER_SIZE 256
int16_t audioBuffer[AUDIO_BUFFER_SIZE];
volatile int bufferIndex = 0;

// PDM pins
#define PDM_DATA_PIN 0
#define PDM_CLOCK_PIN 1

void setup() {
    Serial.begin(115200);

    // Initialize OLED
    u8g2.begin();
    u8g2.clearBuffer();

    // Initialize PDM microphone
    if (!PDM.begin(1, 16000)) { // 1 channel, 16kHz sample rate
        Serial.println("Failed to start PDM microphone!");
        while (1);
    }

    // Set the PDM data/clock pins explicitly
    PDM.setPin(PDM_DATA_PIN, PDM_CLOCK_PIN);

    // Attach callback
    PDM.onReceive(onPDMdata);
}

// Callback when PDM data is available
void onPDMdata() {
    int bytesAvailable = PDM.available();
    if (bytesAvailable > 0) {
        // Read PDM data as signed 16-bit PCM
        PDM.read(audioBuffer, bytesAvailable * sizeof(int16_t));
        bufferIndex = bytesAvailable;
    }
}

void loop() {
    if (bufferIndex > 0) {
        u8g2.clearBuffer();

        // Draw waveform
        for (int i = 1; i < bufferIndex; i++) {
            int x0 = map(i - 1, 0, bufferIndex, 0, 127);
            int y0 = map(audioBuffer[i - 1], -32768, 32767, 63, 0);
            int x1 = map(i, 0, bufferIndex, 0, 127);
            int y1 = map(audioBuffer[i], -32768, 32767, 63, 0);
            u8g2.drawLine(x0, y0, x1, y1);
        }

        u8g2.sendBuffer();
        bufferIndex = 0;
    }
}

