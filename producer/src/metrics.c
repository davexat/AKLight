#include "../include/producer.h"

// Helper para leer archivo de una línea
static void read_oneline_file(const char *path, char *dest, size_t max_len) {
    FILE *f = fopen(path, "r");
    if (f) {
        if (fgets(dest, max_len, f) != NULL) {
            // Eliminar salto de línea
            dest[strcspn(dest, "\n")] = 0;
        }
        fclose(f);
    } else {
        strcpy(dest, "0");
    }
}

// 1. CPU Load: Leer /proc/loadavg y tomar el primer número
void get_metric_load(char *buffer) {
    char temp[64];
    read_oneline_file("/proc/loadavg", temp, sizeof(temp));
    
    // El formato es "0.00 0.00 0.00 ..."
    // Cortamos en el primer espacio
    char *space = strchr(temp, ' ');
    if (space) *space = '\0';
    
    strncpy(buffer, temp, MAX_VALUE_LEN - 1);
    buffer[MAX_VALUE_LEN - 1] = '\0';
}

// 2. RAM: Leer MemAvailable de /proc/meminfo (simulado rápido buscando string)
void get_metric_mem(char *buffer) {
    FILE *f = fopen("/proc/meminfo", "r");
    long available = 0;
    if (f) {
        char line[128];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "MemAvailable:")) {
                sscanf(line, "%*s %ld", &available);
                break;
            }
        }
        fclose(f);
    }
    // Convertir a MB para que sea un número más corto
    snprintf(buffer, MAX_VALUE_LEN, "%ldMB", available / 1024);
}

// 3. Threads: Leer campo 4 de /proc/loadavg "running/total"
void get_metric_threads(char *buffer) {
    char temp[64];
    read_oneline_file("/proc/loadavg", temp, sizeof(temp));
    
    // Formato: 0.52 0.58 0.59 1/457 1234
    // Buscamos la barra '/'
    char *slash = strchr(temp, '/');
    if (slash) {
        // El número total está despues del slash, hasta el siguiente espacio
        char *start = slash + 1;
        char *end = strchr(start, ' ');
        if (end) *end = '\0';
        strncpy(buffer, start, MAX_VALUE_LEN - 1);
    } else {
        strcpy(buffer, "0");
    }
    buffer[MAX_VALUE_LEN - 1] = '\0';
}
