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
	// Following fields are for internal usage only, do not modify or use
	unsigned char *Pointer_Message_Buffer; //!< This is the real beginning of the message. Use MQTT_GET_MESSAGE_BUFFER() to get this field.
	int Message_Size; //!< How many bytes in the current message. Use MQTT_GET_MESSAGE_SIZE() to get this field.
	unsigned char *Pointer_Buffer; //!< The buffer in which messages are forged. A message does not necessarily start from offset 0. Only Pointer_Message_Buffer pointer tells the message beginning.
} TMQTTContext;

/** Parameters to provide when establishing a MQTT connection to the server. */
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
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** How many bytes are expected for a CONNACK message. */
#define MQTT_CONNACK_MESSAGE_SIZE 4

/** Retrieve a message payload buffer.
 * @param Pointer_Context An initialized MQTT context containing a valid message.
 * @return A pointer on the message beginning.
 */
#define MQTT_GET_MESSAGE_BUFFER(Pointer_Context) (Pointer_Context)->Pointer_Message_Buffer

/** Retrieve a message payload size.
 * @param Pointer_Context An initialized MQTT context containing a valid message.
 * @return The message size in bytes.
 */
#define MQTT_GET_MESSAGE_SIZE(Pointer_Context) (Pointer_Context)->Message_Size

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Create a CONNECT packet to send to the server.
 * @param Pointer_Context On output, context will be fully initialized using provided parameters. User does not need to initialize anything from this variable.
 * @param Pointer_Connection_Parameters All connection parameters are defined in this structure. See TMQTTConnectionParameters for field details.
 */
void MQTTConnect(TMQTTContext *Pointer_Context, TMQTTConnectionParameters *Pointer_Connection_Parameters);

/** Process a CONNACK message received from the server.
 * @param Pointer_Message_Buffer The CONNACK message sent by the server. See notes for detail.
 * @param Message_Size How many bytes of data were read from the server.
 * @return -1 if the message is malformed,
 * @return The server response code (0 or a positive value). A value of 0 tells that connection is granted, all other values indicate an error (see specifications for details).
 * @note After sending a CONNECT message using MQTTConnect(), wait for the server to send a MQTT_CONNACK_MESSAGE_SIZE bytes packet. Then use MQTTIsConnectionEstablished() to retrieve the server response code.
 */
int MQTTIsConnectionEstablished(void *Pointer_Message_Buffer, int Message_Size);

/** Create a PUBLISH packet to send to the server.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @param Pointer_String_Topic_Name Topic name is mandatory, user must always provide a string.
 * @param Pointer_Application_Message Data to send for the specified topic, it can by binary data. This pointer does not need to be valid if Application_Message_Size is equal to zero.
 * @param Application_Message_Size How many bytes of application message to send. Set to zero if there is no application data.
 */
void MQTTPublish(TMQTTContext *Pointer_Context, char *Pointer_String_Topic_Name, void *Pointer_Application_Message, int Application_Message_Size);

/** Create a SUBSCRIBE packet to send to the server.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @param Pointer_String_Topic_Name Topic name is mandatory, user must always provide a string.
 * @note Only one topic can be specified at a time (specifications allow multiple ones). Just send multiple SUBSCRIBE messages to carry the same operation.
 */
void MQTTSubscribe(TMQTTContext *Pointer_Context, char *Pointer_String_Topic_Name);

/** Create a DISCONNECT packet to send to the server.
 * @param Pointer_Context A context previously initialized with a call to MQTTConnect().
 * @note Client should close network connection after this packet has been sent.
 */
void MQTTDisconnect(TMQTTContext *Pointer_Context);

#endif
