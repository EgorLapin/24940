#!/usr/bin/env bash
set -euo pipefail

# Интервал между сообщениями (по умолчанию 0.5 секунды, можно поменять первым аргументом)
INTERVAL="${1:-0.5}"

# Проверяем, что клиент существует и исполняемый
if [[ ! -x ./client ]]
then
    echo "Ошибка: исполняемый файл ./client не найден или не является исполняемым" >&2
    exit 1
fi

# Бесконечно пишем строки, которые идут в stdin клиента
while true
do
    printf "hello from client at %s\n" "$(date '+%H:%M:%S')"
    sleep "${INTERVAL}"
done | ./client
