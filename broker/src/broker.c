#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../include/broker.h"

#define MAX_CLIENTS 10

// ========================================
// VARIABLES GLOBALES
// ========================================

// Lista de clientes
Client clients[MAX_CLIENTS];

// Semáforo de clientes
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

// ========================================
// FUNCIONES DEL SERVIDOR
// ========================================

// Inicializar broker
int initialize_broker(const char *broker_ip, int port) {
    // 1. Crear el descriptor de archivo para el socket (IPv4, TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error al crear el socket");
        return -1;
    }

    // 2. Configurar la estructura de dirección del servidor
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // Convertir y asignar la IP proporcionada
    if (inet_pton(AF_INET, broker_ip, &address.sin_addr) <= 0) {
        perror("Dirección IP inválida o no soportada");
        close(server_fd);
        return -1;
    }

    // 3. Configurar opciones del socket para reutilizarlo
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error al configurar SO_REUSEADDR");
        close(server_fd);
        return -1;
    }

    // 4. Vincular el socket a la IP y puerto especificados
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Falló el bind (puerto en uso o permiso denegado)");
        close(server_fd);
        return -1;
    }

    // 5. Poner el socket en modo 'escucha' para aceptar conexiones entrantes
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Falló listen");
        close(server_fd);
        return -1;
    }

    printf("Broker inicializado y escuchando en %s:%d\n", broker_ip, port);
    return server_fd;
}

// Procesar mensaje
void process_message(int index, char *buffer) {
    
}

// Atender al cliente conectado
void *handle_client(void *arg) {
    int index = *(int *)arg;
    free(arg);

    char buffer[128];
    ssize_t bytes_read;

    pthread_mutex_lock(&client_mutex);
    int client_fd = clients[index].fd;
    clients[index].active = 1;
    pthread_mutex_unlock(&client_mutex);

    // Recepción de comandos
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';

        // Imprimir mensaje recibido
        printf("Mensaje de cliente %d recibido: %s\n", index, buffer);
        
        // Procesar mensaje
        process_message(index, buffer);
    }

    // Limpieza del cliente
    close(client_fd);
    pthread_mutex_lock(&client_mutex);
    clients[index].fd = -1;
    clients[index].thread = 0;
    clients[index].active = 0;
    clients[index].has_topic = 0;
    memset(clients[index].topic, 0, sizeof(clients[index].topic));
    pthread_mutex_unlock(&client_mutex);

    printf("Cliente %d desconectado\n", index);
    return NULL;
}

// ========================================
// FUNCIÓN PRINCIPAL
// ========================================
int main(int argc, char *argv[]) {
    // 1. Parsear argumentos
    if (argc < 3) {
        printf("Uso: %s <ip_broker> <puerto>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 9092\n", argv[0]);
        return 1;
    }

    // 2. Inicializar broker
    int server_fd = initialize_broker(argv[1], atoi(argv[2]));
    if (server_fd == -1) return 1;
    printf("Broker inicializado, esperando conexiones...\n");

    // 3. Bucle principal
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (new_fd < 0) {
            perror("Error al aceptar una conexión");
            continue;
        }

        printf("Conexión aceptada de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Agregar cliente a la lista de clientes
        pthread_mutex_lock(&client_mutex);
        int index = find_available_client_index();
        if (index == -1) {
            pthread_mutex_unlock(&client_mutex);
            perror("Error: No hay espacio para más clientes");
            close(new_fd);
            continue;
        }
        
        clients[index].fd = new_fd;
        pthread_mutex_unlock(&client_mutex);

        // Crear argumento para el hilo
        int *arg = malloc(sizeof(int));
        *arg = index;

        // Crear hilo para el cliente
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, arg) != 0) {
            perror("Error al crear el hilo para el cliente");
            free(arg);
            close(new_fd);

            pthread_mutex_lock(&client_mutex);
            clients[index].fd = -1;
            pthread_mutex_unlock(&client_mutex);
            continue;
        }

        // Guardar thread y liberar recursos automáticamente
        pthread_detach(thread);

        pthread_mutex_lock(&client_mutex);
        clients[index].thread = thread;
        pthread_mutex_unlock(&client_mutex);
    }

    return 0;
}