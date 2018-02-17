//
// Created by cout970 on 2017-07-15.
//

#include <math.h>
#include <string.h>
#include "network.h"

void network_signal(NetworkCard* network, Byte signal) {
    ((NetworkCard volatile*) network)->signal = signal;
}

Boolean network_is_internet_allowed(NetworkCard* network) {
    return network->internetAllowed;
}

Int network_get_max_sockets(NetworkCard* network) {
    return network->maxSockets;
}

Int network_get_active_sockets(NetworkCard* network) {
    return network->activeSockets;
}

Int network_get_mac(NetworkCard* network) {
    return network->targetMac;
}

Int network_get_target_mac(NetworkCard* network) {
    return network->targetMac;
}

void network_set_target_mac(NetworkCard* network, Int mac) {
    ((NetworkCard volatile*) network)->targetMac = mac;
}

void network_set_target_ip(NetworkCard* network, const String *ip) {
    Int lenPlusOne = 0, i;
    while (ip[lenPlusOne++]);

    if (lenPlusOne == 1 || lenPlusOne - 1 > NETWORK_IP_MAX_SIZE) {
        return;
    }

    volatile char *dst = network->targetIp;
    for (i = 0; i < lenPlusOne; i++) {
        dst[i] = ip[i];
    }
}

void network_set_target_port(NetworkCard* network, Short port) {
    ((NetworkCard volatile*) network)->targetPort = port;
}

Boolean network_is_connection_open(NetworkCard* network) {
    return (Boolean) network->connectionOpen;
}

Int network_get_connection_error(NetworkCard* network) {
    return network->connectionError;
}

Int network_send(NetworkCard *network, ByteBuffer data, Int size) {
    network->hardwareLock = 1;
    // Bytes to copy into the buffer
    Int toMove = MIN(size, NETWORK_BUFFER_MAX_SIZE - network->outputBufferPtr);
    // Copy data to buffer
    memcpy(network->outputBuffer + network->outputBufferPtr, data, (UInt) toMove);
    // Increase buffer size
    network->outputBufferPtr += toMove;
    network->hardwareLock = 0;
    return toMove;
}

Int network_receive(NetworkCard *network, ByteBuffer data, Int size) {
    network->hardwareLock = 1;
    // Bytes to copy into the buffer
    Int toMove = MIN(size, network->inputBufferPtr);
    // Data that will remain in the internal buffer
    Int missingData = MAX(0, network->inputBufferPtr - toMove);

    // Copy data to buffer
    memcpy(data, network->inputBuffer, (UInt) toMove);

    // Move remaining data to the start of the buffer
    memcpy(network->inputBuffer, network->inputBuffer + toMove, (UInt) missingData);

    // Size of the buffer is the amount of bytes not moved
    network->inputBufferPtr = missingData;
    network->hardwareLock = 0;
    return toMove;
}
