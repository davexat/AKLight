#include "../include/producer.h"

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
    printf("\n\nSeñal recibida, cerrando producer...\n");
}

// ========================================
// CONEXIÓN
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

// ========================================
// ENVÍO DE MENSAJE
// ========================================

void send_string(int socket_fd, const char *message) {
    size_t len = strlen(message);
    ssize_t sent = send(socket_fd, message, len, 0);
    
    if (sent == -1) {
        if (errno == EPIPE || errno == ECONNRESET) {
            fprintf(stderr, "\nBroker desconectado, mensaje descartado: %s\n", message);
            running = 0;
        } else {
            fprintf(stderr, "Error enviando al broker: %s\n", strerror(errno));
        }
    } else if (sent < (ssize_t)len) {
        fprintf(stderr, "Mensaje rechazado (formato inválido)\n");
    } else {
        printf("Mensaje enviado al broker: %s", message);
        fflush(stdout);
    }
}

// ========================================
// MAIN
// ========================================

int main(int argc, char *argv[]) {
    // Configurar manejadores de señales
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN); // Ignorar SIGPIPE, manejar EPIPE en send()
    
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
    if (metric1 < 0 || metric1 > 4 || metric2 < 0 || metric2 > 4) return 1;

    // Validar que las métricas sean diferentes
    if (metric1 == metric2) return 1;

    // 2. Inicializar conexión
    int socket_fd = initialize_connection(broker_ip, port);
    if (socket_fd == -1) {
        fprintf(stderr, "No se pudo conectar al broker. Abortando...\n");
        return 1;
    }

    // 3. Preparar tópicos para cada métrica
    const char *metric_names[] = {"cpu", "memory", "processes", "uptime", "cpus"};
    
    printf("\nProducer iniciado en puerto %d\n", port);
    printf("Métrica 1: %s/%s\n", base_topic, metric_names[metric1]);
    printf("Métrica 2: %s/%s\n", base_topic, metric_names[metric2]);
    printf("Presiona Ctrl+C para detener\n\n");
    fflush(stdout);
    
    // Buffers para construir mensajes
    char message[256];
    char metric_value[128];
    
    // 4. Bucle de envío de mensajes
    while (running) {
        // Enviar métrica 1
        switch (metric1) {
            case 0: get_metric_load(metric_value, sizeof(metric_value)); break;
            case 1: get_metric_mem(metric_value, sizeof(metric_value)); break;
            case 2: get_metric_threads(metric_value, sizeof(metric_value)); break;
            case 3: get_metric_uptime(metric_value, sizeof(metric_value)); break;
            case 4: get_metric_cpus(metric_value, sizeof(metric_value)); break;
        }
        snprintf(message, sizeof(message), "PUB %s/%s %s\n", base_topic, metric_names[metric1], metric_value);
        if (strlen(message) >= sizeof(message) - 1) {
            fprintf(stderr, "Error formateando mensaje para broker\n");
        } else {
            send_string(socket_fd, message);
        }
        
        // Enviar métrica 2
        switch (metric2) {
            case 0: get_metric_load(metric_value, sizeof(metric_value)); break;
            case 1: get_metric_mem(metric_value, sizeof(metric_value)); break;
            case 2: get_metric_threads(metric_value, sizeof(metric_value)); break;
            case 3: get_metric_uptime(metric_value, sizeof(metric_value)); break;
            case 4: get_metric_cpus(metric_value, sizeof(metric_value)); break;
        }
        snprintf(message, sizeof(message), "PUB %s/%s %s\n", base_topic, metric_names[metric2], metric_value);
        if (strlen(message) >= sizeof(message) - 1) {
            fprintf(stderr, "Error formateando mensaje para broker\n");
        } else {
            send_string(socket_fd, message);
        }
        
        sleep(5);
    }
    
    printf("\n");
    end_connection(socket_fd);
    printf("Producer cerrado\n");
    return 0;
}