#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>


// Определяем пины CS для двух модулей
#define PN532_CS_1 D3
#define PN532_CS_2 D2

#define RELE_1  D1

PN532_SPI pn532spi1(SPI, PN532_CS_1);
NfcAdapter nfc1 = NfcAdapter(pn532spi1);
String tagId_1 = "None";

PN532_SPI pn532spi2(SPI, PN532_CS_2);
NfcAdapter nfc2 = NfcAdapter(pn532spi2);


const char* ssid = "X32_Mix";         // Замените на ваш Wi-Fi SSID
const char* password = "89265357196"; // Замените на ваш Wi-Fi пароль
//const char* mqttServer = "192.168.1.20";    // IP вашего MQTT-брокера
const char* mqttServer = "192.168.0.103";
const int mqttPort = 1883;                 // Порт MQTT
const char* mqttTopic = "kod_generator"; // Тема для подписки
const char* mqttStatus = "devices/esp4/status"; // Тема для статуса
const char* getStatus = "getStatus"; // Сообщение о получении статуса
const char* mqttWord = "devices/esp4/word";
const char* mqttEdit = "devices/esp4/setting";

const char* mqttOn = "onCube";
const char* mqttOff = "offCube";

const char* ID = "esp4";


uint16_t curTimeStatusESP = 0;
uint16_t curTimeRele = 0;


WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();
void updateStatus(const String& status);
void sendWord(const String& word);

int GetCode(String word, int index);
String utf8ToReadable(String utf8String);

void mqttLoop();

void readNFC();

int readFromTag(NfcAdapter &nfc);
bool writeToTag(NfcAdapter &nfc, int number);
bool correctABC(int index, int nfc);
bool chekNFC();

void BigLump();

enum Status {
  NEW_GAME,
  WAITE,
  STOP,
  LED
};
Status game;

bool flag = true;
int datanfc_2 = -1;
int datanfc_1 = -1;


// Фиксированный ключ дешифровки
const uint8_t decryptionKey[4] = {0x12, 0x34, 0x56, 0x78}; // Пример ключа


String mapWord[3] = {
  "КРИПЕР",
  "КРИПЕР",
  "КРИПЕР"
};

void setup() {
  Serial.begin(115200);
  //Подключение к Wi-Fi
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
    if (client.connect(ID)) {  // Идентификатор клиента
        Serial.println("Connected!");
        //client.subscribe(mqttEdit);  // Подписываемся на топик устройства
        client.subscribe(mqttStatus);  // Подписываемся на топик статуса
        updateStatus("online");  // Отправляем статус "online" после подключения
        client.subscribe(mqttTopic);  // Подписываемся на топик устройства
        
    } else {
        Serial.print("Failed, rc=");
        Serial.println(client.state());
        delay(5000);
    }
  }

  Serial.println("Initializing NFC modules...");

  // Инициализация первого модуля

  nfc1.begin();
  Serial.println("NFC module 1 initialized.");

  // Инициализация второго модуля
  nfc2.begin();
  Serial.println("NFC module 2 initialized.");

  // digitalWrite(PN532_CS_1, LOW);
  // digitalWrite(PN532_CS_2, HIGH);

  game = Status::WAITE;

  pinMode(RELE_1, OUTPUT);
  digitalWrite(RELE_1, LOW);

  // delay(1000);
  // digitalWrite(RELE_1, HIGH);
  // delay(1000);
  // digitalWrite(RELE_1, LOW);
  
}

void loop() {
  
  // nfc1.format();
  // if (writeToTag(nfc1, 10)) {
  //   Serial.println("Tag 1 written successfully.");
  // };

  // Serial.println("Waiting for 5 seconds...");
  // delay(5000);

  // if (readFromTag(nfc1) == 1) {
  //   Serial.println("Tag 1 read successfully.");
  // };

  mqttLoop();

  if (game == Status::NEW_GAME) {
    flag = true;
    game = Status::WAITE;
    Serial.println("Старт игры!");
    digitalWrite(RELE_1, LOW);
    
  } else if (game == Status::WAITE) {
    if (chekNFC()) {
      game = Status::STOP;
      Serial.println("Победа!");
      updateStatus("Pobeda");
      curTimeRele = millis();
      //digitalWrite(RELE_1, HIGH);
    }
  } else if (game == Status::STOP) {
    BigLump();
  } else if (game == Status::LED) {
    digitalWrite(RELE_1, HIGH);
  }

}

void BigLump() {
  if (millis() - curTimeRele > 10000) {
    game = Status::NEW_GAME;
    digitalWrite(RELE_1, LOW);
    // digitalWrite(RELE_2, LOW);
    

  } else {
    digitalWrite(RELE_1, HIGH);
  }
}

void readNFC() {

 if (nfc1.tagPresent()) {
   NfcTag tag = nfc1.read();
   tag.print();
   tagId_1 = tag.getUidString();
 }
 delay(5000);
}

bool writeToTag(NfcAdapter &nfc, int number) {
  // Преобразуем число в строку
  // String data = String(number);

  // Проверяем, готова ли метка для записи
  if (nfc.tagPresent()) {
    NdefMessage message = NdefMessage();  
    message.addTextRecord(String(number));
   
    bool success = nfc.write(message);
    if (success) {
      //if writing was successful, displaying a message about it
      Serial.println("Successful. Try reading the message using reader or mobile phone.");
      return true;
    } else {
      //if writing was not successful, displaying that there was an error
      Serial.println("Writing unsuccessful.");
      return false;
    }
    return false;
  }
  return false;
}

int readFromTag(NfcAdapter &nfc) {
 if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    if (tag.hasNdefMessage()) {
        NdefMessage message = tag.getNdefMessage();
        NdefRecord record = message.getRecord(0);

        // Получим полезную нагрузку
        int payloadLength = record.getPayloadLength();
        byte* payload = (byte*)malloc(payloadLength);
        record.getPayload(payload);

        // Преобразуем байты в строку
        String payloadString = String((char*)payload);
        Serial.print("Прочитано с метки: ");
        Serial.println(payloadString);

        // Извлекаем только цифры из строки
        String numberString = "";
        for (int i = 0; i < payloadString.length(); i++) {
            if (isDigit(payloadString[i])) {
                numberString += payloadString[i];
            }
        }

        // Преобразуем строку в число
        int number = numberString.toInt();
        Serial.print("Число: ");
        Serial.println(number);
        return number; 
        free(payload);
    }
  }


  return -1;

}

bool chekNFC() {
  
  if (flag) {
    datanfc_1 = readFromTag(nfc1);

    Serial.print("Метка 1: "); Serial.println(datanfc_1);
    if (datanfc_1 == -1){
      return false;
    }

    if (correctABC(datanfc_1, 1)) {
      flag = false;
    }
  }
  if (!flag) {
    datanfc_2 = readFromTag(nfc2);

    Serial.print("Метка 2: "); Serial.println(datanfc_2);
    if (datanfc_2 == -1) {
      return false;
    }

    if (correctABC(datanfc_2, 2)) {
      flag = false;
    }
    return true;
  }
  return false;
}

bool correctABC(int index, int nfc) {

  if (nfc == 1) {
    if ( GetCode(mapWord[0], 0) == index) return true;
    else return false;
  } else if (nfc == 2) {
    if ( GetCode(mapWord[0], 3) == index) return true;
    else return false;
  }
  return false;
}

void mqttLoop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Обрабатывает входящие сообщения MQTT
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Сообщение получено в топике: ");
  Serial.println(topic);

  if (String(topic) == mqttStatus) {
    Serial.println("Топик статуса");
    // Переводим сообщение из байтов в строку
    // Переводим сообщение из байтов в строку
    String message = "";
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.print("Сообщение: ");
    Serial.println(message);

    if (message == getStatus) {
      Serial.print("Отправка статуса: ");
      updateStatus("online");  // Отправляем статус "online" после подключения
      sendWord(mapWord[0]);
    }

    return;
    
  } else if (String(topic) == mqttEdit) {
    Serial.println("Топик настройки");
    // Переводим сообщение из байтов в строку
    String message = "";
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.print("Сообщение: ");
    Serial.println(message);

    if (message == mqttOn) {
      game = Status::LED;
    } else if (message == mqttOff) {
      game = Status::NEW_GAME;
    }

    return;
    
  }

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

  // String message = "";
  // for (int i = 0; i < length; i++) {
  //   message += (char)payload[i]; // Чтение как UTF-8
  // }

  // Serial.print("Сообщение: ");
  // Serial.println(message);

  // // Если сообщение содержит некорректные символы, обрабатывайте их как UTF-8
  // message = utf8ToReadable(message);
  // Serial.println("Обработанное сообщение: " + message);

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

  game = Status::NEW_GAME;

}

void sendWord(const String& word) {
  client.publish(mqttWord, word.c_str());
}

String utf8ToReadable(String utf8String) {
  String readableString = "";
  for (unsigned int i = 0; i < utf8String.length(); i++) {
    if ((utf8String[i] & 0x80) == 0) {
      readableString += utf8String[i]; // ASCII символ
    } else if ((utf8String[i] & 0xE0) == 0xC0) {
      readableString += '?'; // Заменить некорректный символ
      i++;
    } else if ((utf8String[i] & 0xF0) == 0xE0) {
      readableString += '?'; // Заменить некорректный символ
      i += 2;
    }
  }
  return readableString;
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Подключение к MQTT...");
    if (client.connect(ID)) {
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