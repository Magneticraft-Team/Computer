//
// Created by cout970 on 2017-07-15.
//

#ifndef DRIVER_NETWORK_H
#define DRIVER_NETWORK_H

#include "devices.h"
#include "types.h"

struct network_header {
/*    0 0x000 */    struct device_header header;
/*    4 0x004 */    const i8 internetAllowed;
/*    5 0x005 */    const i8 maxSockets;
/*    6 0x006 */    const i8 activeSockets;
/*    7 0x007 */    i8 signal;
/*    8 0x008 */    const i32 macAddress;
/*   12 0x00c */    i32 targetMac;
/*   16 0x010 */    i32 targetPort;
/*   20 0x014 */    i8 targetIp[80];
/*  100 0x064 */    const i32 connectionOpen;
/*  104 0x068 */    const i32 connectionError;
/*  108 0x06c */    i32 inputBufferPtr;
/*  112 0x070 */    i32 outputBufferPtr;
/*  116 0x074 */    i8 inputBuffer[1024];
/* 1140 0x474 */    i8 outputBuffer[1024];
};

typedef struct network_header *Network;

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

void network_signal(Network network, i8 signal);

boolean network_is_internet_allowed(Network network);

i32 network_get_max_sockets(Network network);

i32 network_get_active_sockets(Network network);

i32 network_get_mac(Network network);

i32 network_get_target_mac(Network network);

void network_set_target_mac(Network network, i32 mac);

void network_set_target_ip(Network network, const char *ip);

void network_set_target_port(Network network, i16 port);

boolean network_is_connection_open(Network network);

i32 network_get_connection_error(Network network);

i32 network_get_input_pointer(Network network);

void network_set_input_pointer(Network network, i32 ptr);

i8 *network_get_input_buffer(Network network);

i32 network_get_output_pointer(Network network);

void network_set_output_pointer(Network network, i32 ptr);

i8 *network_get_output_buffer(Network network);

#endif //DRIVER_NETWORK_H
