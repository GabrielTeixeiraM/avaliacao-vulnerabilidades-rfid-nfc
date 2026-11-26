#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ (2)
#define PN532_RESET (3)
#define GREEN_LED (8)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// UID autorizado no experimento, extraído via Proxmark3
uint8_t authorizedUid[4] = {0xB0, 0x18, 0x5C, 0x32};

void setup(void) {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);

  nfc.begin();

  uint32_t firmwareVersion = nfc.getFirmwareVersion();
  if (!firmwareVersion) {
    Serial.print("[ERRO] PN532 não encontrado. Verifique os cabos e a chave DIP.");
    while (1);
  }

  nfc.SAMConfig();

  Serial.println("[INFO] Sistema alvo PN532 inicializado. Controle de acesso por UID.");
  Serial.println("[INFO] Aguardando leitura de tag...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.print("[INFO] Tag detectada. UID:");

    bool match = (uidLength == 4);

    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" ");
      Serial.print(uid[i], HEX);
      if (i < 4 && uid[i] != authorizedUid[i]) {
        match = false;
      }
    }
    Serial.println("");

    if (match) {
      Serial.println("[OK] Acesso liberado.");
      digitalWrite(GREEN_LED, HIGH);
      delay(3000);
      digitalWrite(GREEN_LED, LOW);
    } else {
      Serial.println("[NEGADO] Credencial desconhecida. Acesso negado.");
      for (int i = 0; i < 3; i++) {
        digitalWrite(GREEN_LED, HIGH);
        delay(100);
        digitalWrite(GREEN_LED, LOW);
        delay(100);
      }
    }

    // Intervalo entre leituras do mesmo cartão
    delay(1000);
  }
}
