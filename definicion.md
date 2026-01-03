# Proyecto Final AKLight

Se requiere desarrollar y desplegar una solución de monitoreo de contenedores (Docker) para conocer el estado de los mismos. Para ello, deberá desarrollar una versión ligera de la solución **Apache Kafka**, considerando lo siguiente:

## Definiciones

### Broker
El broker se denomina a un servicio que estará disponible para atender mensajes de diferentes productores.
* El mismo gestionará una versión mejorada de cola de mensajes tradicionalmente utilizado en soluciones de MQTT, considerando la persistencia de los mismos.
* Para identificar cada mensaje se asociará un tópico.

### Productor
Se denomina a un sistema que desea enviar un mensaje a un broker para que pueda ser difundido a diferentes consumidores.
* El mensaje será identificado mediante un tópico.

### Consumidor
Se denomina a un sistema que desea recibir mensajes de uno o varios productores mediante un broker.

### Tópico
Es una forma para identificar a los mensajes. Los productores envían mensajes a un tópico específico y los consumidores se suscriben a los tópicos para recibir los mensajes.
* Los tópicos deberán ser considerados de diferentes niveles.
* Los mensajes que se envían deberán ser almacenados en un broker por un período de tiempo configurable, permitiendo que los consumidores tengan acceso a los datos históricos de ser necesario.

### Partición
Cada tópico puede dividirse en una o más particiones y cada partición se almacena en uno o más brokers.

### Clúster
Se denomina a la agrupación de varios brokers con la finalidad de gestionar los mensajes de los tópicos que se encuentren particionados.

---

## Funcionamiento

Para el funcionamiento de la solución tomen en consideración lo siguiente:

1.  **Envío de mensajes:** Los productores deben enviar mensajes a los brokers. Para esto se deberá definir un tópico y una clave.
    * La clave determinará en qué partición se almacenará el mensaje.
    * En el caso de **no establecerse la clave**, deberá implementar un mecanismo de *round-robin* donde cada partición va a recibir un mensaje de manera cíclica con la finalidad de distribuir los mensajes en las diferentes particiones.
2.  **Niveles de Tópicos:** Los tópicos deberán ser considerados de N niveles.
    * *Ejemplo:* `tópico_general/subtópico1/subtópico2`
3.  **Wildcards:** Los tópicos deberán considerar un wildcard de múltiples niveles (`#`).
    * *Ejemplo:* `tópico/subtópico1/#`
4.  **Sesiones:** Para el caso de los consumidores deberá implementarse la posibilidad de que se maneje una sesión (persistente y no persistente).

---

## Despliegue de la solución

Para el despliegue de la solución deberá considerar lo siguiente:

* **Contenedores:** Deberá implementar mínimo **5 contenedores (Docker)**:
    * 2 serán de tipo **Broker** y conformarán un clúster.
    * 2 serán de tipo **Productor**.
    * 1 será de tipo **Consumidor**.
* **Conectividad:** Deberá implementar la conectividad entre los diferentes contenedores.
* **Métricas:** Cada productor deberá estar en la capacidad de enviar mínimo **2 métricas** del contenedor.
    * *Ejemplos:* Uso de la CPU, memoria, disco, latencia, # errores en logs, etc.
* **Configuración de Tópicos:** Deberá utilizar tópicos con mínimo 2 niveles y wildcard de múltiple nivel.
* **Persistencia:** Deberá implementar que la sesión pueda ser persistente y no persistente, y que sea configurada inicialmente.