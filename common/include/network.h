#ifndef NETWORK_H
#define NETWORK_H

// ========================================
// INCLUDES DEL SISTEMA
// ========================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// ========================================
// FUNCIONES DE RED COMPARTIDAS
// ========================================

// Inicializar conexión TCP a un broker
// Retorna: file descriptor del socket, o -1 en caso de error
int initialize_connection(const char *broker_ip, int port);

// Cerrar conexión
void end_connection(int socket_fd);

#endif
