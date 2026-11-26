#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ (2)
#define PN532_RESET (3)
#define GREEN_LED (8)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

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
  Serial.println("[INFO] Sistema alvo inicializado. Controle por nível de acesso.");
  Serial.println("[INFO] Aguardando leitura de cartão MIFARE...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success && uidLength == 4) {
    Serial.println("------------------------------------");
    Serial.println("[INFO] Cartão detectado. Autenticando o setor 1...");

    // Chave A padrão usada para autenticar o setor 1
    uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyA);

    if (success) {
      uint8_t blockData[16];

      success = nfc.mifareclassic_ReadDataBlock(4, blockData);

      if (success) {
        Serial.print("[INFO] Dados do bloco 4 (nível de acesso): ");
        nfc.PrintHex(blockData, 16);

        // O acesso de administrador exige os 16 bytes do bloco 4 iguais a 0xFF
        bool isAdmin = true;
        for (uint8_t i = 0; i < 16; i++) {
          if (blockData[i] != 0xFF) {
            isAdmin = false;
            break;
          }
        }

        if (isAdmin) {
          Serial.println("[OK] Acesso de administrador liberado.");
          digitalWrite(GREEN_LED, HIGH);
          delay(3000);
          digitalWrite(GREEN_LED, LOW);
        } else {
          Serial.println("[NEGADO] Privilégio insuficiente (nível básico). Acesso negado.");
        }
      } else {
        Serial.println("[ERRO] Falha na leitura do bloco 4.");
      }
    } else {
      Serial.println("[ERRO] Falha na autenticação do setor 1.");
    }

    delay(2000);
  }
}
