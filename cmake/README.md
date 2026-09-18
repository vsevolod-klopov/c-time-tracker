# Intec Time Tracker

Кроссплатформенное консольное приложение для сбора информации об активности пользователя и отправки накопленных событий на HTTP API.

## Возможности

Приложение:

* получает имя компьютера и использует его как идентификатор агента;
* отслеживает активность пользователя;
* собирает информацию о текущем процессе и активном окне;
* формирует события в формате JSON;
* накапливает события в буфере;
* периодически отправляет накопленные события на HTTP API;
* сохраняет неотправленные данные в резервный JSON-файл;
* поддерживает Windows и Linux.

## Используемые технологии

* C++20
* CMake
* vcpkg
* libcurl — HTTP-запросы
* nlohmann/json — работа с JSON
* WinAPI — получение системной информации и активности пользователя в Windows
* X11/Xss — получение информации об активности пользователя в Linux

## Структура проекта

```plaintext
cmake/
├── CMakeLists.txt
├── vcpkg.json
│
└── cmake/
	├── CMakeLists.txt
	├── main.cpp
	├── apiClient.cpp
	├── apiClient.h
	├── event.h
	├── eventQueue.h
	├── eventSource.h
	├── eventSourceWin.cpp
	├── eventSourceWin.h
	├── eventSourcePosix.cpp
	├── eventSourcePosix.h
	└── json.hpp
```

## Требования

### Windows

Необходимы:

* Windows 10/11;
* Visual Studio 2022 или более новая версия с поддержкой C++;
* CMake 3.16 или новее;
* vcpkg.

### Linux

Необходимы:

* компилятор с поддержкой C++20;
* CMake 3.16 или новее;
* libcurl;
* X11;
* X11 Xss extension;
* pthreads.

## Сборка Windows

### 1. Клонировать или скачать проект

Перейти в корневую директорию проекта:

```powershell
cd "C:\путь\к\проекту"
```

### 2. Проверить CMake и vcpkg

```powershell
cmake --version
vcpkg version
```

### 3. Настроить проект

Для Visual Studio и vcpkg:

```powershell
cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE="C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

Путь к `vcpkg.cmake` необходимо изменить в соответствии с расположением Visual Studio и vcpkg в системе.

### 4. Собрать проект

```powershell
cmake --build build --config Debug
```

Для Release:

```powershell
cmake --build build --config Release
```

### 5. Запустить

Для Debug-конфигурации:

```powershell
.\build\cmake\Debug\intec_time_tracker.exe
```

Для Release:

```powershell
.\build\cmake\Release\intec_time_tracker.exe
```

## Сборка Linux

Установить необходимые системные зависимости.

Для Ubuntu/Debian:

```sh
sudo apt update
sudo apt install build-essential cmake libcurl4-openssl-dev libx11-dev libxss-dev
```

После установки зависимостей перейти в директорию проекта:

```sh
cd /path/to/project
```

Настроить проект:

```sh
cmake -S . -B build
```

Собрать:

```sh
cmake --build build
```

Запустить:

```sh
./build/intec_time_tracker
```

## Работа приложения

После запуска приложение начинает собирать события активности пользователя.

Пример структуры отправляемых данных:

```json
{
	"agent_id": "DESKTOP-MIDDLE-C",
	"timestamp": 1792147320,
	"payload": [
		{
			"time": "2026-09-15 13:55:00",
			"process_name": "chrome.exe",
			"window_title": "Google Chrome",
			"user_active": true
		}
	]
}
```

События накапливаются в памяти до момента отправки на API.

Если отправка не удалась, данные не удаляются из буфера и могут быть сохранены в резервный JSON-файл. Это позволяет повторно отправить накопленные события при следующей попытке.

## Завершение работы

Для корректного завершения приложения используется:

```plaintext
Ctrl+C
```

При завершении приложение останавливает рабочие потоки и выполняет необходимую очистку ресурсов.

## Основные компоненты

### `main.cpp`

Точка входа приложения.

Инициализирует необходимые компоненты и запускает рабочие потоки.

### `eventSourceWin.cpp`

Реализация источника событий для Windows.

Использует WinAPI для получения системной информации и данных об активности пользователя.

### `eventSourcePosix.cpp`

Реализация источника событий для Linux/POSIX-систем.

Использует X11 и Xss для получения информации об активности пользователя.

### `eventQueue.h`

Потокобезопасная очередь событий, используемая для передачи данных между потоками.

### `apiClient.cpp`

Отвечает за:

* формирование JSON;
* накопление событий;
* отправку данных на API;
* обработку ошибок HTTP-запросов;
* сохранение неотправленных данных.

Для HTTP-запросов используется libcurl.

### `event.h`

Описание структуры события активности пользователя.

### `eventSource.h`

Общий интерфейс источника событий. Позволяет использовать разные реализации для Windows и Linux.

## Многопоточность

Приложение использует несколько потоков:

1. **Поток сбора событий** — получает данные об активности пользователя и помещает их в очередь.
2. **Поток отправки** — периодически получает накопленные события и передаёт их API-клиенту.
3. Дополнительный внутренний поток источника событий может использоваться для отслеживания текущего состояния активности пользователя.

Для синхронизации доступа к общим данным используются потокобезопасные механизмы.

## Зависимости проекта

Основная внешняя зависимость для Windows устанавливается через vcpkg:

```plaintext
curl
```

JSON используется через заголовочный файл `json.hpp`.

В Linux дополнительно требуются системные библиотеки X11/Xss и pthread.

## Очистка сборки

Для полной пересборки проекта можно удалить директорию `build`:

```powershell
Remove-Item -Recurse -Force .\build
```

После этого выполнить конфигурацию и сборку заново:

```powershell
cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE="C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"

cmake --build build --config Debug
```