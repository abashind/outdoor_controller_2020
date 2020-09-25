#pragma region Include
#include <OneWire.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <map>
#include <DallasTemperature.h>
#pragma endregion

#pragma region Pins
#define gate_lamps_pin 4
#define reset_pin 2
#define in_temp_pin 33
#pragma endregion

#pragma region ForBlynk
char auth[] = "vxFa-4f0xhSnfX87x2mrsKtjtaLECdBW";
char ssid[] = "7SkyHome";
char pass[] = "89191532537";
#pragma endregion

#pragma region Security
int panic_mode = 1;	  
#pragma endregion

#pragma region Blynk virt pins
#define vpin_gate_lamps_mode 1
#define vpin_panic_mode 2
#define vpin_temp_inside 3
#pragma endregion

Preferences pref;

int gate_lamps_mode = 1;                           
bool gate_lamps_enabled;

TaskHandle_t slow_blink_handle;
TaskHandle_t fast_blink_handle_1;
TaskHandle_t fast_blink_handle_2;

SemaphoreHandle_t wifi_mutex;
SemaphoreHandle_t pref_mutex;

float temp_inside;
OneWire sensor_1(in_temp_pin);
DallasTemperature temp_inside_sensor(&sensor_1);

#pragma region For watchdog

#define wdtTimeout 5000
hw_timer_t *timer = NULL;

void IRAM_ATTR restart() 
{
	pinMode(reset_pin, OUTPUT);
}
#pragma endregion

void setup()
{
	Serial.begin(9600);
	Serial.println("Setup start...");
	
	wifi_mutex = xSemaphoreCreateMutex();
	pref_mutex = xSemaphoreCreateMutex();
	
	pref.begin("pref_1", false);
	
	xSemaphoreTake(wifi_mutex, portMAX_DELAY);
	Blynk.begin(auth, ssid, pass);
	xSemaphoreGive(wifi_mutex);
	
	Wire.begin();
	
	read_settings_from_pref();
	
	temp_inside_sensor.begin();
	
	#pragma region PinInit
	pinMode(gate_lamps_pin, OUTPUT);
	#pragma endregion
	
	#pragma region Watchdog init
	timer = timerBegin(0, 80, true);                   
	timerAttachInterrupt(timer, &restart, true);    
	timerAlarmWrite(timer, wdtTimeout * 1000, false);  
	timerAlarmEnable(timer);                           
	#pragma endregion
	
	#pragma region TaskCreate
	xTaskCreatePinnedToCore(gate_lamps_control, "gate_lamps_control", 1024, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(send_data_to_blynk, "send_data_to_blynk", 10240, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(run_blynk, "run_blynk", 2048, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(write_setting_to_pref, "write_setting_to_pref", 2048, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(feed_watchdog, "feed_watchdog", 1024, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(heart_beat, "heart_beat", 1024, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(panic_control, "panic_control", 2048, NULL, 1, NULL,1);
	xTaskCreatePinnedToCore(get_temps, "get_temps", 2048, NULL, 2, NULL, 1);
	#pragma endregion
}

#pragma region BLYNK_WRITE

BLYNK_WRITE(vpin_panic_mode)
{
	panic_mode = param.asInt();
}

BLYNK_WRITE(vpin_gate_lamps_mode)
{
	gate_lamps_mode = param.asInt();
}

BLYNK_CONNECTED()
{
	Blynk.syncAll();
}

#pragma endregion

void loop(){}
