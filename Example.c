/** @file Example.c
 * Example and test case for the MQTT library.
 * @author Adrien RICCIARDI
 */
#include <MQTT.h>
#include <stdio.h>

// TEST
int MQTTAppendStringToPayload(unsigned char **Pointer_Pointer_Payload_Buffer, char *Pointer_String);

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	unsigned char Buffer[1024], *ptr = Buffer;
	FILE *Pointer_File;
	int Payload_Size;
	
	Payload_Size = MQTTAppendStringToPayload(&ptr, "premier message");
	Payload_Size += MQTTAppendStringToPayload(&ptr, "ceci est un message plus long");
	Payload_Size += MQTTAppendStringToPayload(&ptr, "plus court");
	Payload_Size += MQTTAppendStringToPayload(&ptr, "1");
	Payload_Size += MQTTAppendStringToPayload(&ptr, "");
	Payload_Size += MQTTAppendStringToPayload(&ptr, "celui-ci est vraiment tarpin long comparé aux autres, mais c'est vraiment dans un pur but de test. Je remercie tout ceux qui ont lu cette phrase jusqu'à la fin");
	
	Pointer_File = fopen("test", "w");
	fwrite(Buffer, 1, Payload_Size, Pointer_File);
	fclose(Pointer_File);
	
	return 0;
}
