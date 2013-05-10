/*
 * client.h
 *
 *  Created on: 2013-02-16
 *      Author: setareh
 */

#ifndef CLIENT_H_
#define CLIENT_H_
typedef unsigned char byte;
typedef unsigned short uint16;
void create_socket( char *argv[]);
void * recv_message(void * arg);
void * send_message(void * arg);
void subscribe(byte * data, const byte * payload, uint16 length);
void query(byte * data);

#endif /* CLIENT_H_ */
