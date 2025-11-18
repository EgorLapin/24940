#!/usr/bin/env bash
set -euo pipefail

SERVER_BIN=./server
CLIENT_BIN=./client
SOCKET_PATH=./socket
LOG_FILE=server.log

if [[ ! -x "$SERVER_BIN" ]]
then
    echo "Ошибка: сервер '$SERVER_BIN' не найден или не исполняемый" >&2
    exit 1
fi

if [[ ! -x "$CLIENT_BIN" ]]
then
    echo "Ошибка: клиент '$CLIENT_BIN' не найден или не исполняемый" >&2
    exit 1
fi

rm -f "$LOG_FILE" "$SOCKET_PATH"

stdbuf -oL "$SERVER_BIN" >"$LOG_FILE" 2>&1 &
SERVER_PID=$!
echo "Server started, PID = $SERVER_PID"

cleanup()
{
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT

echo "Waiting for socket '$SOCKET_PATH'..."
for _ in {1..50}
do
    if [[ -S "$SOCKET_PATH" ]]
    then
        break
    fi
    sleep 0.1
done

if [[ ! -S "$SOCKET_PATH" ]]
then
    echo "Сокет '$SOCKET_PATH' так и не появился" >&2
    exit 1
fi

MSG="ping-$(date +%s%N)"
MSG_UPPER=$(printf '%s' "$MSG" | tr '[:lower:]' '[:upper:]')

echo "Sending message: $MSG"

T1=$(date +%s%N)

printf '%s\n' "$MSG" | "$CLIENT_BIN"

echo "Waiting for server to log the message..."

while true
do
    if grep -q "$MSG_UPPER" "$LOG_FILE"
    then
        T2=$(date +%s%N)
        break
    fi
    sleep 0.01
done

DT_NS=$((T2 - T1))
DT_MS=$((DT_NS / 1000000))

echo "----------------------------------------"
echo "Latency:"
echo "  $DT_NS ns"
echo "  $DT_MS ms (примерно)"
echo "----------------------------------------"
