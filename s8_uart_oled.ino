#include <Arduino.h>
#include "s8_uart.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* 配置区 */
#define DEBUG_BAUDRATE 115200

#if defined(USE_SOFTWARE_SERIAL) || defined(ARDUINO_ARCH_RP2040)
  #define S8_RX_PIN 10
  #define S8_TX_PIN 11
#else
  #define S8_UART_PORT 1
#endif

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   32
#define OLED_RESET    -1       // -1 = 没有接 RESET 引脚
#define OLED_ADDRESS  0x3C     // Scanner 发现的地址
/* 结束配置 */

#ifdef USE_SOFTWARE_SERIAL
  SoftwareSerial S8_serial(S8_RX_PIN, S8_TX_PIN);
#else
  #if defined(ARDUINO_ARCH_RP2040)
    REDIRECT_STDOUT_TO(Serial)
    UART S8_serial(S8_TX_PIN, S8_RX_PIN, NC, NC);
  #else
    HardwareSerial S8_serial(S8_UART_PORT);
  #endif
#endif

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

S8_UART *sensor_S8;
S8_sensor sensor;

void setup() {
  delay(2000);
  // 串口调试
  Serial.begin(DEBUG_BAUDRATE);
  while (!Serial) delay(1);
  Serial.println("\nSetup start");

  // 默认 I2C 引脚
  Wire.begin();  
  delay(10);

  // OLED 初始化
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED init failed!");
    while (1) { delay(10); }  
  }
  Serial.println("OLED init OK");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("OLED OK @0x3C");
  display.display();
  delay(1000);

  // S8 传感器初始化
  S8_serial.begin(S8_BAUDRATE);
  sensor_S8 = new S8_UART(S8_serial);

  // 检查传感器
  sensor_S8->get_firmware_version(sensor.firm_version);
  if (strlen(sensor.firm_version) == 0) {
    Serial.println("S8 not found!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("S8 Not Found!");
    display.display();
    while (1) delay(1);
  }
  Serial.print("S8 FW: "); Serial.println(sensor.firm_version);
  sensor.sensor_id = sensor_S8->get_sensor_ID();
  Serial.print("S8 ID: 0x"); printIntToHex(sensor.sensor_id,4); Serial.println();

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("S8 Ready");
  display.display();
  delay(500);
}

void loop() {
  sensor.co2 = sensor_S8->get_co2();
  Serial.print("CO2: "); Serial.print(sensor.co2); Serial.println(" ppm");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("    CO2:");
  display.setCursor(0, 18);
  display.print(" ");
  display.print(sensor.co2);
  display.print(" ppm");
  display.display();

  delay(2000);
}
