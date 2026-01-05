#ifndef BROKER_H
#define BROKER_H

// ========================================
// INCLUDES DEL SISTEMA
// ========================================

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// ========================================
// CONSTANTES
// ========================================

#define MAX_CLIENTS        10
#define MAX_TOPIC_LEN      62
#define MAX_VALUE_LEN      62

// ========================================
// ESTRUCTURAS
// ========================================

// Mensaje publicado/recibido
typedef struct {
    char type[3];                    // Tipo de mensaje (PUB, SUB, MSG)
    char topic[MAX_TOPIC_LEN];       // Tópico del mensaje
    char value[MAX_VALUE_LEN];       // Valor/contenido del mensaje
} Message;

// Cliente conectado al broker
typedef struct {
    int fd;                          // File descriptor (-1 = slot libre)
    char topic[MAX_TOPIC_LEN];       // Tópico suscrito ("" = no suscrito)
    char line_buffer[256];           // Buffer para línea incompleta (TCP stream)
    size_t buffer_pos;               // Posición actual en line_buffer
} Client;

// ========================================
// VARIABLES GLOBALES
// ========================================

// Control de ejecución del broker
extern volatile sig_atomic_t running;
extern int server_fd_global;

// Array de clientes conectados
extern Client clients[MAX_CLIENTS];

// Mutex para proteger acceso a clients[]
extern pthread_mutex_t client_mutex;

// ========================================
// FUNCIONES
// ========================================

// Manejador de señales (SIGINT)
void signal_handler(int signum);

// Inicialización del broker
int initialize_broker(const char *broker_ip, int port);

// Gestión de clientes
int find_available_client_index(void);
void init_clients(void);

// Procesamiento de mensajes
uint8_t compare_topics(const char *pattern, const char *topic);
void broadcast_message(Message *message);

#endif