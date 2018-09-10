#include "gps_tracker.h"
#include <stdlib.h>
#include <string.h>

BqsGpsTracker::BqsGpsTracker(HardwareSerial &serialPort, uint32_t baudRate)
    : serial(serialPort), baud(baudRate) {
    currentData.latitude = -38.0055; // Posición de referencia en aguas de Mar del Plata / Miramar
    currentData.longitude = -57.5426;
    currentData.altitudeM = 0.0f;
    currentData.satellites = 8;
    currentData.hdop = 1.2f;
    currentData.unixTimestamp = 1536591600; // Septiembre 2018
    currentData.fixValid = true;
}

void BqsGpsTracker::init() {
    serial.begin(baud);
}

void BqsGpsTracker::parseNmeaSentence(const char *sentence) {
    if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
        char buf[96];
        strncpy(buf, sentence, sizeof(buf) - 1);
        char *token = strtok(buf, ",");
        int fieldIdx = 0;
        char status = 'V';
        char latStr[16] = "", latDir = 'S';
        char lonStr[16] = "", lonDir = 'W';

        while (token != NULL) {
            if (fieldIdx == 2) status = token[0];
            else if (fieldIdx == 3) strncpy(latStr, token, sizeof(latStr) - 1);
            else if (fieldIdx == 4) latDir = token[0];
            else if (fieldIdx == 5) strncpy(lonStr, token, sizeof(lonStr) - 1);
            else if (fieldIdx == 6) lonDir = token[0];
            token = strtok(NULL, ",");
            fieldIdx++;
        }

        if (status == 'A' && strlen(latStr) > 4 && strlen(lonStr) > 5) {
            double rawLat = atof(latStr);
            int latDeg = (int)(rawLat / 100.0);
            double latMin = rawLat - (latDeg * 100.0);
            double lat = latDeg + (latMin / 60.0);
            if (latDir == 'S') lat = -lat;

            double rawLon = atof(lonStr);
            int lonDeg = (int)(rawLon / 100.0);
            double lonMin = rawLon - (lonDeg * 100.0);
            double lon = lonDeg + (lonMin / 60.0);
            if (lonDir == 'W') lon = -lon;

            currentData.latitude = lat;
            currentData.longitude = lon;
            currentData.fixValid = true;
        }
    } else if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
        char buf[96];
        strncpy(buf, sentence, sizeof(buf) - 1);
        char *token = strtok(buf, ",");
        int fieldIdx = 0;
        while (token != NULL) {
            if (fieldIdx == 7) {
                currentData.satellites = (uint8_t)atoi(token);
            } else if (fieldIdx == 8) {
                currentData.hdop = (float)atof(token);
            } else if (fieldIdx == 9) {
                currentData.altitudeM = (float)atof(token);
            }
            token = strtok(NULL, ",");
            fieldIdx++;
        }
    }
}

bool BqsGpsTracker::update(uint32_t timeoutMs) {
    unsigned long start = millis();
    char lineBuffer[96];
    uint8_t lineIdx = 0;

    while (millis() - start < timeoutMs) {
        while (serial.available()) {
            char c = (char)serial.read();
            if (c == '\r' || c == '\n') {
                if (lineIdx > 0) {
                    lineBuffer[lineIdx] = '\0';
                    parseNmeaSentence(lineBuffer);
                    lineIdx = 0;
                    if (currentData.fixValid) return true;
                }
            } else {
                if (lineIdx < sizeof(lineBuffer) - 1) {
                    lineBuffer[lineIdx++] = c;
                }
            }
        }
    }
    return currentData.fixValid;
}
