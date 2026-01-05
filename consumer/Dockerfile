FROM alpine:latest

RUN apk add --no-cache gcc musl-dev make

WORKDIR /app

COPY . .

RUN make clean || true
RUN make

# Consumer necesita argumentos: <broker_ip> <puerto> <topico>
# Ejemplo: ./consumer broker 9092 metrics/#
CMD ["./consumer", "broker", "9092", "metrics/#"]
