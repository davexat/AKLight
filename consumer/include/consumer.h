#ifndef CONSUMER_H
#define CONSUMER_H

// ========================================
// INCLUDES DEL SISTEMA
// ========================================

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// ========================================
// VARIABLES GLOBALES
// ========================================

// Control de ejecución del consumer
extern volatile sig_atomic_t running;
extern int consumer_fd_global;

// ========================================
// FUNCIONES
// ========================================

// Manejador de señales (SIGINT)
void signal_handler(int signum);

// Conexión al broker
int initialize_connection(const char *broker_ip, int port);
void end_connection(int socket_fd);

// Suscripción y recepción
void subscribe_to_topic(int socket_fd, const char *topic);
void listen_messages(int socket_fd);

#endif
