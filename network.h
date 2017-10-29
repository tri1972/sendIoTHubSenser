#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>

#include <string.h> /* for strncpy */
#include <unistd.h> /* for close */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define INTERFACE "wlan0"

char * getIpAddress();

#endif
