from flask import Flask, request, render_template, jsonify
import paho.mqtt.client as mqtt
import logging
import time

app = Flask(__name__)

# Настройки MQTT
mqtt_broker = "localhost"
mqtt_port = 1883
topics = {
    "fakel_one": {"status": "offline", "last_seen": 0},
    "fakel_two": {"status": "offline", "last_seen": 0},
    "fakel_three": {"status": "offline", "last_seen": 0},
    "kod_generator": {"status": "offline", "last_seen": 0},
}

client = mqtt.Client()
client.connect(mqtt_broker, mqtt_port, 60)

# Настройка логирования
logging.basicConfig(
    filename="mqtt_log.txt",  # Имя файла для журнала
    level=logging.INFO,  # Уровень логирования
    format="%(asctime)s - %(message)s"  # Формат сообщения
)

# MQTT Callback для обновления статуса
def on_message(client, userdata, msg):
    topic = msg.topic
    if topic in topics:
        topics[topic]["status"] = "online"
        topics[topic]["last_seen"] = time.time()

client.on_message = on_message
client.subscribe("devices/+/status")  # Подписываемся на статус всех устройств
client.loop_start()

# Главная страница
@app.route('/')
def index():
    return render_template('index.html', topics=topics)

# Обработка отправки сообщений
@app.route('/send', methods=['POST'])
def send():
    # Получаем данные из формы
    message = request.form.get('message')
    target = request.form.getlist('target')  # Получаем выбранные устройства
    broadcast = 'broadcast' in request.form  # Проверяем чекбокс "Отправить всем"

    # Проверяем, что введены три слова
    words = message.split()
    if len(words) != 3:
        logging.warning("Попытка отправки сообщения с некорректным количеством слов.")
        return jsonify({"success": False, "error": "Введите ровно три слова, разделенных пробелами."}), 400

    # Преобразуем все буквы в заглавные
    words = [word.upper() for word in words]
    formatted_message = " ".join(words)

    # Формируем список устройств
    targets = []
    if broadcast:
        targets = ["fakel_one", "fakel_two", "fakel_three"]
    else:
        targets = target

    # Логируем отправку
    if targets:
        logging.info(f"Отправлено сообщение: '{formatted_message}' на устройства: {', '.join(targets)}")
    else:
        logging.info(f"Отправлено сообщение: '{formatted_message}', но ни одно устройство не выбрано.")

    # Отправляем сообщения через MQTT
    for topic in targets:
        client.publish(topic, formatted_message)

    return jsonify({"success": True, "message": f"Отправлено: {formatted_message} на устройства {', '.join(targets)}"})

# Обновление статусов
@app.route('/status', methods=['GET'])
def status():
    # Обновляем статусы устройств
    now = time.time()
    for topic, data in topics.items():
        if now - data["last_seen"] > 60:  # Считаем оффлайн, если прошло более 60 секунд
            data["status"] = "offline"
    return jsonify(topics)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
