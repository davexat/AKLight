#include "../include/network.h"

// ========================================
// CONEXIÓN TCP
// ========================================

int initialize_connection(const char *broker_ip, int port) {
    printf("Intentando conectar al broker en %s:%d...\n", broker_ip, port);
    
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
            fprintf(stderr, "No se pudo resolver el host del broker: %s\n", broker_ip);
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
