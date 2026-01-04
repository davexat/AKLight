#ifndef BROKER_H
#define BROKER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

// ========================================
// DEFINICIONES Y CONSTANTES
// ========================================

#define MAX_CLIENTS 10
#define MAX_TOPIC_LEN 62
#define MAX_VALUE_LEN 62

// ========================================
// ESTRUCTURAS
// ========================================

// Estructura de Mensaje (topic value)
typedef struct {
    char type[3];
    char topic[MAX_TOPIC_LEN];
    char value[MAX_VALUE_LEN];
} Message;

// Cliente
typedef struct {
    int fd;
    uint8_t active;
    char topic[MAX_TOPIC_LEN];
    pthread_t thread;
} Client;

// ========================================
// VARIABLES GLOBALES EXTERNAS
// ========================================

// Lista de clientes
extern Client clients[MAX_CLIENTS];

// Semáforo de clientes
extern pthread_mutex_t client_mutex;

// ========================================
// FUNCIONES
// ========================================

// -- Servidor --
int initialize_broker(const char *broker_ip, int port);

// -- Gestión de Clientes --
int find_available_client_index(void);
void init_clients(void);

// -- Gestión de Mensajes --
uint8_t compare_topics(const char *pattern, const char *topic);
void broadcast_message(Message *message);
void process_message(int client_index, char *data);

#endif