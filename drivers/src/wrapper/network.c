//
// Created by cout970 on 2017-07-15.
//

#include "network.h"

Boolean network_is_internet_allowed(NetworkCard *network) {
    return network->internetAllowed;
}

Int network_get_max_sockets(NetworkCard *network) {
    return network->maxSockets;
}

Int network_get_active_sockets(NetworkCard *network) {
    return network->activeSockets;
}

Int network_get_mac(NetworkCard *network) {
    return network->targetMac;
}

Int network_get_target_mac(NetworkCard *network) {
    return network->targetMac;
}

void network_set_target_mac(NetworkCard *network, Int mac) {
    ((NetworkCard volatile *) network)->targetMac = mac;
}

void network_set_target_ip(NetworkCard *network, const String *ip) {
    Int lenPlusOne = 0, i;
    while (ip[lenPlusOne++]);

    if (lenPlusOne == 1 || lenPlusOne - 1 > NETWORK_IP_MAX_SIZE) {
        return;
    }

    char volatile *dst = network->targetIp;
    for (i = 0; i < lenPlusOne; i++) {
        dst[i] = ip[i];
    }
}

void network_set_target_port(NetworkCard *network, Short port) {
    ((NetworkCard volatile *) network)->targetPort = port;
}

Boolean network_is_connection_open(NetworkCard *network) {
    return (Boolean) network->connectionOpen;
}

Int network_get_connection_error(NetworkCard *network) {
    return network->connectionError;
}

Int network_get_input_pointer(NetworkCard *network) {
    return network->inputBufferPtr;
}

void network_set_input_pointer(NetworkCard *network, Int ptr) {
    ((NetworkCard volatile *) network)->inputBufferPtr = ptr;
}

Byte *network_get_input_buffer(NetworkCard *network) {
    return network->inputBuffer;
}

Int network_get_output_pointer(NetworkCard *network) {
    return network->outputBufferPtr;
}

void network_set_output_pointer(NetworkCard *network, Int ptr) {
    ((NetworkCard volatile *) network)->outputBufferPtr = ptr;
}

Byte volatile *network_get_output_buffer(NetworkCard *network) {
    return network->outputBuffer;
}
