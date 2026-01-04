#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../include/broker.h"

// ========================================
// VARIABLES GLOBALES
// ========================================

// Lista de clientes
Client clients[MAX_CLIENTS];

// Semáforo de clientes
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

// ========================================
// FUNCIONES DE UTILIDAD
// ========================================

// Inicializar arreglo de clientes
void init_clients(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].active = 0;
        clients[i].type = CLIENT_UNKNOWN;
        clients[i].subscription_count = 0;
    }
}

// Buscar slot disponible para nuevo cliente
int find_available_client_index(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            return i;
        }
    }
    return -1;
}

// Comparar tópicos retorna 1 si coinciden, 0 si no
uint8_t compare_topics(const char *pattern, const char *topic) {
    // Caso exacto
    if (strcmp(pattern, topic) == 0) return 1;

    // Caso wildcard '#'
    size_t len_p = strlen(pattern);
    
    // Verificar si termina en '#'
    if (len_p > 0 && pattern[len_p - 1] == '#') {
        // Verificar prefijo antes del '#'
        if (len_p == 1) return 1;
        
        // Si pattern es "prefix/#", verificar que topic empiece con "prefix/"
        char prefix[128];
        strncpy(prefix, pattern, len_p - 1);
        prefix[len_p - 1] = '\0';
        
        // Verificamos si comienza con el prefijo
        if (strncmp(topic, prefix, strlen(prefix)) == 0) {
            return 1;
        }
    }
    
    return 0;
}

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

// Enviar mensaje a clientes suscritos
void broadcast_message(Message *message) {
    pthread_mutex_lock(&client_mutex);
    
    printf("Intentando difundir mensaje Tópico: '%s', Valor: '%s'\n", message->topic, message->value);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Solo enviar a clientes activos
        if (clients[i].active) {
            // Verificar suscripción
            int match = 0;
            
            if (compare_topics(clients[i].topic, message->topic)) match = 1;
            
            if (match) {
                // Preparar mensaje de datos
                Message out_msg = *message;
                
                if (send(clients[i].fd, &out_msg, sizeof(Message), 0) == -1) {
                    perror("Error enviando mensaje a consumidor");
                } else {
                    printf("Enviado a Cliente %d\n", i);
                }
            }
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

// Procesar mensaje recbido de un cliente
void process_message(int index, char *data) {
    Message msg;
    
    char *saveptr;
    char *line = strtok_r(data, " ", &saveptr);

    
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

    while (1) {
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_read <= 0) break;

        if (bytes_read == sizeof(buffer)) {
            process_message(index, buffer);
        } else {
            printf("Advertencia: Paquete incompleto recibido de Cliente %d\n", index);
        }
    }

    // Limpieza del cliente
    close(client_fd);
    pthread_mutex_lock(&client_mutex);
    clients[index].fd = -1;
    clients[index].thread = 0;
    clients[index].active = 0;
    clients[index].type = CLIENT_UNKNOWN;
    clients[index].subscription_count = 0;
    pthread_mutex_unlock(&client_mutex);

    printf("Cliente %d desconectado\n", index);
    return NULL;
}

// ========================================
// FUNCIÓN PRINCIPAL
// ========================================
int main(int argc, char *argv[]) {
    // 0. Inicializar clientes
    init_clients();

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