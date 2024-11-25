#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

//Credenciais do WiFi
#define WIFI_SSID "Marco"
#define WIFI_PASSWORD "12345678"

//Credenciais do Bot Telegram
#define BOT_TOKEN "7991659826:AAFRcDqLIZrC9VnrNxPJ_gT82PyIKO8Ma2E"
#define CHAT_ID "8147771515"

//Configurações do sensor DHT11
#define DHTPIN 15 // PINO de dados do DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Configurações do Reed Switch
#define REED_PIN 21 // PINO de entrada do reed switch

//Configurações do LED RGB
#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

//WiFi e Telegram Bot
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//Variáveis de status
bool portaAberta = false;
bool temperaturaAlta = false;
float temperatura = 0.0;
unsigned long tempoAnterior = 0;
const long intervaloEnvio = 30000;//30 segundos de intervalo entre mensagens

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(REED_PIN, INPUT_PULLUP);//Configura o reed switch como entrada com pull-up

  //Configura os pinos do LED RGB como saídas
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  //Conectar ao WiFi
  conectarWiFi();

  //Inicializa o LED RGB apagado
  apagarLedRGB();
}

void conectarWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  client.setInsecure();
}

void enviarMensagem(String mensagem) {
  if (bot.sendMessage(CHAT_ID, mensagem, "")) {
    Serial.println("Mensagem enviada com sucesso!");
  } else {
    Serial.println("Falha ao enviar mensagem.");
  }
}

void verificarEstado() {
  //Lê o status da porta
  portaAberta = digitalRead(REED_PIN) == HIGH;

  //Lê a temperatura atual
  temperatura = dht.readTemperature();

  //Verifica se houve erro de leitura do DHT11
  if (isnan(temperatura)) {
    Serial.println("Falha ao ler o sensor DHT11");
    return;
  }

  temperaturaAlta = temperatura > 5.0; //Temperatura alta acima de 5°C

  //Lógica para enviar a mensagem correta
  String mensagem;
  if (portaAberta && temperaturaAlta) {
    mensagem = "Alerta: Porta aberta e temperatura alta!";
    setCorLedRGB(0, 0, 255); //Azul
  } else if (portaAberta) {
    mensagem = "Alerta: Porta do freezer está aberta!";
    setCorLedRGB(0, 255, 0); //Verde
  } else if (temperaturaAlta) {
    mensagem = "Alerta: Temperatura alta no freezer!";
    setCorLedRGB(255, 0, 0); //Vermelho
  } else {
    mensagem = "Tudo ok: Freezer funcionando normalmente.";
    apagarLedRGB(); //Apaga o LED
  }

  //Enviar a mensagem se o intervalo de tempo foi atingido
  if (millis() - tempoAnterior >= intervaloEnvio) {
    enviarMensagem(mensagem);
    tempoAnterior = millis();
  }
}

void setCorLedRGB(int vermelho, int verde, int azul) {
  analogWrite(RED_PIN, vermelho);
  analogWrite(GREEN_PIN, verde);
  analogWrite(BLUE_PIN, azul);
}

void apagarLedRGB() {
  setCorLedRGB(0, 0, 0); //Desliga todas as cores
}

void loop() {
  verificarEstado();
  delay(2000); //Delay de 2 segundos antes de verificar novamente
}