/** @file Example.c
 * Example and test case for the MQTT library.
 * @author Adrien RICCIARDI
 */
#include <arpa/inet.h>
#include <errno.h>
#include <MQTT.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	unsigned char Buffer[1024];
	TMQTTContext MQTT_Context;
	TMQTTConnectionParameters MQTT_Connection_Parameters;
	int Socket;
	struct sockaddr_in Address;
	
	Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket == -1)
	{
		perror("socket()");
		return -1;
	}
	
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = inet_addr("192.168.1.13");
	Address.sin_port = htons(1883);
	if (connect(Socket, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		perror("connect()");
		return -1;
	}
	
	MQTT_Connection_Parameters.Pointer_String_Client_Identifier = "ID du client";
	MQTT_Connection_Parameters.Pointer_String_User_Name = "nom d'utilisateur";
	MQTT_Connection_Parameters.Pointer_String_Password = "super mot de passe de ouf";
	MQTT_Connection_Parameters.Is_Clean_Session_Enabled = 1;
	MQTT_Connection_Parameters.Keep_Alive = 60;
	MQTT_Connection_Parameters.Pointer_Buffer = Buffer;
	MQTTConnect(&MQTT_Context, &MQTT_Connection_Parameters);
	
	write(Socket, MQTT_Context.Pointer_Message_Buffer, MQTT_Context.Message_Size);
	
	//read(Socket, Buffer, sizeof(Buffer));
	
	close(Socket);
	
	return 0;
}