#include <Wire.h>
#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED SETUP 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define OLED_SDA 19
#define OLED_SCL 21

Adafruit_SSD1306 display(
    SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

//BLUETOOTH 

BluetoothSerial SerialBT;



#define BUZZER_PIN 18

//modes

enum pomodorotimermodes {
    waitingforauser,
    Locked_in_Working,
    Break
};

pomodorotimermodes currentmode = waitingforauser;



int workMinutes = 0;
int breakMinutes = 0;
bool breakAllowed = false;
String incomingCommand = "";

int sessioncount = 0;
int worksessioncount = 0;
int breaksessioncount = 0;

// TIMER VARIABLES 

int countdown = 0;

unsigned long timerEndMillis = 0;
unsigned long lastDisplayUpdate = 0;

bool timerActive = false;

//BUZZER FUNCTIONS 

void workStartSound() {
    tone(BUZZER_PIN, 1800);
    delay(150);
    noTone(BUZZER_PIN);    
    }

void breakStartSound() {
        tone(BUZZER_PIN, 1000);
    delay(150);
    noTone(BUZZER_PIN);
    }

void finishSound() {
   tone(BUZZER_PIN, 2200);
    delay(500);
    noTone(BUZZER_PIN);
    }

//STARTUP SMILEY

void showSmiley() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Face
    display.drawCircle(64, 29, 20, SSD1306_WHITE);

    // Eyes
    display.fillCircle(57, 23, 2, SSD1306_WHITE);
    display.fillCircle(71, 23, 2, SSD1306_WHITE);

    // Smile
    display.drawLine(54, 33, 58, 38, SSD1306_WHITE);
    display.drawLine(58, 38, 64, 40, SSD1306_WHITE);
    display.drawLine(64, 40, 70, 38, SSD1306_WHITE);
    display.drawLine(70, 38, 74, 33, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(39, 55);
    display.print("READY TO FOCUS");

    display.display();
    delay(1500);
}

//START A TIMER

void startTimer(int durationMinutes) {
    countdown = durationMinutes * 60;
    timerEndMillis = millis() + (unsigned long)countdown * 1000UL;
    timerActive = true;
}

// BLUETOOTH FUNCTION 

void handleBluetooth() {
    while (SerialBT.available()) {
        char received = SerialBT.read();

        if (received == '\n') {
            incomingCommand.trim();

            // Select work duration
            if (incomingCommand == "W25") {
                workMinutes = 25;
                SerialBT.println("Work timer set to 25 minutes.");
            }
            else if (incomingCommand == "W45") {
                workMinutes = 45;
                SerialBT.println("Work timer set to 45 minutes.");
            }
            // Select break duration
            else if (incomingCommand == "B5") {
                breakMinutes = 5;
                SerialBT.println("Break timer set to 5 minutes.");
            }
            else if (incomingCommand == "B15") {
                breakMinutes = 15;
                SerialBT.println("Break timer set to 15 minutes.");
            }
            // Start work
            else if (incomingCommand == "STARTWORK") {
                if (currentmode == waitingforauser && breakAllowed == false && workMinutes > 0) {
                    startTimer(workMinutes);
                    currentmode = Locked_in_Working;
                    workStartSound();
                    SerialBT.println("Work started!");
                }
                else {
                    SerialBT.println("Cannot start work!");
                    if (breakAllowed == true) {
                        SerialBT.println("Finish your break first.");
                    }
                    else if (workMinutes == 0) {
                        SerialBT.println("Choose W25 or W45 first.");
                    }
                    else {
                        SerialBT.println("Timer is already running.");
                    }
                }
            }
            // Start break
            else if (incomingCommand == "STARTBREAK") {
                if (currentmode == waitingforauser && breakAllowed == true && breakMinutes > 0) {
                    startTimer(breakMinutes);
                    currentmode = Break;
                    breakStartSound();
                    SerialBT.println("Break started!");
                }
                else {
                    SerialBT.println("Cannot start break!");
                    if (breakAllowed == false) {
                        SerialBT.println("Complete work first.");
                    }
                    else if (breakMinutes == 0) {
                        SerialBT.println("Choose B5 or B15 first.");
                    }
                    else {
                        SerialBT.println("Timer is already running.");
                    }
                }
            }
            // Reset timer and restart the cycle
            else if (incomingCommand == "RESET") {
                timerActive = false;
                countdown = 0;
                breakAllowed = false;
                currentmode = waitingforauser;
                noTone(BUZZER_PIN);
                SerialBT.println("Timer reset.");
                SerialBT.println("Ready for a work session.");
            }
            
            else if (incomingCommand.length() > 0) {
                SerialBT.println("Unknown command");
            }

            incomingCommand = "";
        }
        else if (received != '\r') {
            incomingCommand += received;
        }
    }
}

//WORK TIMER FUNCTION 
void worktime() {
    if (!timerActive) {
        return;
    }

    unsigned long now = millis();

    if (now >= timerEndMillis) {
        countdown = 0;
        timerActive = false;

        sessioncount++;
        worksessioncount++;
        breakAllowed = true; // Allow break after work finishes

        finishSound();
        SerialBT.println("Work session done lets goo");
        currentmode = waitingforauser;
        return;
    }

    countdown = (timerEndMillis - now + 999UL) / 1000UL;
}

//BREAK TIMER FUNCTION 

void breaktime() {
    if (!timerActive) {
        return;
    }

    unsigned long now = millis();

    if (now >= timerEndMillis) {
        countdown = 0;
        timerActive = false;

        breaksessioncount++;
        breakAllowed = false; // Require work session next

        finishSound();
        SerialBT.println("Break finished!");
        currentmode = waitingforauser;
        return;
    }

    countdown = (timerEndMillis - now + 999UL) / 1000UL;
}

//OLED UPDATE FUNCTION

void updateoled() {
    if (millis() - lastDisplayUpdate < 200) {
        return;
    }

    lastDisplayUpdate = millis();

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);

    switch (currentmode) {
        case waitingforauser:
            display.println("POMODORO  READY");
            break;
        case Locked_in_Working:
            display.println("WORK SESSION");
            break;
        case Break:
            display.println("BREAK TIME");
            break;
    }

    int minutes = countdown / 60;
    int seconds = countdown % 60;

    char timeText[6];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", minutes, seconds);

    display.setTextSize(2);
    display.setCursor(25, 17);
    display.print(timeText);

    display.setTextSize(1);
    display.setCursor(0, 44);
    display.print("Sessions: ");
    display.print(sessioncount);

    display.setCursor(0, 55);
    if (timerActive) {
        display.print("RUNNING");
    }
    else {
        display.print("WAITING");
    }

    display.display();
}

//SETUP

void setup() {
    Serial.begin(115200);

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED initialization failed");
        while (true) {
            delay(100);
        }
    }

    pinMode(BUZZER_PIN, OUTPUT);

    tone(BUZZER_PIN, 2000, 200); // Plays a 2000Hz tone for 200ms
    delay(250);
    
    SerialBT.begin("ESP32_Study_helper");
    
    showSmiley();

    SerialBT.println("W25       - Set work to 25 minutes");
    SerialBT.println("W45       - Set work to 45 minutes");
    SerialBT.println("B5        - Set break to 5 minutes");
    SerialBT.println("B15       - Set break to 15 minutes");
    SerialBT.println("STARTWORK - Start work session");
    SerialBT.println("STARTBREAK - Start break session");
    SerialBT.println("RESET     - Stop the timer");

    Serial.println("Pomodoro device should be working");
    SerialBT.println("Pomodoro device is ready");
}

void loop() {
    handleBluetooth();

    switch (currentmode) {
        case waitingforauser:
            break;
        case Locked_in_Working:
            worktime();
            break;
        case Break:
            breaktime();
            break;
    }

    updateoled();
}
