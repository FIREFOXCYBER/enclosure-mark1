#include "Arduino.h"
#include "MycroftMouth.h"
#include "MouthImages.h"
#include "../HT1632/font_5x4.h"

MycroftMouth::MycroftMouth(int pinCS1, int pinWR, int pinDATA) {
    ht1632 = HT1632Class();
    ht1632.begin(pinCS1, pinWR, pinDATA);
    reset();
}

void MycroftMouth::reset() {
    state = NONE;
    width = 8;
    height = 8;
    textWd = 0;
    textIdx = 0;
    ht1632.clear();
    ht1632.render();
}

void MycroftMouth::run() {

    switch (state) {
        case TALK:
            this->talk();
            break;
        case LISTEN:
            this->listen();
            break;
        case THINK:
            this->think();
            break;
        case SMILE:
            this->smile();
            break;
        case TEXT:
            this->updateText();
            break;
        default:
            this->reset();
    }
}

void MycroftMouth::talk() {
    state = TALK;
    byte size = 4;
    byte total = (size * 2) - 2;

    for (byte i = 0, count = 0; count < total; count++) {
        ht1632.clear();
        for (byte j = 0; j < size; j++) {
            byte idx = (i * size) + j;
            byte x = j * 8;
            this->read_buffer(idx);
            ht1632.drawImage(buffer, width, height, x, 0);
        }
        ht1632.render();
        delay(70);

        if (i < size - 1) {
            i++;
        } else {
            i--;
        }
    }
}

void MycroftMouth::read_buffer(byte idx) {
    byte size = sizeof(buffer);
    for (byte j = 0; j < size; j++) {
        buffer[j] = (char) pgm_read_byte(&(TALK_ANIMATION[idx][j]));
    }
}

void MycroftMouth::listen() {
    state = LISTEN;
}

void MycroftMouth::think() {
    state = THINK;
}

void MycroftMouth::smile() {
    state = SMILE;
}

void MycroftMouth::write(const char *value) {
    state = TEXT;
    copyText(value);
    textWd = HT1632.getTextWidth(text, FONT_5X4_WIDTH, FONT_5X4_HEIGHT);
    textIdx = 0;
    this->updateText();
}

void MycroftMouth::copyText(const char *value) {
    for (byte i = 0; i < sizeof(text); i++) {
        text[i] = ' ';
    }
    strcpy(text, value);
    Serial.println(text);
}

void MycroftMouth::updateText() {
    ht1632.transition(TRANSITION_BUFFER_SWAP);
    ht1632.clear();
    ht1632.drawText(text, OUT_SIZE - textIdx, 2, FONT_5X4, FONT_5X4_WIDTH, FONT_5X4_HEIGHT, FONT_5X4_STEP_GLYPH);
    ht1632.render();
    textIdx = (textIdx + 1) % (textWd + OUT_SIZE);
    delay(150);
}