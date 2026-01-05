#ifndef PRODUCER_H
#define PRODUCER_H

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

// Control de ejecución del producer
extern volatile sig_atomic_t running;

// ========================================
// FUNCIONES
// ========================================

// Manejador de señales (SIGINT)
void signal_handler(int signum);

// Conexión al broker
int initialize_connection(const char *broker_ip, int port);
void end_connection(int socket_fd);

// Envío de mensajes
void send_string(int socket_fd, const char *message);

// Métricas del sistema (formato: "metric=value")
void get_metric_load(char *buffer, size_t size);
void get_metric_mem(char *buffer, size_t size);
void get_metric_threads(char *buffer, size_t size);
void get_metric_uptime(char *buffer, size_t size);
void get_metric_cpus(char *buffer, size_t size);

#endif