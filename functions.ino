void read_settings_from_pref()
{
	xSemaphoreTake(pref_mutex, portMAX_DELAY);
	gate_lamps_mode = pref.getInt("gate_lamps_mode");
	xSemaphoreGive(pref_mutex);	
}

void gate_lamps_control(void *pvParameters)
{
	while (true)
	{
		//If panic, gate lamps are managed from panic_control()
		if(panic_mode != 1) {}
		//Outside lamp OFF
		else if(gate_lamps_mode == 1)
		{
			digitalWrite(gate_lamps_pin, LOW);
			gate_lamps_enabled = false;
		}
		//Outside lamp ON
		else if(gate_lamps_mode == 2)
		{
			digitalWrite(gate_lamps_pin, HIGH);
			gate_lamps_enabled = true;
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void run_blynk(void *pvParameters)
{
	while (true)
	{
		xSemaphoreTake(wifi_mutex, portMAX_DELAY);
		if (WiFi.status() != WL_CONNECTED) 
		{
			Serial.println("WiFi is not connected, try to establish connection...");
			Blynk.connectWiFi(ssid, pass);
		}
		Blynk.run();
		xSemaphoreGive(wifi_mutex);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}

void send_data_to_blynk(void *pvParameters)
{
	while (true)
	{
		xSemaphoreTake(wifi_mutex, portMAX_DELAY);
		if (panic_mode != 1)
		{	
			if (gate_lamps_enabled)
				Blynk.virtualWrite(vpin_gate_lamps_mode, 2);
			else
				Blynk.virtualWrite(vpin_gate_lamps_mode, 1);
		}
		Blynk.virtualWrite(vpin_temp_inside, temp_inside);
		
		xSemaphoreGive(wifi_mutex);
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void gate_lamps_blinks(void *pvParameters)
{
	int interval = (int) pvParameters;
	
	while (true)
	{
		if (gate_lamps_enabled)
		{
			digitalWrite(gate_lamps_pin, LOW);
			gate_lamps_enabled = false;
		}
		else
		{
			digitalWrite(gate_lamps_pin, HIGH);
			gate_lamps_enabled = true;
		}
		vTaskDelay(interval / portTICK_RATE_MS);
	}
}

void panic_control(void *pvParameters)
{
	while (true)
	{
		//Gate lamps blink.
		if(panic_mode == 2)
		{
			if ((slow_blink_handle) == NULL)
				xTaskCreate(gate_lamps_blinks, "gate_lamps_blinks", 10000, (void *)1000, 1, &slow_blink_handle);
		}
		else if(slow_blink_handle != NULL)
		{
			vTaskDelete(slow_blink_handle);
			slow_blink_handle = NULL;
		}
		//Gate lamps work like a strobe.
		if(panic_mode == 3)
		{
			if (fast_blink_handle_1 == NULL)
				xTaskCreate(gate_lamps_blinks, "gate_lamps_blinks", 10000, (void *)166, 1, &fast_blink_handle_1); 
		}
		else 
		{
			if (fast_blink_handle_1 != NULL)
			{
				vTaskDelete(fast_blink_handle_1);
				fast_blink_handle_1 = NULL;
			}
		}
		//Gate lamps work like a strobe.
		if(panic_mode == 4)
		{
			if (fast_blink_handle_2 == NULL)
				xTaskCreate(gate_lamps_blinks, "gate_lamps_blinks", 10000, (void *)166, 1, &fast_blink_handle_2); 
		}
		else if(fast_blink_handle_2 != NULL)
		{
			vTaskDelete(fast_blink_handle_2);
			fast_blink_handle_2 = NULL;
		}
		vTaskDelay(300 / portTICK_RATE_MS);
	}
}

void write_setting_to_pref(void *pvParameters)
{
	while (true)
	{
		xSemaphoreTake(pref_mutex, portMAX_DELAY);
		pref.putInt("gate_lamps_mode", gate_lamps_mode);                                     
		xSemaphoreGive(pref_mutex);
		vTaskDelay(30000 / portTICK_RATE_MS);
	}
}

void feed_watchdog(void *pvParameters)
{
	while (true)
	{
		timerWrite(timer, 0);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void heart_beat(void *pvParameters)
{
	while (true)
	{
		Serial.println("This is just loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong string.");
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void get_temps(void *pvParameters)
{
	while (true)
	{
		temp_inside_sensor.requestTemperatures();
		temp_inside = temp_inside_sensor.getTempCByIndex(0);
		
		vTaskDelay(10000 / portTICK_RATE_MS);
	}
}