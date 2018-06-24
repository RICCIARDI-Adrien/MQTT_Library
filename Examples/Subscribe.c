/** @file Subscribe.c
 * Topic "subscribe" example and test case for the MQTT library.
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
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	static unsigned char Buffer[1024]; // Avoid storing a big buffer on the stack
	char *Pointer_String_Server_IP_Address;
	unsigned short Server_Port;
	int Socket, Result;
	TMQTTContext MQTT_Context;
	TMQTTConnectionParameters MQTT_Connection_Parameters;
	struct sockaddr_in Address;
	ssize_t Read_Bytes_Count;
	
	// Check parameters
	if (argc != 3)
	{
		printf("Usage : %s MQTT_Server_IP_Address MQTT_Server_Port\n"
			"Use wireshark or another network traffic analyzer in the same time to check if the packets are well formed.\n", argv[0]);
		return EXIT_FAILURE;
	}
	Pointer_String_Server_IP_Address = argv[1];
	Server_Port = atoi(argv[2]);
	
	// Create a TCP socket
	Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket == -1)
	{
		printf("Error : failed to create socket (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Try to connect to the server
	printf("Connecting to '%s:%d'...\n", Pointer_String_Server_IP_Address, Server_Port);
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = inet_addr(Pointer_String_Server_IP_Address);
	Address.sin_port = htons(Server_Port);
	if (connect(Socket, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		printf("Error : failed to connect to MQTT server (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Send MQTT CONNECT packet
	printf("Sending CONNECT packet...\n");
	MQTT_Connection_Parameters.Pointer_String_Client_Identifier = "ID du client SUBSCRIBE";
	MQTT_Connection_Parameters.Pointer_String_User_Name = "Ceci est un message bien plus long pour voir si le calcul d'une taille de paquet supérieure à 127 octets fonctionne correctement pour SUBSCRIBE";
	MQTT_Connection_Parameters.Pointer_String_Password = "Ce champ aussi est allongé dans le but décrit exhaustivement dans le champ précédent";
	MQTT_Connection_Parameters.Is_Clean_Session_Enabled = 1;
	MQTT_Connection_Parameters.Keep_Alive = 60;
	MQTT_Connection_Parameters.Pointer_Buffer = Buffer;
	MQTTConnect(&MQTT_Context, &MQTT_Connection_Parameters);
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT CONNECT packet (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Specifications allow to send control packets without waiting for CONNACK, but if the client is too fast the server can't keep up, so wait for the CONNACK
	printf("Waiting for CONNACK packet...\n");
	Read_Bytes_Count = read(Socket, Buffer, sizeof(Buffer));
	Result = MQTTIsConnectionEstablished(Buffer, Read_Bytes_Count);
	if (Result != 0)
	{
		printf("Error : server rejected connection. CONNACK return code : 0x%X\n", Result);
		return EXIT_FAILURE;
	}
	
	// Subscribe to a topic
	printf("Sending SUBSCRIBE packet...\n");
	MQTTSubscribe(&MQTT_Context, "le topic est lui aussi assez long pour que le message publish dépasse 127 caractères et qu'il faille calculer la taille sur deux octets");
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT SUBSCRIBE packet (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	getchar();
	
	// Disconnect from MQTT server
	MQTTDisconnect(&MQTT_Context);
	printf("Sending DISCONNECT packet...\n");
	if (write(Socket, MQTT_GET_MESSAGE_BUFFER(&MQTT_Context), MQTT_GET_MESSAGE_SIZE(&MQTT_Context)) != MQTT_GET_MESSAGE_SIZE(&MQTT_Context))
	{
		printf("Error : failed to send MQTT DISCONNECT packet (%s).\n", strerror(errno));
		return EXIT_FAILURE;
	}
	close(Socket);
	
	// Test message with all 
	//PublishConnectAndPublishData("ID du client", "nom d'utilisateur", "super mot de passe de ouf", "test1", "charge utile courte", sizeof("charge utile courte") - 1);
	
	// Send a "big" message to test "remaining length" computation algorithm
	//PublishConnectAndPublishData("ID du client", "Ceci est un message bien plus long pour voir si le calcul d'une taille de paquet supérieure à 127 octets fonctionne correctement", "Ce champ aussi est allongé dans le but décrit exhaustivement dans le champ précédent", "le topic est lui aussi assez long pour que le message publish dépasse 127 caractères et qu'il faille calculer la taille sur deux octets", "la charge utile est un peu plus longue aussi même si la longueur du topic devrait suffire pour atteindre les 127 octets", sizeof("la charge utile est un peu plus longue aussi même si la longueur du topic devrait suffire pour atteindre les 127 octets") - 1);
	
	// Send a message with less parameters
	//PublishConnectAndPublishData("ID du client", NULL, NULL, "topic", "données", sizeof("données") - 1);
	
	return 0;
}
