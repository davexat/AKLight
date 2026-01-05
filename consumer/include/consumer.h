#ifndef CONSUMER_H
#define CONSUMER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// ========================================
// FUNCIONES
// ========================================

// Conexión
int initialize_connection(const char *broker_ip, int port);
void end_connection(int socket_fd);

// Suscripción
void subscribe_to_topic(int socket_fd, const char *topic);

// Recepción de mensajes
void listen_messages(int socket_fd);

#endif
