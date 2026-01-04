#ifndef BROKER_H
#define BROKER_H

#include <stdbool.h>

// ========================================
// ESTRUCTURAS
// ========================================

// Mensaje
typedef struct {
    char topic[128];
    char value[128];
} Message;

// Cliente
typedef struct {
    int fd;
    uint8_t active;
    uint8_t has_topic;
    char topic[128];
    pthread_t thread;
} Client;

// Lista de clientes
extern Client clients[10];

// Semáforo de clientes
extern pthread_mutex_t client_mutex;

// ========================================
// FUNCIONES DE SERVIDOR
// ========================================

// Conexión
int initialize_broker(const char *broker_ip, int port);
void end_broker(int socket_fd);

// ========================================
// FUNCIONES DE GESTIÓN DE CLIENTES
// ========================================

// Encontrar slot libre
int find_free_slot(void);

// Inicializar cliente
void initialize_client(int slot);

// Finalizar cliente
void end_client(int slot);

// ========================================
// FUNCIONES DE GESTIÓN DE MENSAJES
// ========================================

// Comparar tópicos
uint8_t compare_topics(const char *topic1, const char *topic2);

// Envía el mensaje a los clientes correspondientes
void broadcast_message(const Message *message);

#endif