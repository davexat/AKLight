FROM alpine:latest

RUN apk add --no-cache gcc musl-dev make

WORKDIR /app

COPY . .

RUN make clean || true
RUN make

# Producer necesita argumentos: <broker_ip> <puerto> <topico_base> <metric1> <metric2>
# Ejemplo: ./producer broker 9092 metrics/container1 0 1
CMD ["./producer", "broker", "9092", "metrics/container", "0", "1"]
