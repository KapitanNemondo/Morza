#include <Arduino.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <FastLED.h>

#define LED_PIN_1  0
#define TIME_INTERVAL 400
#define TIME_OFF      800
#define TIME_ON_INTERVAL_WORD 2000

// Параметры ленты
#define NUM_LEDS_1    64
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

CRGB leds[NUM_LEDS_1];

const char* ssid = "TP1";         // Замените на ваш Wi-Fi SSID
const char* password = "Takeoffmai_2024"; // Замените на ваш Wi-Fi пароль
const char* mqttServer = "192.168.1.20";    // IP вашего MQTT-брокера
const int mqttPort = 1883;                 // Порт MQTT
const char* mqttTopic = "fakel_one"; // Тема для подписки
const char* mqttStatus = "devices/esp1/status"; // Тема для статуса

uint16_t curTimeStatusESP = 0;


WiFiClient espClient;
PubSubClient client(espClient);

CRGB selectedColor = CRGB::Red;

uint16_t mapABC[32][6] = {
  {1, 2, 0, 0, 0, 0},   // А 0
  {2, 1, 1, 1, 0, 0},   // Б 1
  {1, 2, 2, 0, 0, 0},   // В 2
  {2, 2, 1, 0, 0, 0},   // Г 3
  {2, 1, 1, 0, 0, 0},   // Д 4
  {1, 0, 0, 0, 0, 0},   // Е 5
  {1, 1, 1, 2, 0, 0},   // Ж 6
  {2, 2, 1, 1, 0, 0},   // З 7
  {1, 1, 0, 0, 0, 0},   // И 8
  {1, 2, 2, 2, 0, 0},   // Й 9
  {2, 1, 2, 0, 0, 0},   // К 10 
  {1, 2, 1, 1, 0, 0},   // Л 11
  {2, 2, 0, 0, 0, 0},   // М 12
  {2, 1, 0, 0, 0, 0},   // Н 13
  {2, 2, 2, 0, 0, 0},   // О 14
  {1, 2, 2, 1, 0, 0},   // П 15
  {1, 2, 1, 0, 0, 0},   // Р 16
  {1, 1, 1, 0, 0, 0},   // С 17
  {2, 0, 0, 0, 0, 0},   // Т 18
  {1, 1, 2, 0, 0, 0},   // У 19
  {2, 1, 1, 2, 0, 0},   // Ф 20
  {1, 1, 1, 1, 0, 0},   // Х 21
  {2, 1, 2, 1, 0, 0},   // Ц 22
  {2, 2, 2, 1, 0, 0},   // Ч 23
  {2, 2, 2, 2, 0, 0},   // Ш 24
  {2, 2, 1, 2, 0, 0},   // Щ 25
  {2, 1, 2, 1, 2, 1},   // Ъ 26
  {2, 1, 2, 2, 0, 0},   // Ы 27
  {2, 1, 1, 2, 0, 0},   // Ь 28
  {1, 1, 2, 1, 1, 0},   // Э 29
  {1, 1, 2, 2, 0, 0},   // Ю 30
  {1, 2, 1, 2, 0, 0}    // Я 31
};



// Массив кодов символов для букв в UTF-8 (А-Я)
uint16_t mapLetters[32][2] = {
  {0xD090, 0x0410}, // А [0]
  {0xD091, 0x0411}, // Б [1]
  {0xD092, 0x0412}, // В [2]
  {0xD093, 0x0413}, // Г [3]
  {0xD094, 0x0414}, // Д [4]
  {0xD095, 0x0415}, // Е [5]
  //{0xD081, 0x0401}, // Ё [6]
  {0xD096, 0x0416}, // Ж [6]
  {0xD097, 0x0417}, // З [7]
  {0xD098, 0x0418}, // И [8]
  {0xD099, 0x0419}, // Й [9]
  {0xD09A, 0x041A}, // К [10]
  {0xD09B, 0x041B}, // Л [11]
  {0xD09C, 0x041C}, // М [12]
  {0xD09D, 0x041D}, // Н [13]
  {0xD09E, 0x041E}, // О [14]
  {0xD09F, 0x041F}, // П [15]
  {0xD0A0, 0x0420}, // Р [16]
  {0xD0A1, 0x0421}, // С [17]
  {0xD0A2, 0x0422}, // Т [18]
  {0xD0A3, 0x0423}, // У [19]
  {0xD0A4, 0x0424}, // Ф [20]
  {0xD0A5, 0x0425}, // Х [21]
  {0xD0A6, 0x0426}, // Ц [22]
  {0xD0A7, 0x0427}, // Ч [23]
  {0xD0A8, 0x0428}, // Ш [24]
  {0xD0A9, 0x0429}, // Щ [25]
  {0xD0AA, 0x042A}, // Ъ [26]
  {0xD0AB, 0x042B}, // Ы [27]
  {0xD0AC, 0x042C}, // Ь [28]
  {0xD0AD, 0x042D}, // Э [29]
  {0xD0AE, 0x042E}, // Ю [30]
  {0xD0AF, 0x042F}  // Я [31]
};


// Массив слов
String mapWord[] = {
  "ПАУК",
  "КРИПЕР",
  "СКЕЛЕТ",
};



void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();
void updateStatus(const String& status);

void mqttLoop();

String currentWord = "";         // Текущее слово для передачи
int currentLetterIndex = 0;      // Индекс текущей буквы в слове
int currentSignalIndex = 0;      // Индекс текущего сигнала в букве
unsigned long lastActionTime = 0; // Время последнего действия
bool isBetweenSignals = false;   // Состояние ожидания между сигналами
bool isBetweenLetters = false;   // Состояние ожидания между буквами
bool isBetweenWords = false;     // Состояние ожидания между словами
int currentWordIndex = 0;        // Индекс текущего слова
int signal;                      // Текущий сигнал
int currentletterCode;           // Код текущей буквы

void ProcessMorse();              // Обрабатывает текущий процесс передачи сигнала Морзе
void StartWord(String word);      // Начинает передачу нового слова
void StartLetter();               // Начинает передачу новой буквы
void StartSignal();               // Начинает передачу нового сигнала (точка/тире)
void EndSignal();                 // Завершает текущий сигнал (точка/тире)
int GetCode(String word, int index); // Возвращает код символа в UTF-8 из строки на заданной позиции



void setup() {
  Serial.begin(9600);
  // Подключение к Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");

    // Настройка MQTT
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

  while (!client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (client.connect("ESP8266Client")) {  // Идентификатор клиента
            Serial.println("Connected!");
            client.subscribe(mqttStatus);  // Подписываемся на топик статуса
            updateStatus("online");  // Отправляем статус "online" после подключения
            client.subscribe(mqttTopic);  // Подписываемся на топик устройства
        } else {
            Serial.print("Failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }



  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds, NUM_LEDS_1);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();

  isBetweenWords = true;
  lastActionTime = millis();


}

void loop() {
  mqttLoop();
  ProcessMorse();    // Обработка передачи Морзе

  // if (millis() - curTimeStatusESP > 10000) {
  //   updateStatus("online");
  //   curTimeStatusESP = millis();
  // }
}

// Функция для обработки сообщений MQTT
void mqttLoop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Обрабатывает входящие сообщения MQTT
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Сообщение получено в топике: ");
  Serial.println(topic);

  // Если топик не соответствует mqttTopic, выходим
  if (String(topic) != mqttTopic) {
    Serial.println("Получено сообщение с ненужного топика. Игнорируем.");
    return; // Игнорируем сообщения, которые не с нужного топика
  }

  // Переводим сообщение из байтов в строку
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Сообщение: ");
  Serial.println(message);

  // Разделяем строку на слова
  String tempWord[3];  // Временный массив для новых слов
  int wordIndex = 0;   // Индекс текущего слова
  int startIndex = 0;  // Начало текущего слова

  for (int i = 0; i <= message.length(); i++) {
    if (message[i] == ' ' || i == message.length()) { // Найден пробел или конец строки
      if (wordIndex < 3) { // Сохраняем только три слова
        tempWord[wordIndex] = message.substring(startIndex, i);
        wordIndex++;
      }
      startIndex = i + 1; // Обновляем индекс начала следующего слова
    }
  }

  // Перезаписываем массив mapWord
  for (int i = 0; i < 3; i++) {
    if (i < wordIndex) { // Если слово есть
      mapWord[i] = tempWord[i];
    } else { // Если слов меньше трёх
      mapWord[i] = "";
    }
  }

  // currentWord = 0;

  // Выводим обновлённый массив
  Serial.println("Обновлённый массив mapWord:");
  for (int i = 0; i < 3; i++) {
    Serial.print("mapWord[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(mapWord[i]);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Подключение к MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Подключено");
      client.subscribe(mqttTopic);
    } else {
      Serial.print("Ошибка подключения, rc=");
      Serial.print(client.state());
      Serial.println(" Попробую снова через 5 секунд");
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Подключение к Wi-Fi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi подключен");
}

void updateStatus(const String& status) {
  client.publish(mqttStatus, status.c_str());
}

void ProcessMorse() {
  unsigned long now = millis();

  // Переход между словами
  if (isBetweenWords) {
    // if (now - lastActionTime >= TIME_ON_INTERVAL_WORD) {
    //   isBetweenWords = false;
    //   StartWord(mapWord[currentWordIndex]); // Переход к следующему слову
    // }

    if (now - lastActionTime < TIME_ON_INTERVAL_WORD) {
      // Включаем желтый свет
      for (int i = 0; i < NUM_LEDS_1; i++) {
        leds[i] = CRGB::Yellow; // Устанавливаем желтый цвет
      }
      FastLED.show();
    } else {
      // Гасим свет и переходим к следующему слову
      FastLED.clear();
      FastLED.show();
      isBetweenWords = false;
      StartWord(mapWord[currentWordIndex]); // Переход к следующему слову
    }
    return;
  }

  // Переход между буквами
  if (isBetweenLetters) {
    if (now - lastActionTime >= TIME_OFF) {
      isBetweenLetters = false;
      StartLetter(); // Переход к следующей букве
    }
    return;
  }

  // Переход между сигналами
  if (isBetweenSignals) {
    if (now - lastActionTime >= TIME_OFF) {
      isBetweenSignals = false;
      StartSignal(); // Переход к следующему сигналу
    }
    return;
  }

  // Завершение текущего сигнала
  int k = 0;
  if (signal == 2) k = 3 * TIME_INTERVAL;
  if (signal == 1) k = TIME_INTERVAL;
  if (now - lastActionTime >= k) {
    EndSignal(); // Завершаем текущий сигнал
  }
}


void StartWord(String word) {
  currentWord = word;
  currentLetterIndex = 0;
  currentSignalIndex = 0; // Сбрасываем индекс сигналов
  isBetweenLetters = true; // Устанавливаем паузу между буквами
  lastActionTime = millis();

  Serial.print("Начинаем слово: ");
  Serial.println(currentWord); // Отладка
}

void StartLetter() {
  Serial.print("Текущая буква: ");
  Serial.println(currentLetterIndex);

  if (currentLetterIndex >= currentWord.length() / 2) { // Если обработали все буквы
    Serial.println("Переход к следующему слову...");
    currentLetterIndex = 0; // Сбрасываем индекс буквы
    currentSignalIndex = 0; // Сбрасываем индекс сигнала
    currentWordIndex++; // Переходим к следующему слову

    // Если все слова обработаны, начинаем с первого слова
    if (currentWordIndex >= 3) currentWordIndex = 0;

    isBetweenWords = true; // Устанавливаем паузу между словами
    lastActionTime = millis(); // Запоминаем время
    Serial.print("Переход к слову: ");
    Serial.println(mapWord[currentWordIndex]); // Отладка
    return;
  }

  // Инициализация буквы
  currentletterCode = GetCode(currentWord, currentLetterIndex);
  if (currentletterCode != -1) {
    currentSignalIndex = 0; // Сбрасываем индекс сигналов
    isBetweenSignals = true; // Устанавливаем паузу между сигналами
    lastActionTime = millis();

    Serial.print("Начинаем букву: ");
    Serial.println(currentWord[currentLetterIndex]);
  }
}


void StartSignal() {

  Serial.print("Обработка сигнала: ");
  Serial.print(currentSignalIndex);
  Serial.print(" для буквы: ");
  Serial.print(currentWord[currentLetterIndex]);
  Serial.print(" (Код: ");
  Serial.print(currentletterCode);
  Serial.println(")");

  signal = mapABC[currentletterCode][currentSignalIndex]; // Получаем текущий сигнал

  // Проверяем, если все сигналы обработаны
  if (signal == 0 || currentSignalIndex == 5) {
    Serial.println("Все сигналы буквы обработаны.");
    currentSignalIndex = 0;          // Сбрасываем индекс сигналов
    currentLetterIndex++;            // Переходим к следующей букве
    isBetweenLetters = true;         // Устанавливаем паузу между буквами
    lastActionTime = millis();       // Сохраняем время
    return;                          // Завершаем выполнение
  }

  // Включаем светодиоды для текущего сигнала
  for (int i = 0; i < NUM_LEDS_1; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();

  Serial.print("Сигнал (точка = 1, тире = 2): ");
  Serial.println(signal);

  lastActionTime = millis(); // Запоминаем время начала сигнала
}


void EndSignal() {
  FastLED.clear();   // Гасим светодиоды
  FastLED.show();

  currentSignalIndex++;        // Переходим к следующему сигналу
  isBetweenSignals = true;     // Устанавливаем паузу между сигналами
  lastActionTime = millis();   // Запоминаем время
}


int GetCode(String word, int index) {
  // Получаем первый байт UTF-8
  index = index * 2;
  Serial.print("Получаем код буквы: "); Serial.println(index);
  char letter = word.charAt(index);
  byte firstByte = letter;

  if (firstByte >= 0xD0 && firstByte <= 0xD1) {
    // Извлекаем второй байт UTF-8
    byte secondByte = word.charAt(index + 1);
    // Формируем полный код символа
    uint16_t letterCode = (firstByte << 8) | secondByte;

    Serial.print("Processing letter: ");
    Serial.println(letter);

    Serial.print("Буква в UTF-8: ");
    Serial.println(letterCode, HEX);

    // Поиск кода в массиве mapLetters
    for (int i = 0; i < 33; i++) {
      if (mapLetters[i][0] == letterCode) {
        Serial.print("Найдена буква в массиве: ");
        Serial.println(i);
        return i; // Возвращаем индекс буквы в массиве
      }
    }

    // Если буква не найдена
    Serial.print("Буква не найдена: ");
    Serial.println(letterCode, HEX);
    return -1;
  }

  // Если символ не соответствует русскому алфавиту
  Serial.print("Символ не соответствует русскому алфавиту: ");
  Serial.println(firstByte, HEX);
  return -1;
}