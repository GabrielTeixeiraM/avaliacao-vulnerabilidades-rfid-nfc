#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ (2)
#define PN532_RESET (3)
#define GREEN_LED (8)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// Chave A de autenticação do setor 1
uint8_t authenticationKey[6] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6};

void setup(void) {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);

  nfc.begin();
  uint32_t firmwareVersion = nfc.getFirmwareVersion();
  if (!firmwareVersion) {
    Serial.print("[ERRO] Falha na comunicação com o PN532.");
    while (1);
  }

  nfc.SAMConfig();
  Serial.println("[INFO] Sistema de segurança inicializado. Monitor de chaves ativo.");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success && uidLength == 4) {
    Serial.println("\n------------------------------------");
    Serial.print("[INFO] Cartão detectado. UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    Serial.print("[INFO] Autenticando o bloco 4 com a chave A: ");
    for (uint8_t i = 0; i < 6; i++) {
      if (authenticationKey[i] < 0x10)
        Serial.print("0");
      Serial.print(authenticationKey[i], HEX);
      if (i < 5)
        Serial.print(":");
    }
    Serial.println();

    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, authenticationKey);

    if (success) {
      Serial.println("[OK] Chave correta. Acesso liberado.");
      digitalWrite(GREEN_LED, HIGH);
      delay(3000);
      digitalWrite(GREEN_LED, LOW);
    } else {
      Serial.println("[NEGADO] Chave incorreta. Acesso negado.");
      for (int i = 0; i < 3; i++) {
        digitalWrite(GREEN_LED, HIGH);
        delay(100);
        digitalWrite(GREEN_LED, LOW);
        delay(100);
      }
    }
    Serial.println("------------------------------------");
    delay(1000);
  }
}
