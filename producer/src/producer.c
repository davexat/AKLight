#include "../include/producer.h"

// ========================================
// CONEXIÓN
// ========================================

int initialize_connection(int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return -1;
    }

    return socket_fd;
}

void end_connection(int socket_fd) {
    close(socket_fd);
}

// ========================================
// ENVÍO DE MENSAJE
// ========================================

void send_message(const Message *message) {
    
}


int main(int argc, char *argv[]) {
    // 1. Parsear argumentos
    if (argc < 4){
        printf("Uso: %s <ip> <puerto> <clave> <topico> <metrica1> <metrica2>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 9092 1234 usage/cpudisk 0 2\n", argv[0]);
        printf("Metricas: 0 = CPU, 1 = Memoria, 2 = Disco, 3 = Red, 4 = Procesos\n");
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *key = argv[3];
    char *topic = argv[4];
    int metric = atoi(argv[5]);

    if (metric < 0 || metric > 4) {
        printf("Metrica invalida\n");
        return -1;
    }

    // 2. Inicializar conexión
    int socket_fd = initialize_connection(port);
    if (socket_fd == -1) {
        return -1;
    }

    // 3. Bucle de envío de mensajes
    Message message;
    memset(&message, 0, sizeof(message));
    
    while (1) {
        memset(&message, 0, sizeof(message));

        switch (metric) {
            case 0:
                message = get_cpu_usage();
                break;
            case 1:
                message = get_memory_usage();
                break;
            case 2:
                message = get_disk_usage();
                break;
            case 3:
                message = get_network_usage();
                break;
            case 4:
                message = get_process_usage();
                break;
        }

        send_message(&message);
        sleep(3);
    }

    end_connection(socket_fd);
    return 0;
}