#include "../include/producer.h"

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
    
    // Convertir IP string a formato de red
    if (inet_pton(AF_INET, broker_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(socket_fd);
        return -1;
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
        printf("Conexión cerrada.\n");
    }
}

// ========================================
// ENVÍO DE MENSAJE
// ========================================

void send_message(int socket_fd, Message *message) {
    // 1. Establecer Tipo
    memcpy(message->type, MSG_TYPE_PUB, 3);
    
    // 2. Copiar Tópico (asegurando límite)
    strncpy(message->topic, topic, MAX_TOPIC_LEN - 1);
    
    // 3. Copiar Valor
    strncpy(message->value, value, MAX_VALUE_LEN - 1);
    
    // Enviar estructura binaria completa
    ssize_t sent = send(socket_fd, message, sizeof(Message), 0);
    
    if (sent != sizeof(Message)) {
        perror("Error enviando mensaje");
    } else {
        printf("[PUB] %s -> %s\n", message->topic, message->value);
    }
}

// ========================================
// MAIN
// ========================================

int main(int argc, char *argv[]) {
    // 1. Parsear argumentos
    if (argc < 6) {
        printf("Uso: %s <ip_broker> <puerto> <topico_base> <metric1> <metric2>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 9092 metrics/container1 0 1\n", argv[0]);
        printf("\nMétricas disponibles:\n");
        printf("  0 = CPU Load (load average 1min)\n");
        printf("  1 = Memoria (porcentaje usado)\n");
        printf("  2 = Procesos (número total)\n");
        printf("  3 = Uptime (horas)\n");
        printf("  4 = CPUs (número de procesadores)\n");
        return 1;
    }

    char *broker_ip = argv[1];
    int port = atoi(argv[2]);
    char *base_topic = argv[3];
    int metric1 = atoi(argv[4]);
    int metric2 = atoi(argv[5]);

    // Validar métricas
    if (metric1 < 0 || metric1 > 4 || metric2 < 0 || metric2 > 4) {
        fprintf(stderr, "Error: Las métricas deben estar entre 0 y 4\n");
        return 1;
    }

    // Validar que las métricas sean diferentes
    if (metric1 == metric2) {
        fprintf(stderr, "Error: Las métricas deben ser diferentes\n");
        return 1;
    }

    // 2. Inicializar conexión
    int socket_fd = initialize_connection(broker_ip, port);
    if (socket_fd == -1) {
        fprintf(stderr, "Error: No se pudo conectar al broker\n");
        return 1;
    }

    // 3. Preparar tópicos para cada métrica
    char topic1[256], topic2[256];
    const char *metric_names[] = {"cpu", "memory", "processes", "uptime", "cpus"};
    
    snprintf(topic1, sizeof(topic1), "%s/%s", base_topic, metric_names[metric1]);
    snprintf(topic2, sizeof(topic2), "%s/%s", base_topic, metric_names[metric2]);

    int n = 5;
    
    printf("\nIniciando envío de métricas cada %d segundos...\n", n);
    printf("Métrica 1: %s (tipo: %s)\n", topic1, metric_names[metric1]);
    printf("Métrica 2: %s (tipo: %s)\n", topic2, metric_names[metric2]);
    
    // 4. Bucle de envío de mensajes
    while (1) {
        // Enviar métrica 1
        switch (metric1) {
            case 0: get_metric_load(socket_fd, topic1); break;
            case 1: get_metric_mem(socket_fd, topic1); break;
            case 2: get_metric_threads(socket_fd, topic1); break;
            case 3: get_uptime(socket_fd, topic1); break;
            case 4: get_cpu_count(socket_fd, topic1); break;
        }
        
        // Enviar métrica 2
        switch (metric2) {
            case 0: get_metric_load(socket_fd, topic2); break;
            case 1: get_metric_mem(socket_fd, topic2); break;
            case 2: get_metric_threads(socket_fd, topic2); break;
            case 3: get_uptime(socket_fd, topic2); break;
            case 4: get_cpu_count(socket_fd, topic2); break;
        }
        
        sleep(n);
    }

    end_connection(socket_fd);
    return 0;
}