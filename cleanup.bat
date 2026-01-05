@echo off
echo Limpiando contenedores de AKLight...

REM Detener contenedores
docker stop aklight-broker aklight-producer1 aklight-producer2 aklight-consumer 2>nul

REM Eliminar contenedores
docker rm aklight-broker aklight-producer1 aklight-producer2 aklight-consumer 2>nul

REM Eliminar red
docker network rm aklight-network 2>nul

set /p REPLY="Eliminar imagenes tambien? (s/n): "
if /i "%REPLY%"=="s" (
    docker rmi aklight-broker aklight-producer aklight-consumer 2>nul
    echo Imagenes eliminadas
)

echo Limpieza completa
