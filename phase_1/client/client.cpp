/*
 * Client.cpp
 *
 *  Created on: 2013-02-16
 *      Author: setareh
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <pthread.h>
#include "client.h"
#include <jansson.h>

#define PORT 9988 // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}
	create_socket(argv);

	return 0;
}
void create_socket(char *argv[]) {
	char* serverName = argv[1];

	// STEP 1: create a socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror("client: socket");
		exit(-1);
	}

	// STEP 2: find server address
	struct hostent* serverHost = gethostbyname(serverName);

	if (serverHost == NULL) {
		herror("client: gethostbyname");
		exit(-1);
	}

	// STEP 3: connect to server
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	memcpy(&(serverAddr.sin_addr), serverHost->h_addr, serverHost->h_length);
	serverAddr.sin_port = PORT;

	printf("client: host name '%s' resolved to '%s'\n", serverName, inet_ntoa(
			serverAddr.sin_addr));

	int ret = connect(sockfd, ((struct sockaddr *) &serverAddr),
			sizeof(struct sockaddr));

	if (ret != 0) {
		close(sockfd);
		perror("client: connect");
		exit(-1);
	}

	printf("client: successfully connected to %s:%d\n", serverName, PORT);
	pthread_t sender, reciever;

	pthread_create(&sender, NULL, send_message, &sockfd);
	pthread_create(&reciever, NULL, recv_message, &sockfd);
	//pthread_create(&hrst_calculator, NULL, &msfm, NULL);

	//
	pthread_join(sender, NULL);
	pthread_join(reciever, NULL);
	//pthread_join(hrst_calculator, NULL);

	//	pid_t pid = fork();
	//	// STEP 4: communicate (send)
	//	if (pid != 0) {
	//		send_message(sockfd);
	//
	//	} else {
	//		recv_message(sockfd);
	//	}

	// STEP 5: closing connection
	//close(sockfd);
	shutdown(sockfd, SHUT_RD);

	printf("client: connection successfully finished.\n");
}
void* send_message(void * arg) {
	int sockfd = *(int*) arg;
	int ret;
	while (1) {

		char message[MAXDATASIZE];
		char s[MAXDATASIZE];
		gets(s);
		strncpy(message, s, MAXDATASIZE - 1);

		char *sendBuf = message;
		if ((strcmp(message, "exit")) == 0) {
			break;
		}
		printf("client: '%s'\n", message);

		// write the message until it is written completely.
		do {
			ret = write(sockfd, sendBuf, strlen(sendBuf));
			if (ret > 0) {
				sendBuf += ret;
				printf("client: %d bytes sent.\n", ret);
			}
		} while (ret > 0 && strlen(sendBuf) > 0);

		if (ret < 0) {
			perror("client: write");
			close(sockfd);
			//exit(-1);
		}

		printf("client: sent.\n");
	}
}
void* recv_message(void* arg) {
	int sockfd = *(int*) arg;
	char message[MAXDATASIZE];
	int ret;

	// Read until the client closes the connection
	while ((ret = read(sockfd, message, MAXDATASIZE - 1)) > 0) {
		message[ret] = '\0';
		if ((strcmp(message, "exit")) == 0) {
			break;
		}
		printf("server: '%s'\n", message);
	}

	if (ret < 0) {
		perror("server: read");
	}

	close(sockfd);
}
/* ********Message Types********
 0 – SUBSCRIBE
 1 – UNSUBSCRIBE
 2 – QUERY
 4 – SENSOR LIST
 5 – KEEPALIVE
 6 – ACK
 7 – ERROR
 CRITICAL MESSAGES:
 3 – UPDATE
 8 - REPORT*/

void process_message(byte * data) {
	byte type = (data[0] >> 4) && 0x01;
	byte version = data[0] & 0x01;
	if (version == 0x01) {
		if (type == 0x04) {			// sensor list


		} else if (type == 0x03) {		//update

		} else if (type == 0x07) {	// error

		}

	} else if (version == 0x02) {

	}

}

/***
 * version 1 packet header
 * +==========+============+==========+=========+
   | Type (4) |Version (4) |    Options (8)     |
   +==========+============+==========+=========+
   |             Payload Length (16)            |
   +=======================+====================+
 *
 *
 */
void subscribe_v1(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x01;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;	//payload length
	memcpy(data+4,payload,(size_t)length);

}
void unsubscribe_v1(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x11;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;	//payload length
	if(length!=0){
		memcpy(data+4,payload,(size_t)length);
	}

}
void ack_v1(byte * data) {
	data[0] = 0x61;		//type+version
	data[1] = 0x00;		//options
	bzero(data + 2, 6);
	//Q: use cliend id in the next header?
	//Q: where is the ack number?
}
void keepalive_v1(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x51;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;
	bzero(data + 4, 4);	//payload length
	memcpy(data+8,payload,(size_t)length);
	//Q: use cliend id in the next header?
}
void query_v1(byte * data) {
	data[0] = 0x21;
	data[1] = 0x00;
	bzero(data + 2, 2);
}




/*
 *
+==========+=========+==========+=========+============================+
| Type (4) | Version (4) |Options (8)|         Payload Length (16)     |
+==========+=========+==========+=========+============================+
|       Identification (16)          |             Offset (16)         |
+====================+====================+============================+
 *
 */
void subscribe_v2(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x02;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;
	bzero(data + 4, 4);	//payload length
	memcpy(data+8,payload,(size_t)length);

}
void unsubscribe_v2(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x12;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;
	bzero(data + 4, 4);	//payload length
	if(length!=0){
		memcpy(data+8,payload,(size_t)length);
	}

}
void ack_v2(byte * data) {
	data[0] = 0x62;		//type+version
	data[1] = 0x00;		//options
	bzero(data + 2, 6);
	//Q: use cliend id in the next header?
	//Q: where is the ack number?
}
void keepalive_v2(byte * data, const byte * payload, uint16 length) {
	data[0] = 0x52;		//type+version
	data[1] = 0x00;		//options
	uint16 *data2 = reinterpret_cast<uint16 *> (&data[2]);
	data2[0]=length;
	bzero(data + 4, 4);	//payload length
	memcpy(data+8,payload,(size_t)length);
	//Q: use cliend id in the next header?
}
void query_v2(byte * data) {
	data[0] = 0x22;
	data[1] = 0x00;
	bzero(data + 2, 6);
	//uint16 *data4 = reinterpret_cast<uint16 *> (&frame1.data[12]);
}

