#ifndef PRODUCER_H
#define PRODUCER_H

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

// Envía el mensaje al broker con el formato (PUB topic/subtopic metric=value)
void send_string(int socket_fd, const char *message);

// Métricas - devuelven string formateado "metric=value"
void get_metric_load(char *buffer, size_t size);
void get_metric_mem(char *buffer, size_t size);
void get_metric_threads(char *buffer, size_t size);
void get_metric_uptime(char *buffer, size_t size);
void get_metric_cpus(char *buffer, size_t size);

#endif