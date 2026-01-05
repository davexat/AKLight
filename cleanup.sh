#!/bin/bash

echo "🧹 Limpiando contenedores de AKLight..."

# Detener contenedores
docker stop aklight-broker aklight-producer1 aklight-producer2 2>/dev/null || true

# Eliminar contenedores
docker rm aklight-broker aklight-producer1 aklight-producer2 2>/dev/null || true

# Eliminar red
docker network rm aklight-network 2>/dev/null || true

# Eliminar imágenes (opcional)
read -p "¿Eliminar imágenes también? (s/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Ss]$ ]]
then
    docker rmi aklight-broker aklight-producer 2>/dev/null || true
    echo "✅ Imágenes eliminadas"
fi

echo "✅ Limpieza completa"
