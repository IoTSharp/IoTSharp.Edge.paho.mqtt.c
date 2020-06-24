#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <MQTTClient.h>
#include <IoTClient.h>


char  _mqtt_CLIENTID[64] = { 0 };
char _mqtt_username[50] = { 0 };
MQTTAsync _iot_client;

#define STR_LEN 32//定义随机输出的字符串长度。
char _iot_str[STR_LEN + 1] = { 0 };
char* GenerateStr(void)
{

	int i, flag;

	srand(time(NULL));//通过时间函数设置随机数种子，使得每次运行结果随机。
	for (i = 0; i < STR_LEN; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			_iot_str[i] = rand() % 26 + 'a';
			break;
		case 1:
			_iot_str[i] = rand() % 26 + 'A';
			break;
		case 2:
			_iot_str[i] = rand() % 10 + '0';
			break;
		}
	}
	echo_mqtt("%s", _iot_str);//输出生成的随机数。

	return _iot_str;
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void* context, MQTTClient_deliveryToken dt)
{
	echo_mqtt("Message with token value %d delivery confirmed", dt);
	deliveredtoken = dt;
}

int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
	echo_mqtt("Message arrived");
	echo_mqtt("     topic: %s", topicName);
	echo_mqtt("   message: %.*s", message->payloadlen, (char*)message->payload);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void* context, char* cause)
{
	echo_mqtt("\nConnection lost");
	echo_mqtt("     cause: %s", cause);
}




int  _SendMQTTMessage(char* _topic, char* playload, int playloadlen)
{
	int rc=0;
	if (_iot_client != NULL)
	{
		MQTTClient_message pubmsg = MQTTAsync_message_initializer;
		MQTTClient_deliveryToken token;
		
		echo_mqtt("Successful connection");
		pubmsg.payload = playload;
		pubmsg.payloadlen = playloadlen;
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
		if ((rc = MQTTClient_publishMessage(_iot_client, _topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
		{
			printf("Failed to publish message, return code %d", rc);
		}
		else
		{
			echo_mqtt("Waiting for up to %d seconds for publication of %s\n"
				"on topic %s for client with ClientID: %s",
				(int)(TIMEOUT / 1000), playload, _topic, _mqtt_CLIENTID);
			rc = MQTTClient_waitForCompletion(_iot_client, token, TIMEOUT);
			echo_mqtt("Message with delivery token %d delivered", token);
		}
	}
	else {
		rc = -1;
	}
	return rc;
}
int  _ConnectMQTTBroker(void)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;

	if ((rc = MQTTClient_create(&_iot_client, ADDRESS, _mqtt_CLIENTID,
		MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
	{
		echo_mqtt("Failed to create client, return code %d", rc);
		rc = EXIT_FAILURE;
	}
	else
	{
		if ((rc = MQTTClient_setCallbacks(_iot_client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
		{
			echo_mqtt("Failed to set callbacks, return code %d", rc);
			rc = EXIT_FAILURE;
		}
		else
		{
			conn_opts.keepAliveInterval = 20;
			conn_opts.cleansession = 1;
			sprintf((char*)&_mqtt_username, "650%s%s", "9999", "X01");
			conn_opts.username = (char*)&_mqtt_username;
			conn_opts.password = "1-q2-w3-e4-r5-t";
			if ((rc = MQTTClient_connect(_iot_client, &conn_opts)) != MQTTCLIENT_SUCCESS)
			{
				echo_mqtt("Failed to connect, return code %d", rc);
				rc = EXIT_FAILURE;
			
			}
			else
			{
				char* _topics[2] = { "/devices/me/rpc/request/+/+", "/devices/me/attributes/update/" };
				int qos[3] = { QOS,QOS ,QOS };
				if ((rc = MQTTClient_subscribeMany(_iot_client, 2, (char* const*)&_topics, (int*)&qos)) != MQTTCLIENT_SUCCESS)
				{
					echo_mqtt("Failed to subscribe, return code %d", rc);
					rc = EXIT_FAILURE;
				}
			}
		}
	}
	return rc;
}
int _DisConnectMQTTBroker(void)
{
	int rc = 0;
	if((rc = MQTTClient_disconnect(_iot_client, 10000)) != MQTTCLIENT_SUCCESS)
	{
		echo_mqtt("Failed to disconnect, return code %d", rc);
		rc = EXIT_FAILURE;
	}
	else
	{
	MQTTClient_destroy(&_iot_client);
	}
	return rc;
}
void SendAttributes(char* playload, int playloadlen)
{
	_SendMQTTMessage("devices/me/attributes", playload, playloadlen);
}

void SendTelemetry(char* playload, int playloadlen)
{
	_SendMQTTMessage("devices/me/telemetry", playload, playloadlen);
}

int main(int argc, char* argv[])
{
	int rc=_ConnectMQTTBroker();
	if (rc == 0)
	{


		int d = 0;
		do
		{
			d++;
			char tempstr[200] = { 0 };
			sprintf(tempstr, "{{\"TEST\":\"%d\"}]}", d);
			SendTelemetry(tempstr, strlen(tempstr));
			char jnamestr[200] = { 0 };
			sprintf(jnamestr, "{{\"name\":\"iotclient\"}}");
			SendAttributes(jnamestr, strlen(jnamestr));
			sleep(1);
		} while (1);
	}
	_DisConnectMQTTBroker();
}