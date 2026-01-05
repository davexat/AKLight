#include "../include/consumer.h"

// ========================================
// CONEXIÓN
// ========================================

int initialize_connection(const char *broker_ip, int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, broker_ip, &addr.sin_addr) != 1) {
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        int ret = getaddrinfo(broker_ip, NULL, &hints, &result);
        if (ret != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
            close(socket_fd);
            return -1;
        }
        
        struct sockaddr_in *resolved = (struct sockaddr_in *)result->ai_addr;
        addr.sin_addr = resolved->sin_addr;
        freeaddrinfo(result);
    }

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(socket_fd);
        return -1;
    }

    printf("Conectado al broker en %s:%d\n", broker_ip, port);
    return socket_fd;
}

void end_connection(int socket_fd) {
    if (socket_fd >= 0) {
        close(socket_fd);
        printf("Conexión cerrada\n");
    }
}

// ========================================
// SUSCRIPCIÓN
// ========================================

void subscribe_to_topic(int socket_fd, const char *topic) {
    char sub_msg[128];
    snprintf(sub_msg, sizeof(sub_msg), "SUB %s\n", topic);
    
    ssize_t sent = send(socket_fd, sub_msg, strlen(sub_msg), 0);
    if (sent == -1) {
        perror("send");
        return;
    }
    
    printf("Suscrito a: %s\n", topic);
}

// ========================================
// RECEPCIÓN DE MENSAJES
// ========================================

void listen_messages(int socket_fd) {
    char buffer[512];
    char line_buffer[512];
    size_t line_pos = 0;
    
    printf("\n=== Esperando mensajes ===\n\n");
    
    while (1) {
        ssize_t n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (n == 0) {
                printf("\nBroker cerró la conexión\n");
            } else {
                perror("recv");
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
    if (socket_fd == -1) return 1;

    // 3. Suscribirse al tópico
    subscribe_to_topic(socket_fd, topic);

    // 4. Escuchar mensajes (loop infinito)
    listen_messages(socket_fd);

    // 5. Limpiar
    end_connection(socket_fd);
    return 0;
}
