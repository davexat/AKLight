FROM alpine:latest

RUN apk add --no-cache gcc musl-dev make

WORKDIR /app/consumer

# Copiar todo desde el contexto raíz (que incluye common/)
COPY consumer/ .
COPY common/ ../common/

RUN make clean || true
RUN make

# Consumer necesita argumentos: <broker_ip> <puerto> <topico>
# Ejemplo: ./consumer broker 9092 metrics/#
CMD ["./consumer", "broker", "9092", "metrics/#"]
