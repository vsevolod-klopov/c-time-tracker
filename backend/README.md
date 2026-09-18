# Data Logger Server

Простой FastAPI-сервер с базовой авторизацией. Он принимает JSON-данные, пишет их в логи и умеет переключать флаг ошибки.

## Что умеет

- `POST /data` — принимает данные и сохраняет их в логи
- `POST /flag` — включает или выключает режим ошибки
- `GET /flag` — показывает текущее состояние флага
- Проверка заголовка `X-API-Key`

## Требования

- Python 3.11+
- pip

## Установка

В корне репозитория выполните:

```powershell
python -m venv .venv
.\.venv\Scripts\pip install -r requirements.txt
```

## Запуск локально

```powershell
$env:API_KEY = "secret"
.\.venv\Scripts\python.exe -m uvicorn main:app --reload
```

Или без переменной окружения — будет использован ключ по умолчанию:

```text
secret
```

## Примеры запросов

### 1) Отправить данные

```powershell
curl -X POST http://127.0.0.1:8000/data `
  -H "X-API-Key: secret" `
  -H "Content-Type: application/json" `
  -d "{\"foo\":\"bar\",\"status\":\"ok\"}"
```

### 2) Включить режим ошибки

```powershell
curl -X POST http://127.0.0.1:8000/flag `
  -H "X-API-Key: secret" `
  -H "Content-Type: application/json" `
  -d "{\"enabled\": true}"
```

### 3) Выключить режим ошибки

```powershell
curl -X POST http://127.0.0.1:8000/flag `
  -H "X-API-Key: secret" `
  -H "Content-Type: application/json" `
  -d "{\"enabled\": false}"
```

### 4) Проверить текущее состояние флага

```powershell
curl -X GET http://127.0.0.1:8000/flag -H "X-API-Key: secret"
```

## Docker

```powershell
docker build -t data-server .
docker run -p 8000:8000 -e API_KEY=secret data-server
```

## Важный момент

Сервер хранит флаг ошибки в памяти процесса. После перезапуска контейнера или сервера состояние сбросится.
