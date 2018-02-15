/** @file MQTT.c
 * @see MQTT.h for description.
 * @author Adrien RICCIARDI
 */
#include <assert.h>
#include <MQTT.h>
#include <string.h>

// TODO macro to specify in makefile to tell system endianness
#define MQTT_CONVERT_WORD_TO_BIG_ENDIAN(Value) (((Value << 8) & 0xFF00) | Value >> 8)

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** CONNECT message fixed and variable headers. */
typedef struct __attribute__((packed))
{
	unsigned char Control_Packet_Type;
	unsigned char Remaining_Length; //!< Variable header size + payload size.
	unsigned short Protocol_Name_Length;
	unsigned char Protocol_Name_Characters[4];
	unsigned char Protocol_Level; //!< Stands for MQTT version.
	unsigned char Connect_Flags;
	unsigned short Keep_Alive;
} TMQTTHeaderConnect;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Append string data to the provided payload buffer.
 * @param Pointer_Pointer_Payload_Buffer A pointer on the payload's pointer. Payload's pointer is updated with the appended data size when function exits.
 * @param Pointer_String The string data to append.
 * @return How many bytes of payload have been added.
 */
/*static*/ int MQTTAppendStringToPayload(unsigned char **Pointer_Pointer_Payload_Buffer, char *Pointer_String)
{
	unsigned char *Pointer_Payload;
	unsigned short Length, *Pointer_Word;
	
	Pointer_Payload = *Pointer_Pointer_Payload_Buffer;
	
	// Set length field
	Length = (unsigned short) strlen(Pointer_String);
	Pointer_Word = (unsigned short *) Pointer_Payload;
	*Pointer_Word = MQTT_CONVERT_WORD_TO_BIG_ENDIAN(Length);
	Pointer_Payload += 2;
	
	// Set data field
	memcpy(Pointer_Payload, Pointer_String, Length);
	Pointer_Payload += Length;
	
	*Pointer_Pointer_Payload_Buffer = Pointer_Payload;
	return Length + 2; // +2 for the length field
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
# if 0
int MQTTConnect(TMQTTContext *Pointer_Context, TMQTTConnectionParameters *Pointer_Connection_Parameters)
{
	TMQTTHeaderConnect *Pointer_Header;
	unsigned char *Pointer_Payload;
	unsigned short Length; *Pointer_Word;
	
	// Do some safety checks on parameters
	assert(Pointer_Context != NULL);
	assert(Pointer_Connection_Parameters != NULL);
	assrt(Pointer_Connection_Parameters->Pointer_String_Client_Identifier != NULL);
	assert(Pointer_Connection_Parameters->Pointer_Buffer != NULL);
	assert(Pointer_Connection_Parameters->Buffer_Size > 0);
	// TODO other parameters
	
	// Initialize context
	memset(Pointer_Context, 0, sizeof(TMQTTContext));
	Pointer_Context->Pointer_Message_Buffer = Pointer_Buffer->Pointer_Buffer;
	Pointer_Context->Message_Buffer_Maximum_Size = Buffer_Size;
	
	// Cache message relevant parts access
	Pointer_Header = Pointer_Context->Pointer_Message_Buffer;
	Pointer_Payload = Pointer_Context->Pointer_Message_Buffer + sizeof(TMQTTHeaderConnect);
	
	// Initialize CONNECT message header fixed fields
	Pointer_Header->Control_Packet_Type = 0x10;
	Pointer_Header->Protocol_Name_Length = MQTT_CONVERT_WORD_TO_BIG_ENDIAN(4);
	Pointer_Header->Protocol_Name_Characters[0] = 'M';
	Pointer_Header->Protocol_Name_Characters[1] = 'Q';
	Pointer_Header->Protocol_Name_Characters[2] = 'T';
	Pointer_Header->Protocol_Name_Characters[3] = 'T';
	Pointer_Header->Protocol_Level = 4;
	
	// Add client identifier (this field is mandatory)
	// Set length field
	Length = (unsigned short) strlen(Pointer_Connection_Parameters->Pointer_String_Client_Identifier);
	Pointer_Word = (unsigned short *) Pointer_Payload;
	*Pointer_Word = MQTT_CONVERT_WORD_TO_BIG_ENDIAN(Length);
	Pointer_Payload += 2;
	// Set data field
	memcpy(Pointer_Payload, Pointer_Connection_Parameters->Pointer_String_Client_Identifier, Length);
	Pointer_Payload += Length;
	
	// Is a user name provided ?
	/*if (Pointer_Connection_Parameters->Pointer_String_User_Name != NULL)
	{
		Length = (unsigned short) strlen(Pointer_Connection_Parameters->Pointer_String_User_Name);
		memcpy(*/
}

#endif