#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdint.h>

// ========================================
// ESTRUCTURA DEL MENSAJE
// ========================================

typedef struct {
    char topic[128];
    char key[64];
    char value[64];
} Message;

// ========================================
// FUNCIONES
// ========================================

// Conexión
void initialize_connection();
void end_connection();

// Envía el mensaje formateado (clave topico/subtopico1/.../mensaje)
void send_message(const Message *message);

// Metricas
void get_cpu_usage();
void get_memory_usage();
void get_disk_usage();
void get_network_usage();
void get_process_usage();

#endif