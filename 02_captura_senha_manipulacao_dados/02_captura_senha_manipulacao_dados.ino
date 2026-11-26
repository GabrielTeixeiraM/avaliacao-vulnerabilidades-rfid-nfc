#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>

#define GREEN_LED (8)

PN532_I2C pn532I2c(Wire);
PN532 nfc(pn532I2c);

// Senha configurada na página 43 do NTAG213
const uint8_t PASSWORD[4] = {0xA1, 0xB2, 0xC3, 0xD4};

// PACK esperado, configurado na página 44 do NTAG213
const uint8_t EXPECTED_PACK[2] = {0xA1, 0xB2};

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

  Serial.println("[INFO] Leitor PN532 inicializado.");
  Serial.println("[INFO] Aguardando leitura de tag NTAG213...");
}

void loop(void) {
  uint8_t uid[7] = {0};
  uint8_t uidLength = 0;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    digitalWrite(GREEN_LED, LOW);

    Serial.println("\n------------------------------------");
    Serial.println("[INFO] Tag detectada.");

    Serial.print("[INFO] UID: ");
    nfc.PrintHex(uid, uidLength);

    Serial.print("[INFO] Tamanho do UID: ");
    Serial.println(uidLength);

    if (uidLength != 7) {
      Serial.println("[NEGADO] UID incompatível com o critério de leitura NTAG213.");
      signalAccessDenied();
      delay(1000);
      return;
    }

    delay(20); // Intervalo entre a seleção da tag e o comando PWD_AUTH

    // Comando PWD_AUTH (0x1B) seguido da senha de 4 bytes
    uint8_t command[5] = {0x1B, PASSWORD[0], PASSWORD[1], PASSWORD[2], PASSWORD[3]};

    uint8_t response[8] = {0};
    uint8_t responseLength = sizeof(response);

    Serial.println("[INFO] Enviando comando PWD_AUTH...");

    bool authenticated = nfc.inCommunicateThru(command, sizeof(command), response, &responseLength);

    Serial.print("[INFO] Resultado do PWD_AUTH: ");
    Serial.println(authenticated ? "sucesso." : "falha.");

    Serial.print("[INFO] Tamanho da resposta: ");
    Serial.println(responseLength);

    if (authenticated && responseLength > 0) {
      Serial.print("[INFO] Resposta recebida: ");
      nfc.PrintHex(response, responseLength);
    }

    if (!authenticated) {
      Serial.println("[NEGADO] Falha na comunicação ou autenticação.");
      signalAccessDenied();
      delay(1000);
      return;
    }

    if (responseLength < 2) {
      Serial.println("[NEGADO] Resposta PWD_AUTH incompleta.");
      signalAccessDenied();
      delay(1000);
      return;
    }

    // Confirma os dois bytes do PACK retornado pela tag
    if (response[0] != EXPECTED_PACK[0] || response[1] != EXPECTED_PACK[1]) {
      Serial.println("[NEGADO] PACK diferente do esperado.");
      Serial.print("[INFO] PACK esperado: ");
      nfc.PrintHex(EXPECTED_PACK, 2);
      signalAccessDenied();
      delay(1000);
      return;
    }

    Serial.println("[OK] Autenticação por senha realizada.");
    Serial.println("[OK] PACK confirmado: A1 B2.");

    // Leitura da página 4 após a autenticação
    uint8_t pageData[4] = {0};

    if (!nfc.mifareultralight_ReadPage(4, pageData)) {
      Serial.println("[ERRO] Falha na leitura da página 4.");
      Serial.println("[NEGADO] Acesso negado.");
      signalAccessDenied();
      delay(1000);
      return;
    }

    Serial.print("[INFO] Dados da página 4 (hexadecimal): ");
    nfc.PrintHex(pageData, 4);

    bool pageValid = pageData[0] == 0x00 &&
                     pageData[1] == 0x00 &&
                     pageData[2] == 0x00 &&
                     pageData[3] == 0x00;

    if (pageValid) {
      Serial.println("[OK] Acesso liberado.");
      digitalWrite(GREEN_LED, HIGH);
      delay(3000);
      digitalWrite(GREEN_LED, LOW);
    } else {
      Serial.println("[NEGADO] Dados da página 4 alterados.");
      Serial.println("[NEGADO] Acesso negado.");
      signalAccessDenied();
    }

    delay(1000);
  }
}
