# README: Инструкция по использованию Mosquitto MQTT брокера

Этот файл README предоставляет информацию о настройке и использовании Mosquitto MQTT брокера для подписки и публикации сообщений, а также управления статусами устройств.

## Статус Mosquitto

Для проверки текущего статуса сервиса Mosquitto используйте команду:

```bash
sudo systemctl status mosquitto
```

Если сервис активен, вы должны увидеть строку:

```
Active: active (running)
```

## Включение Mosquitto при старте системы

Чтобы настроить автозапуск Mosquitto при старте системы, выполните следующую команду:

```bash
sudo systemctl enable mosquitto
```

## Подписка на MQTT топик

Для подписки на топик MQTT через Mosquitto используйте команду `mosquitto_sub`. Укажите хост брокера и топик, на который хотите подписаться. Например:

```bash
mosquitto_sub -h localhost -t test/topic
```

- `-h`: Указывает хост, на котором работает брокер MQTT (в данном примере это `localhost`).
- `-t`: Указывает топик, на который подписывается клиент.

## Публикация сообщений на MQTT топик

Для отправки сообщений на MQTT топик используйте команду `mosquitto_pub`. Например:

```bash
mosquitto_pub -h localhost -t topic -m "message"
```

- `-h`: Указывает хост, на котором работает брокер MQTT.
- `-t`: Указывает топик, на который будет отправлено сообщение.
- `-m`: Сообщение, которое будет отправлено на указанный топик.

## Команды управления

### 1. Запуск терминала

Для открытия терминала используйте сочетание клавиш: **Ctrl + Alt + T**.

### 2. Включение автозапуска Mosquitto

Пропишите команду для включения автозапуска Mosquitto:

```bash
sudo systemctl enable mosquitto
```

Ответ терминала должен быть похож на следующий:

```
● mosquitto.service - Mosquitto MQTT Broker
     Loaded: loaded (/lib/systemd/system/mosquitto.service; enabled; vendor preset: enabled)
     Active: active (running) since Sat 2024-12-21 17:17:13 GMT; 1min 24s ago
       Docs: man:mosquitto.conf(5)
             man:mosquitto(8)
```

**Состояние "Active:" должно быть "active (running)"**.

### 3. Подписка на все устройства

Для подписки на данные со всех устройств используйте команду:

```bash
mosquitto_sub -h localhost -t "#"
```

### 4. Получение статуса устройств

Для получения статуса всех устройств используйте команду:

```bash
mosquitto_pub -h localhost -t devices/esp1/status -m "getStatus"
```

Замените `esp1` на `esp2`, `esp3` и т.д., чтобы получить данные с других устройств.

Ответ на запрос статуса будет выглядеть следующим образом:

```
getStatus
online
ПАУК

getStatus
online
ПАУК

getStatus
online
ПАУК
```

### 5. Отправка сообщения на устройства

Для отправки сообщений на устройства используйте команду `mosquitto_pub`, указывая название топика и сообщение:

Для первого устройства:

```bash
mosquitto_pub -h localhost -t fakel_one -m "КРИПЕР КРИПЕР КРИПЕР"
```

Для второго устройства:

```bash
mosquitto_pub -h localhost -t fakel_two -m "КРИПЕР КРИПЕР КРИПЕР"
```

Для третьего устройства:

```bash
mosquitto_pub -h localhost -t fakel_tree -m "КРИПЕР КРИПЕР КРИПЕР"
```

### 6. Проверка отправки данных

После отправки сообщений на устройства, выполните команду для проверки их статуса:

```bash
mosquitto_pub -h localhost -t devices/esp1/status -m "getStatus"
```

Замените `esp1` на нужное устройство для получения его статуса.

---