# Guía de Uso de Docker Compose para AKLight

## Descripción del Proyecto

**AKLight** es una implementación ligera y académica de **Apache Kafka** desarrollada en **C puro**, diseñada para monitoreo de métricas del sistema en entornos containerizados. El proyecto implementa un sistema de mensajería pub/sub (publicación/suscripción) con las siguientes características principales:

### Características Principales

#### Arquitectura Pub/Sub
- **Broker**: Servidor central que gestiona la distribución de mensajes entre productores y consumidores
- **Productores**: Clientes que publican métricas del sistema a tópicos específicos
- **Consumidores**: Clientes que se suscriben a tópicos para recibir mensajes en tiempo real

#### Sistema de Tópicos Jerárquicos
- Soporte para tópicos multi-nivel (ej: `metrics/container1/cpu`)
- Wildcards de múltiples niveles con `#` (ej: `metrics/#` captura todos los subtópicos)
- Coincidencia inteligente de patrones para suscripciones flexibles

#### Métricas del Sistema
Los productores pueden monitorear y enviar las siguientes métricas:
- **CPU Load** (ID: 0): Load average del sistema (1 minuto)
- **Memoria** (ID: 1): Porcentaje de memoria utilizada
- **Procesos** (ID: 2): Número total de procesos en ejecución
- **Uptime** (ID: 3): Tiempo de actividad del sistema en horas
- **CPUs** (ID: 4): Número de procesadores disponibles

#### Protocolo de Comunicación
- Protocolo simple basado en texto sobre TCP/IP
- Comandos: `PUB <topic> <payload>` para publicar, `SUB <topic>` para suscribirse
- Formato de mensaje: `MSG <topic> <payload>` para entrega a consumidores

#### Arquitectura Modular
```
AKLight/
├── broker/          # Servidor de mensajería
├── producer/        # Cliente publicador de métricas
├── consumer/        # Cliente suscriptor de mensajes
└── common/          # Código compartido (red, constantes)
```

#### Características Técnicas
- ✅ Implementación en **C puro** sin dependencias externas
- ✅ Manejo robusto de señales (SIGINT, SIGPIPE) para shutdown graceful
- ✅ Detección automática de desconexiones de clientes
- ✅ Soporte para múltiples clientes concurrentes (hasta 10 por defecto)
- ✅ Arquitectura thread-safe con mutexes para sincronización
- ✅ Multiplataforma (Linux/Windows con adaptaciones)
- ✅ Containerización completa con Docker y Docker Compose

---

## Inicio Rápido

### 1. Construir y Levantar Todos los Servicios

```bash
docker-compose up --build
```

Este comando:
- ✅ Construye las imágenes de broker, producer y consumer
- ✅ Crea la red `aklight-network`
- ✅ Inicia todos los contenedores en el orden correcto
- ✅ Muestra los logs de todos los servicios en tiempo real

### 2. Levantar en Segundo Plano (Detached)

```bash
docker-compose up -d --build
```

### 3. Ver los Logs

```bash
# Todos los servicios
docker-compose logs -f

# Solo un servicio específico
docker-compose logs -f consumer
docker-compose logs -f producer
docker-compose logs -f broker
```

### 4. Detener los Servicios

```bash
# Detener pero mantener los contenedores
docker-compose stop

# Detener y eliminar contenedores
docker-compose down

# Detener, eliminar contenedores y volúmenes
docker-compose down -v
```

---

## 📋 Estructura de Servicios

```yaml
aklight-network (bridge)
    │
    ├── broker (puerto 9092)
    │   └── Gestiona mensajería pub/sub
    │
    ├── producer → metrics/container1
    │   ├── Métrica 1: CPU Load (ID: 0)
    │   └── Métrica 2: Memoria (ID: 1)
    │
    └── consumer → metrics/#
        └── Recibe todos los mensajes de métricas
```

---

## 🔍 Verificación

### Verificar que los Contenedores Están Corriendo

```bash
docker-compose ps
```

Deberías ver algo como:

```
NAME                  STATUS    PORTS
aklight-broker        Up        0.0.0.0:9092->9092/tcp
aklight-producer1     Up
aklight-producer2     Up
aklight-consumer      Up
```

### Ver Mensajes en Tiempo Real

```bash
docker-compose logs -f consumer
```

Deberías ver mensajes como:

```
MSG metrics/container1/cpu load=0.52
MSG metrics/container1/memory mem=45.3%
```

---

## 🛠️ Comandos Útiles

### Reconstruir Solo un Servicio

```bash
docker-compose up -d --build producer
```

### Reiniciar un Servicio

```bash
docker-compose restart producer
```

### Ejecutar Comandos Dentro de un Contenedor

```bash
# Entrar al shell del broker
docker-compose exec broker sh

# Ver procesos en el producer
docker-compose exec producer1 ps aux
```

### Escalar Producers (Crear Más Instancias)

```bash
# Crear múltiples instancias del producer
docker-compose up -d --scale producer=3
```

**Nota**: Cada instancia publicará a `metrics/container1` con las mismas métricas configuradas.

---

## ⚠️ Troubleshooting

### Error: "Cannot connect to broker"

**Problema:** Los producers/consumers no pueden conectarse al broker.

**Solución:**
```bash
# Verificar que el broker está corriendo
docker-compose logs broker

# Reiniciar todo
docker-compose down
docker-compose up --build
```

### Error: "Address already in use"

**Problema:** El puerto 9092 ya está en uso.

**Solución:**
```bash
# Ver qué está usando el puerto
lsof -i :9092  # Linux/Mac
netstat -ano | findstr :9092  # Windows

# Cambiar el puerto en docker-compose.yml
ports:
  - "9093:9092"  # Usar 9093 en el host
```

### Error de Compilación en Docker

**Problema:** `make` falla dentro del contenedor.

**Solución:**
```bash
# Limpiar y reconstruir
docker-compose down
docker-compose build --no-cache
docker-compose up
```

### Ver Errores de Compilación

```bash
# Construir solo el producer para ver errores
docker-compose build producer1
```

---

## 🧪 Pruebas

### Prueba 1: Verificar Conectividad

```bash
# Terminal 1: Ver logs del consumer
docker-compose logs -f consumer

# Terminal 2: Reiniciar el producer
docker-compose restart producer

# Deberías ver nuevos mensajes en el consumer
```

### Prueba 2: Simular Fallo del Broker

```bash
# Detener el broker
docker-compose stop broker

# Ver qué pasa con el producer (debería mostrar error)
docker-compose logs producer

# Reiniciar el broker
docker-compose start broker
```

### Prueba 3: Agregar un Consumer Adicional

```bash
# Ejecutar un consumer adicional manualmente
docker-compose run --rm consumer ./consumer broker 9092 "metrics/producer1/#"
```

---

## 📊 Métricas Disponibles

| Métrica | ID | Descripción |
|---------|----|-----------| 
| CPU Load | 0 | Load average (1 min) |
| Memoria | 1 | Porcentaje usado |
| Procesos | 2 | Número total |
| Uptime | 3 | Horas de actividad |
| CPUs | 4 | Número de procesadores |

**Configuración Actual:**
- `producer`: CPU Load (0) + Memoria (1)
- Tópico base: `metrics/container1`
- Intervalo de envío: 5 segundos

---

## 🎯 Personalización

### Cambiar Métricas de un Producer

Edita `docker-compose.yml`:

```yaml
producer:
  command: ["./producer", "broker", "9092", "metrics/container1", "2", "3"]
  #                                                               Procs Uptime
```

**Métricas disponibles**: 0=CPU, 1=Memoria, 2=Procesos, 3=Uptime, 4=CPUs

### Cambiar Tópico del Consumer

```yaml
consumer:
  command: ["./consumer", "broker", "9092", "metrics/container1/cpu"]
  #                                          Solo mensajes de CPU
```

**Ejemplos de patrones de suscripción:**
- `metrics/#` - Todos los mensajes de métricas
- `metrics/container1/#` - Todos los mensajes de container1
- `metrics/container1/cpu` - Solo mensajes de CPU

### Agregar Más Producers

```yaml
producer2:
  build:
    context: .
    dockerfile: producer/Dockerfile
  container_name: aklight-producer2
  depends_on:
    - broker
  command: ["./producer", "broker", "9092", "metrics/container2", "2", "3"]
  networks:
    - aklight-network
```

**Nota**: Asegúrate de usar un tópico base diferente (ej: `metrics/container2`) para distinguir entre productores.

---

## ✅ Checklist de Verificación

Antes de ejecutar `docker-compose up`:

- [ ] Todos los Makefiles están actualizados (incluyen `../common/src/network.c`)
- [ ] Los Dockerfiles copian el directorio `common/`
- [ ] El puerto 9092 está disponible
- [ ] Docker está corriendo
- [ ] Tienes permisos para ejecutar Docker
- [ ] Las métricas configuradas son válidas (0-4) y diferentes entre sí

---

## 🎉 Resultado Esperado

Al ejecutar `docker-compose up`, deberías ver:

```
aklight-broker      | Broker inicializado y escuchando en 0.0.0.0:9092
aklight-producer    | Conectado al broker en broker:9092
aklight-producer    | Métrica 1: metrics/container1/cpu
aklight-producer    | Métrica 2: metrics/container1/memory
aklight-consumer    | Conectado al broker en broker:9092
aklight-consumer    | Suscrito a: metrics/#
aklight-consumer    | MSG metrics/container1/cpu load=0.52
aklight-consumer    | MSG metrics/container1/memory mem=45.3%
```

---

## 🏗️ Detalles de Implementación

### Protocolo de Comunicación

AKLight utiliza un protocolo simple basado en texto sobre TCP:

**Comandos del Producer:**
```
PUB <topic> <payload>
```
Ejemplo: `PUB metrics/container1/cpu load=0.52`

**Comandos del Consumer:**
```
SUB <topic_pattern>
```
Ejemplo: `SUB metrics/#`

**Mensajes del Broker a Consumers:**
```
MSG <topic> <payload>
```
Ejemplo: `MSG metrics/container1/cpu load=0.52`

### Arquitectura de Red

- **Puerto del Broker**: 9092 (estándar de Kafka)
- **Red Docker**: `aklight-network` (bridge)
- **Resolución DNS**: Los contenedores se comunican usando nombres de servicio (`broker`, `producer`, `consumer`)
- **Protocolo**: TCP con sockets POSIX

### Manejo de Concurrencia

- El broker utiliza **threads POSIX** para manejar múltiples clientes simultáneamente
- Cada cliente (producer/consumer) se ejecuta en su propio thread
- Sincronización mediante **mutexes** para acceso seguro a estructuras compartidas
- Límite configurable de clientes concurrentes (definido en `MAX_CLIENTS`)

### Gestión de Recursos

- **Shutdown Graceful**: Manejo de señales SIGINT para cierre ordenado
- **Detección de Desconexiones**: El broker detecta automáticamente cuando un cliente se desconecta
- **Limpieza de Recursos**: Cierre apropiado de sockets y liberación de threads
- **Manejo de Errores**: Gestión robusta de errores de red (EPIPE, ECONNRESET)
