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
        memset(clients[i].topic, 0, MAX_TOPIC_LEN);
        memset(clients[i].line_buffer, 0, sizeof(clients[i].line_buffer));
        clients[i].buffer_pos = 0;
    }
}

// Buscar slot disponible para nuevo cliente
int find_available_client_index(void) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd == -1) return i;
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
        size_t prefix_len = len_p - 1;
        memcpy(prefix, pattern, prefix_len);
        prefix[prefix_len] = '\0';
        
        // Verificamos si comienza con el prefijo
        if (strncmp(topic, prefix, prefix_len) == 0) {
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
    // Formatear mensaje
    char formatted_msg[256];
    snprintf(formatted_msg, sizeof(formatted_msg), "MSG %s %s\n", message->topic, message->value);
    
    // Copiar FDs de clientes interesados bajo mutex
    int target_fds[MAX_CLIENTS];
    int target_count = 0;
    
    pthread_mutex_lock(&client_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1 && clients[i].topic[0] != '\0') {
            if (compare_topics(clients[i].topic, message->topic)) {
                target_fds[target_count++] = clients[i].fd;
            }
        }
    }

    pthread_mutex_unlock(&client_mutex);
    
    for (int i = 0; i < target_count; i++) {
        send(target_fds[i], formatted_msg, strlen(formatted_msg), MSG_NOSIGNAL);
    }
}

// Procesar una línea completa recibida de un cliente
static void handle_client_line(int client_index, char *line) {
    char *saveptr;
    char *cmd = strtok_r(line, " ", &saveptr);
    
    if (!cmd) return;
    
    if (strcmp(cmd, "PUB") == 0) {
        // Formato: PUB topic/subtopic metric=value
        char *topic = strtok_r(NULL, " ", &saveptr);
        char *value = strtok_r(NULL, "", &saveptr);
        
        if (topic && value) {
            Message msg;
            memset(&msg, 0, sizeof(Message));
            memcpy(msg.type, "PUB", 3);
            strncpy(msg.topic, topic, MAX_TOPIC_LEN - 1);
            strncpy(msg.value, value, MAX_VALUE_LEN - 1);
            
            broadcast_message(&msg);
        }
    } 
    else if (strcmp(cmd, "SUB") == 0) {
        char *topic = strtok_r(NULL, " \r\n", &saveptr);
        
        if (topic) {
            pthread_mutex_lock(&client_mutex);
            strncpy(clients[client_index].topic, topic, MAX_TOPIC_LEN - 1);
            clients[client_index].topic[MAX_TOPIC_LEN - 1] = '\0';
            pthread_mutex_unlock(&client_mutex);
            
            // Confirmación fuera de mutex
            char ack[64];
            snprintf(ack, sizeof(ack), "OK %s\n", topic);
            send(clients[client_index].fd, ack, strlen(ack), MSG_NOSIGNAL);
        }
    }
}

// Atender al cliente conectado
void *handle_client(void *arg) {
    int index = *(int *)arg;
    free(arg);
    
    // Copiar FD bajo mutex (ownership transfer)
    pthread_mutex_lock(&client_mutex);
    int client_fd = clients[index].fd;
    pthread_mutex_unlock(&client_mutex);
    
    char recv_buffer[256];
    
    while (1) {
        ssize_t n = recv(client_fd, recv_buffer, sizeof(recv_buffer) - 1, 0);
        if (n <= 0) break;
        
        recv_buffer[n] = '\0';
        
        // Procesar byte por byte buscando '\n'
        for (ssize_t i = 0; i < n; i++) {
            char c = recv_buffer[i];
            
            pthread_mutex_lock(&client_mutex);
            
            // Protección contra buffer overflow
            if (clients[index].buffer_pos >= sizeof(clients[index].line_buffer) - 1) {
                clients[index].buffer_pos = 0;
            }
            
            if (c == '\n') {
                // Línea completa
                clients[index].line_buffer[clients[index].buffer_pos] = '\0';
                char line_copy[256];
                strncpy(line_copy, clients[index].line_buffer, sizeof(line_copy) - 1);
                line_copy[sizeof(line_copy) - 1] = '\0';
                clients[index].buffer_pos = 0;
                pthread_mutex_unlock(&client_mutex);
                
                handle_client_line(index, line_copy);
            } else if (c != '\r') {
                clients[index].line_buffer[clients[index].buffer_pos++] = c;
                pthread_mutex_unlock(&client_mutex);
            } else {
                pthread_mutex_unlock(&client_mutex);
            }
        }
    }
    
    // Limpieza
    close(client_fd);
    pthread_mutex_lock(&client_mutex);
    clients[index].fd = -1;
    memset(clients[index].topic, 0, MAX_TOPIC_LEN);
    clients[index].buffer_pos = 0;
    pthread_mutex_unlock(&client_mutex);
    
    return NULL;
}

// ========================================
// FUNCIÓN PRINCIPAL
// ========================================
int main(int argc, char *argv[]) {
    // 0. Inicializar clientes
    init_clients();

    // 1. Parsear argumentos
    if (argc < 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        printf("Ejemplo: %s 9092\n", argv[0]);
        return 1;
    }

    // 2. Inicializar broker (escuchar en todas las interfaces)
    int server_fd = initialize_broker("0.0.0.0", atoi(argv[1]));
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
    }

    return 0;
}