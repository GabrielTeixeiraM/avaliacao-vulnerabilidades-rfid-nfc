#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ (2)
#define PN532_RESET (3)
#define GREEN_LED (8)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// A página 4 deve conter "OPEN" para autorizar a credencial
const uint8_t VALID_PAYLOAD[4] = {0x4F, 0x50, 0x45, 0x4E};


void printHexByte(uint8_t value) {
  if (value < 0x10)
    Serial.print("0");

  Serial.print(value, HEX);
}

bool compare4Bytes(uint8_t *firstBytes, uint8_t *secondBytes) {
  for (int i = 0; i < 4; i++) {
    if (firstBytes[i] != secondBytes[i])
      return false;
  }

  return true;
}

void signalAccessDenied(void) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    delay(100);
    digitalWrite(GREEN_LED, LOW);
    delay(100);
  }
}

void setup(void) {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);

  nfc.begin();

  uint32_t firmwareVersion = nfc.getFirmwareVersion();
  if (!firmwareVersion) {
    Serial.println("[ERRO] PN532 não encontrado.");
    while (1);
  }

  nfc.SAMConfig();

  Serial.println("====================================");
  Serial.println("[INFO] Sistema NTAG213 inicializado. Controle de acesso com verificação de escrita.");
  Serial.println("====================================");
  Serial.println("[INFO] Aguardando leitura de tag...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[7];
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (!success)
    return;

  Serial.println();
  Serial.println("====================================");
  Serial.println("[INFO] Tag detectada.");
  Serial.print("[INFO] UID: ");

  for (uint8_t i = 0; i < uidLength; i++) {
    printHexByte(uid[i]);
    Serial.print(" ");
  }
  Serial.println();

  uint8_t pageData[32];
  success = nfc.ntag2xx_ReadPage(4, pageData);

  if (!success) {
    Serial.println("[ERRO] Falha na leitura da página 4.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(1500);
    return;
  }

  Serial.print("[INFO] Página 4: ");
  for (int i = 0; i < 4; i++) {
    printHexByte(pageData[i]);
    Serial.print(" ");
  }
  Serial.println();

  if (!compare4Bytes(pageData, (uint8_t *)VALID_PAYLOAD)) {
    Serial.println("[NEGADO] Conteúdo inválido na página 4.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(1500);
    return;
  }

  Serial.println("[OK] Credencial válida.");

  uint8_t currentCounterData[32];
  
  success = nfc.ntag2xx_ReadPage(5, currentCounterData);
  
  if (!success) {
    Serial.println("[ERRO] Falha na leitura do contador da página 5.");
    Serial.println("[NEGADO] Acesso negado.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(2000);
    return;
  }
  
  uint8_t writeCounter = currentCounterData[0] + 1;
  uint8_t counterPayload[4] = {writeCounter, 0x00, 0x00, 0x00};

  Serial.print("[INFO] Tentando gravar o contador na página 5: ");
  Serial.println(writeCounter);

  success = nfc.ntag2xx_WritePage(5, counterPayload);

  if (!success) {
    Serial.println("[ERRO] Escrita Bloqueada: falha na escrita da página 5.");
    Serial.println("[NEGADO] Acesso negado.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(2000);
    return;
  }

  uint8_t readbackData[32];
  success = nfc.ntag2xx_ReadPage(5, readbackData);

  if (!success) {
    Serial.println("[ERRO] Falha na leitura da página 5 para verificar a escrita.");
    Serial.println("[NEGADO] Acesso negado.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(2000);
    return;
  }

  Serial.print("[INFO] Página 5 atual: ");
  for (int i = 0; i < 4; i++) {
    printHexByte(readbackData[i]);
    Serial.print(" ");
  }
  Serial.println();

  if (!compare4Bytes(readbackData, counterPayload)) {
    Serial.println("[ALERTA] Escrita Bloqueada: dados lidos diferentes dos enviados para escrita.");
    Serial.println("[ALERTA] Possível bloqueio de escrita por lock bits ou negação de serviço (DoS).");
    Serial.println("[NEGADO] Integridade comprometida.");
    signalAccessDenied();
    Serial.println("====================================");
    delay(2000);
    return;
  }

  Serial.println("[OK] Escrita confirmada.");
  Serial.println("[OK] Integridade válida.");
  Serial.println("[OK] Acesso liberado.");

  digitalWrite(GREEN_LED, HIGH);
  delay(3000);
  digitalWrite(GREEN_LED, LOW);

  Serial.println("====================================");
  delay(2000);
}
