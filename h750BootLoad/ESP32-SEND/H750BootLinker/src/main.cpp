#include <Arduino.h>
#include <WiFiManager.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

WiFiManager wm;
WiFiServer bootRecServer(19980);
QueueHandle_t xTcpRecQueue;


void tcpRec(void *pvParameters);
void serialSend(void *pvParameters);

void setup()
{

	Serial.begin(115200);
	 wm.autoConnect("AutoConnectAP", "password"); // password protected ap
	Serial.print("MOSI: ");
	Serial.println(MOSI);
	Serial.print("MISO: ");
	Serial.println(MISO);
	Serial.print("SCK: ");
	Serial.println(SCK);
	Serial.print("SS: ");
	Serial.println(SS);
	SPI.begin();
	bootRecServer.begin();
	xTcpRecQueue = xQueueCreate(100, sizeof(uint8_t));
	xTaskCreate(tcpRec, "tcpRec", 2048, NULL, 1, NULL);
	xTaskCreate(serialSend, "serialSend", 1024, NULL, 2, NULL);
}

void loop()
{
}

void tcpRec(void *pvParameters)
{
	Serial.println("tcp task start");
	while(1)
	{
		Serial.println("wait for connect.....\r\n");
		WiFiClient client = bootRecServer.available(); // client指向发出请求的客户端对象
		if (client)
		{
			Serial.println("a client connected");
			while (client.connected())
			{
				int recLenth=client.available();
				if (recLenth)
				{ // 检测有无数据可读
					// 处理客户端发送的数据
					uint8_t recByte = client.read();
					
					xQueueSend(xTcpRecQueue, &recByte, portMAX_DELAY);
				}
				vTaskDelay(10);
			}
		}
		else
		{
			client.stop();
			vTaskDelay(10);
		}
		vTaskDelay(1000);
	}

}

void serialSend(void *pvParameters)
{
	Serial.println("serial task start");
	while(1)
	{
		uint8_t val;
		xQueueReceive(xTcpRecQueue, &val, portMAX_DELAY);
		Serial.println("SPI SEND\r\n");
		Serial.println(val);
		SPI.writeBytes((uint8_t*)&val,1);
	}
}