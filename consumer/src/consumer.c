#include "../include/consumer.h"
#include "../../common/include/network.h"
#include "../../common/include/common.h"

// ========================================
// VARIABLES GLOBALES
// ========================================

volatile sig_atomic_t running = 1;
int consumer_fd_global = -1;

// ========================================
// MANEJADOR DE SEÑALES
// ========================================

void signal_handler(int signum) {
    (void)signum;
    running = 0;
    printf("\n\nSeñal recibida, cerrando consumer...\n");
    if (consumer_fd_global >= 0) {
        close(consumer_fd_global);
    }
}

// ========================================
// SUSCRIPCIÓN
// ========================================

void subscribe_to_topic(int socket_fd, const char *topic) {
    char sub_msg[TOPIC_SIZE];
    snprintf(sub_msg, sizeof(sub_msg), "SUB %s\n", topic);
    
    ssize_t sent = send(socket_fd, sub_msg, strlen(sub_msg), 0);
    if (sent == -1) {
        perror("send");
        return;
    }
    
    printf("Suscrito a: %s\n", topic);
    fflush(stdout);
}

// ========================================
// RECEPCIÓN DE MENSAJES
// ========================================

void listen_messages(int socket_fd) {
    char buffer[LARGE_BUFFER_SIZE];
    char line_buffer[LARGE_BUFFER_SIZE];
    size_t line_pos = 0;
    
    printf("\n=== Esperando mensajes (Ctrl+C para salir) ===\n\n");
    fflush(stdout);
    
    while (running) {
        ssize_t n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (n == 0) {
                printf("\nBroker cerró la conexión\n");
            } else {
                if (running) {
                    perror("recv");
                }
            }
            break;
        }
        
        buffer[n] = '\0';
        
        // Procesar byte por byte buscando '\n'
        for (ssize_t i = 0; i < n; i++) {
            char c = buffer[i];
            
            if (c == '\n') {
                // Línea completa
                line_buffer[line_pos] = '\0';
                
                // Mostrar mensaje recibido
                printf("%s\n", line_buffer);
                fflush(stdout);
                
                line_pos = 0;
            } else if (c != '\r') {
                if (line_pos < sizeof(line_buffer) - 1) {
                    line_buffer[line_pos++] = c;
                }
            }
        }
    }
}

// ========================================
// FUNCIÓN PRINCIPAL
// ========================================

int main(int argc, char *argv[]) {
    // Configurar manejadores de señales
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    // 1. Parsear argumentos
    if (argc < 4) {
        printf("Uso: %s <ip_broker> <puerto> <topico>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 9092 metrics/#\n", argv[0]);
        printf("\nWildcard:\n");
        printf("  # = Múltiples niveles\n");
        return 1;
    }

    char *broker_ip = argv[1];
    int port = atoi(argv[2]);
    char *topic = argv[3];

    // 2. Conectar al broker
    int socket_fd = initialize_connection(broker_ip, port);
    if (socket_fd == -1) {
        fprintf(stderr, "No se pudo conectar al broker. Abortando...\n");
        return 1;
    }
    consumer_fd_global = socket_fd;

    // 3. Suscribirse al tópico
    subscribe_to_topic(socket_fd, topic);

    // 4. Escuchar mensajes (loop infinito)
    listen_messages(socket_fd);

    // 5. Limpiar
    printf("\n");
    end_connection(socket_fd);
    printf("Consumer cerrado\n");
    return 0;
}
