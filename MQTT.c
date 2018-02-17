/** @file MQTT.c
 * @see MQTT.h for description.
 * @author Adrien RICCIARDI
 */
#include <assert.h>
#include <MQTT.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
// TODO macro to specify in makefile to tell system endianness
/** Convert a 16-bit word to big endian on little endian systems, or does nothing on big endian systems.
 * @param Value The value to convert.
 */
#define MQTT_CONVERT_WORD_TO_BIG_ENDIAN(Value) ((((Value) << 8) & 0xFF00) | (Value) >> 8)

/** How big can be the MQTT fixed header when the "remaining length" field uses all its available bytes. */
#define MQTT_FIXED_HEADER_MAXIMUM_SIZE 5

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** All supported MQTT control packet types. Values are shifted yet to allow a simple binary "or" operation. */
typedef enum
{
	MQTT_CONTROL_PACKET_TYPE_CONNECT = 1 << 4,
	MQTT_CONTROL_PACKET_TYPE_PUBLISH = 3 << 4,
	MQTT_CONTROL_PACKET_TYPE_DISCONNECT = 14 << 4
} TMQTTControlPacketType;

/** CONNECT message variable headers. */
typedef struct __attribute__((packed))
{
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
static int MQTTAppendString(unsigned char **Pointer_Pointer_Payload_Buffer, char *Pointer_String)
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

/** Compute the fixed header fields and set Pointer_Context->Pointer_Message_Buffer to the beginning of the message.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @param Control_Packet_Type The packet type.
 * @param Variable_Header_And_Payload_Size Message payload plus specific message variable header length.
 */
static void MQTTAddFixedHeader(TMQTTContext *Pointer_Context, TMQTTControlPacketType Control_Packet_Type, int Variable_Header_And_Payload_Size)
{
	unsigned char *Pointer_Fixed_Header;
	int Fixed_Header_Size;
	
	// Compute remaining length (see specification section 2.2.3 to get information about remaining length computation)
	if (Variable_Header_And_Payload_Size <= 127)
	{
		// Fixed header is 2-byte long
		Fixed_Header_Size = 2;
		
		// Remaining length field fits on a single byte
		Pointer_Fixed_Header = Pointer_Context->Pointer_Buffer + MQTT_FIXED_HEADER_MAXIMUM_SIZE - Fixed_Header_Size;
		
		// Set remaining length field
		Pointer_Fixed_Header[1] = Variable_Header_And_Payload_Size;
	}
	else if (Variable_Header_And_Payload_Size <= 16383)
	{
		// Fixed header is 3-byte long
		Fixed_Header_Size = 3;
		
		// Remaining length field fits on a two bytes
		Pointer_Fixed_Header = Pointer_Context->Pointer_Buffer + MQTT_FIXED_HEADER_MAXIMUM_SIZE - Fixed_Header_Size;
		
		// Set remaining length field
		Pointer_Fixed_Header[1] = (Variable_Header_And_Payload_Size % 128) | 0x80; // Set bit 7 to tell there is a following "remaining length" byte
		Pointer_Fixed_Header[2] = (Variable_Header_And_Payload_Size >> 7) % 128; // Fast division by 128
	}
	else assert(0); // TODO implement bigger sizes
	
	// Set control packet type and flags // TODO add flags support
	Pointer_Fixed_Header[0] = (unsigned char) Control_Packet_Type;
	
	// Set final message starting offset and total size
	Pointer_Context->Pointer_Message_Buffer = Pointer_Fixed_Header;
	Pointer_Context->Message_Size = Fixed_Header_Size + Variable_Header_And_Payload_Size;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void MQTTConnect(TMQTTContext *Pointer_Context, TMQTTConnectionParameters *Pointer_Connection_Parameters)
{
	TMQTTHeaderConnect *Pointer_Variable_Header;
	unsigned char *Pointer_Payload;
	int Payload_Size;
	
	// Do some safety checks on parameters
	assert(Pointer_Context != NULL);
	assert(Pointer_Connection_Parameters != NULL);
	assert(Pointer_Connection_Parameters->Pointer_String_Client_Identifier != NULL);
	assert(Pointer_Connection_Parameters->Pointer_Buffer != NULL);
	// TODO other parameters
	
	// Initialize context
	memset(Pointer_Context, 0, sizeof(TMQTTContext));
	Pointer_Context->Pointer_Buffer = Pointer_Connection_Parameters->Pointer_Buffer;
	
	// Cache message relevant parts access
	Pointer_Variable_Header = (TMQTTHeaderConnect *) (MQTT_FIXED_HEADER_MAXIMUM_SIZE + Pointer_Context->Pointer_Buffer); // Keep enough room at the buffer beginning to store the biggest possible fixed header
	Pointer_Payload = (unsigned char *) Pointer_Variable_Header + sizeof(TMQTTHeaderConnect);
	
	// Initialize CONNECT message header fixed fields
	Pointer_Variable_Header->Protocol_Name_Length = MQTT_CONVERT_WORD_TO_BIG_ENDIAN(4);
	Pointer_Variable_Header->Protocol_Name_Characters[0] = 'M';
	Pointer_Variable_Header->Protocol_Name_Characters[1] = 'Q';
	Pointer_Variable_Header->Protocol_Name_Characters[2] = 'T';
	Pointer_Variable_Header->Protocol_Name_Characters[3] = 'T';
	Pointer_Variable_Header->Protocol_Level = 4;
	Pointer_Variable_Header->Connect_Flags = 0; // Reset flags
	Pointer_Variable_Header->Keep_Alive = MQTT_CONVERT_WORD_TO_BIG_ENDIAN(Pointer_Connection_Parameters->Keep_Alive);
	
	// Add client identifier (this field is mandatory)
	Payload_Size = MQTTAppendString(&Pointer_Payload, Pointer_Connection_Parameters->Pointer_String_Client_Identifier);
	
	// Is a user name provided ?
	if (Pointer_Connection_Parameters->Pointer_String_User_Name != NULL)
	{
		Payload_Size += MQTTAppendString(&Pointer_Payload, Pointer_Connection_Parameters->Pointer_String_User_Name);
		// Set user name flag
		Pointer_Variable_Header->Connect_Flags |= 0x80;
	}
	
	// Is a password provided ?
	if (Pointer_Connection_Parameters->Pointer_String_Password != NULL)
	{
		Payload_Size += MQTTAppendString(&Pointer_Payload, Pointer_Connection_Parameters->Pointer_String_Password);
		// Set password flag
		Pointer_Variable_Header->Connect_Flags |= 0x40;
	}
	
	// Is session cleaning required ?
	if (Pointer_Connection_Parameters->Is_Clean_Session_Enabled) Pointer_Variable_Header->Connect_Flags |= 0x02; // Set clean session flag
	
	// Terminate message
	MQTTAddFixedHeader(Pointer_Context, MQTT_CONTROL_PACKET_TYPE_CONNECT, sizeof(TMQTTHeaderConnect) + Payload_Size);
}

void MQTTPublish(TMQTTContext *Pointer_Context, char *Pointer_String_Topic_Name, char *Pointer_Application_Message, int Application_Message_Size)
{
	unsigned char *Pointer_Variable_Header;
	int Data_Size;
	
	// Cache message relevant parts access
	Pointer_Variable_Header = (unsigned char *) (MQTT_FIXED_HEADER_MAXIMUM_SIZE + Pointer_Context->Pointer_Buffer); // Keep enough room at the buffer beginning to store the biggest possible fixed header
	
	// Add topic name (this field is mandatory)
	Data_Size = MQTTAppendString(&Pointer_Variable_Header, Pointer_String_Topic_Name);
	
	// TODO add packet identifier if QoS > 0
	
	// Add application message (if any)
	if (Application_Message_Size > 0)
	{
		memcpy(Pointer_Variable_Header, Pointer_Application_Message, Application_Message_Size); // There is no length field for the application message
		Data_Size += Application_Message_Size;
	}
	
	// Terminate message
	MQTTAddFixedHeader(Pointer_Context, MQTT_CONTROL_PACKET_TYPE_PUBLISH, Data_Size);
}

void MQTTDisconnect(TMQTTContext *Pointer_Context)
{
	Pointer_Context->Pointer_Buffer[0] = MQTT_CONTROL_PACKET_TYPE_DISCONNECT;
	Pointer_Context->Pointer_Buffer[1] = 0;
	
	Pointer_Context->Pointer_Message_Buffer = Pointer_Context->Pointer_Buffer;
	Pointer_Context->Message_Size = 2;
}
