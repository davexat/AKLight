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

#define MAX_TOPIC_LEN 62
#define MAX_VALUE_LEN 62

typedef struct {
    char type[3];
    char topic[MAX_TOPIC_LEN];
    char value[MAX_VALUE_LEN];
} Message;

// ========================================
// FUNCIONES
// ========================================

// Conexión
int initialize_connection(const char *broker_ip, int port);
void end_connection(int socket_fd);

// Envía el mensaje formateado (topico/subtopico metrica=valor)
void send_message(int socket_fd, Message *message);

// -- Métricas Simples --
// Escriben el valor en el buffer proporcionado
void get_metric_load(char *buffer);
void get_metric_mem(char *buffer);
void get_metric_threads(char *buffer);

#endif