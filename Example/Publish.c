/** @file Publish.c
 * Topic "publish" example and test case for the MQTT library.
 * @author Adrien RICCIARDI
 */
#include <arpa/inet.h>
#include <errno.h>
#include <MQTT.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Server IP address. */
static char Publish_String_Server_IP_Address[40]; // Enough for an IPv6 address.
/** Server port. */
static unsigned short Publish_Server_Port;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
void PublishConnectAndPublishData(char *Pointer_String_Client_ID, char *Pointer_String_User_Name, char *Pointer_String_Password, char *Pointer_String_Topic_Name, void *Pointer_Application_Data, int Application_Data_Size)
{
	static unsigned char Buffer[1024]; // Avoid storing a big buffer on the stack
	TMQTTContext MQTT_Context;
	TMQTTConnectionParameters MQTT_Connection_Parameters;
	int Socket, Result;
	struct sockaddr_in Address;
	ssize_t Read_Bytes_Count;
	
	// Create a TCP socket
	Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket == -1)
	{
		printf("Error : failed to create socket (%s).\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Try to connect to the server
	printf("Connecting to '%s:%d'...\n", Publish_String_Server_IP_Address, Publish_Server_Port);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = inet_addr(Publish_String_Server_IP_Address);
	Address.sin_port = htons(Publish_Server_Port);
	if (connect(Socket, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		printf("Error : failed to connect to MQTT server (%s).\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Send MQTT CONNECT packet
	printf("Sending CONNECT packet...\n");
	MQTT_Connection_Parameters.Pointer_String_Client_Identifier = Pointer_String_Client_ID;
	MQTT_Connection_Parameters.Pointer_String_User_Name = Pointer_String_User_Name;
	MQTT_Connection_Parameters.Pointer_String_Password = Pointer_String_Password;
	MQTT_Connection_Parameters.Is_Clean_Session_Enabled = 1;
	MQTT_Connection_Parameters.Keep_Alive = 60;
	MQTT_Connection_Parameters.Pointer_Buffer = Buffer;
	MQTTConnect(&MQTT_Context, &MQTT_Connection_Parameters);
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT CONNECT packet (%s).\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Specifications allow to send control packets without waiting for CONNACK, but if the client is too fast the server can't keep up, so wait for the CONNACK
	Read_Bytes_Count = read(Socket, Buffer, sizeof(Buffer));
	Result = MQTTIsConnectionEstablished(Buffer, Read_Bytes_Count);
	if (Result != 0)
	{
		printf("Error : server rejected connection. CONNACK return code : 0x%X\n", Result);
		exit(EXIT_FAILURE);
	}
	
	// Publish data
	printf("Sending PUBLISH packet...\n");
	MQTTPublish(&MQTT_Context, Pointer_String_Topic_Name, Pointer_Application_Data, Application_Data_Size);
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT PUBLISH packet (%s).\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Disconnect from MQTT server
	MQTTDisconnect(&MQTT_Context);
	printf("Sending DISCONNECT packet...\n");
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT DISCONNECT packet (%s).\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	putchar('\n');
	close(Socket);
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	// Check parameters
	if (argc != 3)
	{
		printf("Usage : %s MQTT_Server_IP_Address MQTT_Server_Port\n"
			"Use wireshark or another network traffic analyzer in the same time to check if the packets are well formed.\n", argv[0]);
		return EXIT_FAILURE;
	}
	strcpy(Publish_String_Server_IP_Address, argv[1]);
	Publish_Server_Port = atoi(argv[2]);
	
	// Test message with all parameters set
	PublishConnectAndPublishData("ID du client", "nom d'utilisateur", "super mot de passe de ouf", "test1", "charge utile courte", sizeof("charge utile courte") - 1);
	
	// Send a "big" message to test "remaining length" computation algorithm
	PublishConnectAndPublishData("ID du client", "Ceci est un message bien plus long pour voir si le calcul d'une taille de paquet supérieure à 127 octets fonctionne correctement", "Ce champ aussi est allongé dans le but décrit exhaustivement dans le champ précédent", "le topic est lui aussi assez long pour que le message publish dépasse 127 caractères et qu'il faille calculer la taille sur deux octets", "la charge utile est un peu plus longue aussi même si la longueur du topic devrait suffire pour atteindre les 127 octets", sizeof("la charge utile est un peu plus longue aussi même si la longueur du topic devrait suffire pour atteindre les 127 octets") - 1);
	
	// Send a message with less parameters
	PublishConnectAndPublishData("ID du client", NULL, NULL, "topic", "données", sizeof("données") - 1);
	
	return 0;
}
