#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// ========================================
// ESTRUCTURA DEL MENSAJE
// ========================================

typedef struct {
    char topic[128];
    char key[64];
    char value[256];  // Aumentado para métricas más largas
} Message;

// ========================================
// FUNCIONES
// ========================================

// Conexión
int initialize_connection(const char *broker_ip, int port);
void end_connection(int socket_fd);

// Envía el mensaje formateado (clave topico/subtopico1/.../mensaje)
void send_message(int socket_fd, const Message *message);

// Metricas - cada una crea y envía su propio mensaje
void get_cpu_usage(int socket_fd, const char *topic, const char *key);
void get_memory_usage(int socket_fd, const char *topic, const char *key);
void get_process_count(int socket_fd, const char *topic, const char *key);
void get_uptime(int socket_fd, const char *topic, const char *key);
void get_cpu_count(int socket_fd, const char *topic, const char *key);

#endif