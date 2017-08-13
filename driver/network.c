//
// Created by cout970 on 2017-07-15.
//

#include "api/network.h"


void network_signal(Network network, i8 signal) {
    ((Network volatile) network)->signal = signal;
}

boolean network_is_internet_allowed(Network network) {
    return network->internetAllowed;
}

i32 network_get_max_sockets(Network network) {
    return network->maxSockets;
}

i32 network_get_active_sockets(Network network) {
    return network->activeSockets;
}

i32 network_get_mac(Network network) {
    return network->targetMac;
}

i32 network_get_target_mac(Network network) {
    return network->targetMac;
}

void network_set_target_mac(Network network, i32 mac) {
    ((Network volatile) network)->targetMac = mac;
}

void network_set_target_ip(Network network, const char *ip) {
    i32 lenPlusOne = 0, i;
    while (ip[lenPlusOne++]);

    if (lenPlusOne == 1 || lenPlusOne - 1 > NETWORK_IP_MAX_SIZE) {
        return;
    }

    volatile char *dst = network->targetIp;
    for (i = 0; i < lenPlusOne; i++) {
        dst[i] = ip[i];
    }
}

void network_set_target_port(Network network, i16 port) {
    ((Network volatile) network)->targetPort = port;
}

boolean network_is_connection_open(Network network) {
    return network->connectionOpen;
}

i32 network_get_connection_error(Network network) {
    return network->connectionError;
}

i32 network_get_input_pointer(Network network) {
    return network->inputBufferPtr;
}

void network_set_input_pointer(Network network, i32 ptr) {
    ((Network volatile) network)->inputBufferPtr = ptr;
}

i8 *network_get_input_buffer(Network network) {
    return network->inputBuffer;
}

i32 network_get_output_pointer(Network network) {
    return network->outputBufferPtr;
}

void network_set_output_pointer(Network network, i32 ptr) {
    ((Network volatile) network)->outputBufferPtr = ptr;
}

i8 *network_get_output_buffer(Network network) {
    return network->outputBuffer;
}
