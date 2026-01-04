#include "../include/producer.h"

// ========================================
// FUNCIONES DE MÉTRICAS
// ========================================

// Métrica 0: CPU Load Average (1 minuto)
void get_cpu_usage(int socket_fd, const char *topic) {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        float load1;
        fscanf(fp, "%f", &load1);
        snprintf(msg.value, sizeof(msg.value), "%.2f", load1);
        fclose(fp);
    } else {
        strcpy(msg.value, "0.00");
    }
    
    send_message(socket_fd, &msg);
}

// Métrica 1: Memoria (porcentaje usado)
void get_memory_usage(int socket_fd, const char *topic) {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp) {
        char line[256];
        long total = 0, available = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "MemTotal: %ld kB", &total);
            sscanf(line, "MemAvailable: %ld kB", &available);
        }
        fclose(fp);
        
        float percent = ((float)(total - available) / total) * 100.0;
        snprintf(msg.value, sizeof(msg.value), "%.2f", percent);
    } else {
        strcpy(msg.value, "0.00");
    }
    
    send_message(socket_fd, &msg);
}

// Métrica 2: Número de procesos en ejecución
void get_process_count(int socket_fd, const char *topic) {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        char buffer[256];
        fgets(buffer, sizeof(buffer), fp);
        // Formato: load1 load5 load15 running/total lastpid
        // Ejemplo: 0.52 0.58 0.59 2/305 12345
        int running, total;
        if (sscanf(buffer, "%*f %*f %*f %d/%d", &running, &total) == 2) {
            snprintf(msg.value, sizeof(msg.value), "%d", total);
        } else {
            strcpy(msg.value, "0");
        }
        fclose(fp);
    } else {
        strcpy(msg.value, "0");
    }
    
    send_message(socket_fd, &msg);
}

// Métrica 3: Uptime del sistema (en horas)
void get_uptime(int socket_fd, const char *topic) {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        float uptime_seconds;
        fscanf(fp, "%f", &uptime_seconds);
        float uptime_hours = uptime_seconds / 3600.0;
        snprintf(msg.value, sizeof(msg.value), "%.2f", uptime_hours);
        fclose(fp);
    } else {
        strcpy(msg.value, "0.00");
    }
    
    send_message(socket_fd, &msg);
}

// Métrica 4: Número de CPUs
void get_cpu_count(int socket_fd, const char *topic) {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        int cpu_count = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "processor", 9) == 0) {
                cpu_count++;
            }
        }
        fclose(fp);
        
        snprintf(msg.value, sizeof(msg.value), "%d", cpu_count);
    } else {
        strcpy(msg.value, "0");
    }
    
    send_message(socket_fd, &msg);
}
