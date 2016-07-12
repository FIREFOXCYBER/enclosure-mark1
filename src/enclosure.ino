#include "MycroftMouth.h"
#include "MycroftEyes.h"
#include "MycroftWeather.h"

#include "MycroftEncoder.h"
#include "MouthProcessor.h"
#include "EyesProcessor.h"
#include "ArduinoProcessor.h"

#define BUTTON_PIN 2
#define SPEAKER_PIN 4
#define ENC1_PIN 14
#define ENC2_PIN 15

#define EYES_PIN 3
#define EYES_SIZE 24
#define EYES_TYPE NEO_GRB + NEO_KHZ800

#define MOUTH_CS1 7
#define MOUTH_WR 8
#define MOUTH_DATA 9

// Must be initialized first
MycroftEncoder encoder(ENC1_PIN, ENC2_PIN, BUTTON_PIN);
MycroftEyes eyes(EYES_SIZE, EYES_PIN, EYES_TYPE);
MycroftMouth mouth(MOUTH_CS1, MOUTH_WR, MOUTH_DATA);
MycroftWeather weather(&mouth, &eyes);

MouthProcessor mouthProcessor(mouth);
EyesProcessor eyesProcessor(eyes);
ArduinoProcessor arduinoProcessor(SPEAKER_PIN);
BaseProcessor *processors[3] = {&mouthProcessor, &eyesProcessor, &arduinoProcessor};

int16_t time = 1000;

void timerIsr() {
    encoder.isr();
}

void initSerial() {
    Serial.begin(9600);
    while (!Serial);
    Serial.flush();
    Serial.println(F("Mycroft Hardware v0.1.3 - Connected"));
}

void setup() {
    initSerial();
    eyesProcessor.setup();
    arduinoProcessor.setup();
    Timer1.initialize(time);
    Timer1.attachInterrupt(timerIsr);
}

bool contains(String value, String term) {
    return value.indexOf(term) > -1;
}

void processWeather(String cmd){
  if (contains(cmd, "display=")){
      cmd.replace("display=", "");
      int8_t cond = cmd.charAt(0) - '0';
      cmd.remove(0,1);
      weather.display(cond, cmd);
  }
}

void processVolume() {
    MycroftEncoder::Direction d = encoder.getDirection();
    if (d == MycroftEncoder::Direction::RIGHT) {
        Serial.println("volume.up");
    }
    else if (d == MycroftEncoder::Direction::LEFT) {
        Serial.println("volume.down");
    }
}

void processButton() {
    ClickEncoder::Button b = encoder.clickEncoder->getButton();
    if (b != ClickEncoder::Open) {
        switch (b) {
            case ClickEncoder::Pressed:
                break;
            case ClickEncoder::Held:
                break;
            case ClickEncoder::Released:
                break;
            case ClickEncoder::Clicked:
                Serial.println("mycroft.stop");
                break;
            case ClickEncoder::DoubleClicked:
            break;
        }
    }
}

void loop() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        Serial.flush();
        Serial.print(F("Command: "));
        Serial.println(cmd);

        if (contains(cmd, "weather.")) {
            cmd.replace("weather.", "");
            processWeather(cmd);
        }
        else
            for (auto *i:processors)
                if (i->tryProcess(cmd))
                    break;
    }
    while (Serial.available() <= 0) {
        processVolume();
        processButton();
        mouthProcessor.drawAnimation();
    }
}
