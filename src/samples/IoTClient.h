#include <MQTTClient.h>
#include <MQTTAsync.h>
#ifndef __IOTCLIENT_H__
#define __IOTCLIENT_H__
void delivered(void* context, MQTTClient_deliveryToken dt);
char* GenerateStr(void);
int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message);
void connlost(void* context, char* cause);
int  _SendMQTTMessage(char* _topic, char* playload, int playloadlen);
int _DisConnectMQTTBroker(void);
int  _ConnectMQTTBroker(void);

#define echo_mqtt(fmt, args...)   fprintf(stderr,"echo_mqtt:[%ld %s,%s:%d]: "fmt"\n",time(NULL), __FILE__, __func__,__LINE__ , ## args)

#define ADDRESS     "mqtt.eclipse.org:1883"

#define QOS         1
#define TIMEOUT     10000L

#endif
