#include "../include/producer.h"

// ========================================
// MÉTRICAS SIMPLES
// ========================================

// 0. CPU Load Average (1 minuto)
void get_metric_load(char *buffer, size_t size) {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        float load1;
        if (fscanf(fp, "%f", &load1) == 1) {
            snprintf(buffer, size, "load=%.2f", load1);
        } else {
            snprintf(buffer, size, "load=0.00");
        }
        fclose(fp);
    } else {
        snprintf(buffer, size, "load=0.00");
    }
}

// 1. Memoria disponible (MB)
void get_metric_mem(char *buffer, size_t size) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp) {
        char line[256];
        long mem_total = 0, mem_available = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
            if (sscanf(line, "MemAvailable: %ld kB", &mem_available) == 1) break;
        }
        fclose(fp);
        
        if (mem_total > 0) {
            float percent_used = ((float)(mem_total - mem_available) / mem_total) * 100.0;
            snprintf(buffer, size, "mem=%.1f%%", percent_used);
        } else {
            snprintf(buffer, size, "mem=0%%");
        }
    } else {
        snprintf(buffer, size, "mem=0%%");
    }
}

// 2. Número total de procesos
void get_metric_threads(char *buffer, size_t size) {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        char line[256];
        if (fgets(line, sizeof(line), fp)) {
            // Formato: 0.52 0.58 0.59 1/457 12345
            int running, total;
            if (sscanf(line, "%*f %*f %*f %d/%d", &running, &total) == 2) {
                snprintf(buffer, size, "procs=%d", total);
            } else {
                snprintf(buffer, size, "procs=0");
            }
        } else {
            snprintf(buffer, size, "procs=0");
        }
        fclose(fp);
    } else {
        snprintf(buffer, size, "procs=0");
    }
}

// 3. Uptime del sistema (horas)
void get_metric_uptime(char *buffer, size_t size) {
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        float uptime_seconds;
        if (fscanf(fp, "%f", &uptime_seconds) == 1) {
            float uptime_hours = uptime_seconds / 3600.0;
            snprintf(buffer, size, "uptime=%.1fh", uptime_hours);
        } else {
            snprintf(buffer, size, "uptime=0h");
        }
        fclose(fp);
    } else {
        snprintf(buffer, size, "uptime=0h");
    }
}

// 4. Número de CPUs
void get_metric_cpus(char *buffer, size_t size) {
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
        
        snprintf(buffer, size, "cpus=%d", cpu_count);
    } else {
        snprintf(buffer, size, "cpus=0");
    }
}
