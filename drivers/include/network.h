//
// Created by cout970 on 2017-07-15.
//

#ifndef DRIVER_NETWORK_H
#define DRIVER_NETWORK_H

#include "devices.h"
#include "types.h"

struct network_card_header {
/*    0 0x000 */    struct device_header header;
/*    4 0x004 */    const Byte internetAllowed;
/*    5 0x005 */    const Byte maxSockets;
/*    6 0x006 */    const Byte activeSockets;
/*    7 0x007 */    Byte signal;
/*    8 0x008 */    const Int macAddress;
/*   12 0x00c */    Int targetMac;
/*   16 0x010 */    Int targetPort;
/*   20 0x014 */    Byte targetIp[80];
/*  100 0x064 */    const Int connectionOpen;
/*  104 0x068 */    const Int connectionError;
/*  108 0x06c */    Int inputBufferPtr;
/*  112 0x070 */    Int outputBufferPtr;
/*  116 0x074 */    Byte inputBuffer[1024];
/* 1140 0x474 */    Byte outputBuffer[1024];
};

typedef struct network_card_header NetworkCard;

#define NETWORK_IP_MAX_SIZE 80
#define NETWORK_BUFFER_MAX_SIZE 1024

#define NETWORK_ERROR_NONE 0
#define NETWORK_ERROR_INVALID_PORT 1
#define NETWORK_ERROR_INVALID_IP_SIZE 2
#define NETWORK_ERROR_EXCEPTION_PARSING_IP 3
#define NETWORK_ERROR_EXCEPTION_OPEN_SOCKET 4
#define NETWORK_ERROR_INTERNET_NOT_ALLOWED 5
#define NETWORK_ERROR_MAX_SOCKET_REACH 6
#define NETWORK_ERROR_SOCKET_CLOSED 7
#define NETWORK_ERROR_UNABLE_TO_READ_PACKET 8
#define NETWORK_ERROR_UNABLE_TO_SEND_PACKET 9
#define NETWORK_ERROR_INVALID_OUTPUT_BUFFER_POINTER 10
#define NETWORK_ERROR_INVALID_INPUT_BUFFER_POINTER 11

#define NETWORK_SIGNAL_OPEN_TCP_CONNECTION 1
#define NETWORK_SIGNAL_CLOSE_TCP_CONNECTION 2
#define NETWORK_SIGNAL_OPEN_SSL_TCP_CONNECTION 3

void network_signal(NetworkCard *network, Byte signal);

Boolean network_is_internet_allowed(NetworkCard *network);

Int network_get_max_sockets(NetworkCard *network);

Int network_get_active_sockets(NetworkCard *network);

Int network_get_mac(NetworkCard *network);

Int network_get_target_mac(NetworkCard *network);

void network_set_target_mac(NetworkCard *network, Int mac);

void network_set_target_ip(NetworkCard *network, const String *ip);

void network_set_target_port(NetworkCard *network, Short port);

Boolean network_is_connection_open(NetworkCard *network);

Int network_get_connection_error(NetworkCard *network);

Int network_get_input_pointer(NetworkCard *network);

void network_set_input_pointer(NetworkCard *network, Int ptr);

Byte *network_get_input_buffer(NetworkCard *network);

Int network_get_output_pointer(NetworkCard *network);

void network_set_output_pointer(NetworkCard *network, Int ptr);

Byte volatile *network_get_output_buffer(NetworkCard *network);

#endif //DRIVER_NETWORK_H
