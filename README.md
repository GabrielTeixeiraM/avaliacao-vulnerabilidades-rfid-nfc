# Avaliação Experimental de Vulnerabilidades em Tags RFID/NFC Aplicadas ao Controle de Acesso

Repositório contendo os códigos-fonte desenvolvidos para o microcontrolador **Arduino Uno R3** utilizados na simulação do sistema de controle de acesso do Trabalho de Conclusão de Curso (TCC) apresentado ao Curso Superior de Tecnologia em Sistemas para Internet da **Universidade Tecnológica Federal do Paraná (UTFPR) — Câmpus Toledo (2026)**.

---

## Estrutura do Repositório

Cada diretório corresponde aos ensaios práticos descritos no **Capítulo 5 (Avaliação Experimental)** e aos comandos do **Apêndice A** do trabalho:

| Diretório | Ensaio Correspondente no TCC | Tecnologia / Arquitetura |
| :--- | :--- | :--- |
| `01_replicacao_uid/` | **5.1** — Replicação de Identificador Único | ISO/IEC 14443-A (UID) |
| `02_captura_senha_manipulacao_dados/` | **5.2.1** — Captura de Senha e Manipulação de Dados | NTAG213 (NFC Forum Type 2) |
| `03_congelamento_memoria_dos/` | **5.2.2** — Congelamento de Memória e Negação de Serviço | NTAG213 (NFC Forum Type 2) |
| `04_clonagem_interceptacao_mifare/` | **5.3.1** — Clonagem de Memória via Chaves Padrão<br>**5.3.2** — Ataque de Interceptação de Autenticação e Criptoanálise (*Sniffing*/mfkey64) | MIFARE Classic 1K (Crypto-1) |
| `05_escalacao_privilegios_memoria/` | **5.3.3** — Escalação de Privilégios via Manipulação de Dados em Memória | MIFARE Classic 1K (Crypto-1) |

> **Nota:** Os ensaios **5.3.1** e **5.3.2** compartilham o mesmo código-alvo no Arduino (`04_clonagem_interceptacao_mifare.ino`), pois em ambos o leitor valida o acesso por meio da autenticação do Bloco 4 com a chave customizada `A1:B2:C3:D4:E5:F6`.

---

## Ambiente Experimental

### Hardware
* **Microcontrolador:** Arduino Uno R3
* **Leitor RFID/NFC:** Módulo NXP PN532 (13,56 MHz, comunicação via barramento I²C)
* **Sinalização visual:** LED conectado ao pino digital `8` com resistor limitador
* **Credenciais avaliadas:** MIFARE Classic 1K (chip compatível Fudan), NTAG213 e *Magic Tag* Gen1a
* **Ferramenta de auditoria:** Proxmark3 Easy (*firmware* Iceman v4.17768)

### Software e Bibliotecas
* **Arduino IDE:** versão 2.8.3 (Monitor Serial configurado em `115200 baud`)
* **Bibliotecas:**
  * `Adafruit_PN532` — utilizada nos ensaios `01`, `03`, `04` e `05`.
  * `SeeedStudio_PN532` (`PN532` + `PN532_I2C`) — utilizada exclusivamente no ensaio `02_captura_senha_manipulacao_dados` para garantir a transmissão do comando `PWD_AUTH` (`0x1B`).

---

## Aviso Legal

Os códigos e procedimentos disponibilizados neste repositório têm finalidade estritamente acadêmica e educacional, destinados à análise de segurança e à conscientização sobre mecanismos de proteção em sistemas RFID e NFC em ambiente controlado.
