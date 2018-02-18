/** @file MQTT.h
 * Simple MQTT 3.1.1 library targeted to 32-bit embedded systems providing a standard C library.
 * Implementation is based on MQTT specifications version 3.1.1 : http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html.
 * @author Adrien RICCIARDI
 */
#ifndef H_MQTT_H
#define H_MQTT_H

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** Context shared across MQTT functions. */
typedef struct
{
	// Following fields contain the MQTT message ready to transmit or to decode (in case of reception)
	unsigned char *Pointer_Message_Buffer;
	int Message_Size;
	
	// Following fields are for internal usage only, do not modify or use
	unsigned char *Pointer_Buffer; //!< The buffer in which messages are forged. A message does not necessarily start from offset 0. Only Pointer_Message_Buffer pointer tells the message beginning.
} TMQTTContext;

/** Parameters to provide when establishing an MQTT connection to the server. */
typedef struct
{
	char *Pointer_String_Client_Identifier; //!< This string is mandatory.
	char *Pointer_String_User_Name; //!< Set to NULL if no user name is provided.
	char *Pointer_String_Password; //!< Set to NULL if no password is provided.
	int Is_Clean_Session_Enabled; //!< Set to 1 to tell the server to clean any previous saved state.
	unsigned short Keep_Alive;
	// TODO : add will support
	void *Pointer_Buffer; //!< The buffer in which messages will be forged. Make sure it is big enough.
} TMQTTConnectionParameters;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Create a CONNECT packet to send to the server.
 * @param Pointer_Context On output, context will be fully initialized using provided parameters. User does not need to initialize anything from this variable.
 * @param Pointer_Connection_Parameters All connection parameters are defined in this structure. See TMQTTConnectionParameters for field details.
 */
void MQTTConnect(TMQTTContext *Pointer_Context, TMQTTConnectionParameters *Pointer_Connection_Parameters);

/** Create a PUBLISH packet to send to the server.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @param Pointer_String_Topic_Name Topic name is mandatory, user must always provide a string.
 * @param Pointer_Application_Message Data to send for the specified topic, it can by binary data. This pointer does not need to be valid if Application_Message_Size is equal to zero.
 * @param Application_Message_Size How many bytes of application message to send. Set to zero if there is no application data.
 */
void MQTTPublish(TMQTTContext *Pointer_Context, char *Pointer_String_Topic_Name, void *Pointer_Application_Message, int Application_Message_Size);

/** Create a DISCONNECT packet to send to the server.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @note Client should close network connection after this packet has been sent.
 */
void MQTTDisconnect(TMQTTContext *Pointer_Context);

#endif
