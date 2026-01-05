FROM alpine:latest

RUN apk add --no-cache gcc musl-dev make

WORKDIR /app

COPY . .

RUN make clean || true
RUN make

EXPOSE 9092
CMD ["./broker", "9092"]