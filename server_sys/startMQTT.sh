# Запуск MQTT в интерактивном режиме
mosquitto -v 


# Запуск MQTT в фоновом режиме
sudo systemctl start mosquitto

# Статус MQTT
sudo systemctl status mosquitto

# Включение MQTT при запуске системы
sudo systemctl enable mosquitto


# Скрипт для подписки на топик MQTT через Mosquitto

# Параметры:
# -h: Указывает хост, где работает брокер MQTT. В данном случае это localhost.
# -t: Указывает топик, на который осуществляется подписка.
mosquitto_sub -h localhost -t test/topic

# Параметры:
# -h: Указывает хост, где работает брокер MQTT. В данном случае это localhost.
# -t: Указывает топик, на который осуществляется отправка.
# -m: Указывает что будет сообщение
# message: Сообщение которое будет отправлено

mosquitto_pub -h localhost -t topic -m "message"



