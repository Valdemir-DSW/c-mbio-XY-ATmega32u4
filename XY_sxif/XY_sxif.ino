#include <Joystick.h>

// ========== CONFIGURAÇÕES DE HARDWARE ==========
#define PINO_X   A0
#define PINO_Y   A1
#define PINO_REV 7   // Entrada botão ré (porta aterrada)

#define DEBUG true  // Ativa saída serial para debug

// True Shoulders (valores físicos do pot)
#define X_MIN 100
#define X_MAX 900
#define Y_MIN 120
#define Y_MAX 880

// ========== DIVISÕES PERSONALIZADAS (0-1023, NORMALIZADO) ==========
// Ajuste fino para as divisões do câmbio conforme sua mecânica
// Devem obedecer: 0 < X_DIV1 < X_DIV2 < 1023 e 0 < Y_DIV1 < Y_DIV2 < 1023
#define X_DIV1 320  // limite entre esquerda e centro no eixo X
#define X_DIV2 700  // limite entre centro e direita no eixo X

#define Y_DIV1 400  // limite entre trás e neutro no eixo Y
#define Y_DIV2 620  // limite entre neutro e frente no eixo Y

// ========== INVERTER EIXOS SE NECESSÁRIO ==========
#define INVERTER_EIXO_X false
#define INVERTER_EIXO_Y false

// ========== OBJETO JOYSTICK ==========
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_GAMEPAD,
                   7, 0, // 7 botões para 6 marchas + ré
                   false, false, false,
                   false, false, false,
                   false, false, false);

int marchaAtual = 0;

void setup() {
  pinMode(PINO_REV, INPUT_PULLUP);
  Joystick.begin();

  if (DEBUG) {
    Serial.begin(115200);
    Serial.println(F("DEBUG ativado"));
  }
}

void loop() {
  // Leitura bruta dos potenciômetros
  int rawX = analogRead(PINO_X);
  int rawY = analogRead(PINO_Y);

  // Normaliza entre 0 e 1023 com True Shoulders e inverte se configurado
  int normX = map(rawX, X_MIN, X_MAX, 0, 1023);
  int normY = map(rawY, Y_MIN, Y_MAX, 0, 1023);

  if (INVERTER_EIXO_X) normX = 1023 - normX;
  if (INVERTER_EIXO_Y) normY = 1023 - normY;

  // Detecta marcha baseado nas divisões personalizadas
  int marcha = detectarMarcha(normX, normY);

  // Botão de ré: só ativa ré se estiver na 1ª marcha + botão apertado
  if (marcha == 1 && digitalRead(PINO_REV) == LOW) {
    marcha = 7;  // ré
  }

  // Atualiza só se mudar
  if (marcha != marchaAtual) {
    atualizarJoystick(marcha);
    marchaAtual = marcha;
  }

  if (DEBUG) {
    Serial.print(F("Raw X=")); Serial.print(rawX);
    Serial.print(F(" Norm X=")); Serial.print(normX);
    Serial.print(F(" | Raw Y=")); Serial.print(rawY);
    Serial.print(F(" Norm Y=")); Serial.print(normY);
    Serial.print(F(" | Marcha=")); Serial.print(marcha);
    Serial.print(F(" | Botão Ré=")); Serial.println(digitalRead(PINO_REV) == LOW ? "Ativado" : "Solto");
  }

  delay(15);
}

int detectarMarcha(int x, int y) {
  // Y: trás < Y_DIV1 < neutro < Y_DIV2 < frente
  // X: esquerda < X_DIV1 < centro < X_DIV2 < direita

  if (y < Y_DIV1) {
    // atrás - marchas 2,4,6
    if (x < X_DIV1) return 2;
    else if (x < X_DIV2) return 4;
    else return 6;
  }
  else if (y > Y_DIV2) {
    // frente - marchas 1,3,5
    if (x < X_DIV1) return 1;
    else if (x < X_DIV2) return 3;
    else return 5;
  }
  else {
    // neutro, qualquer x
    return 0;
  }
}

void atualizarJoystick(int marcha) {
  // libera todos os botões
  for (int i = 0; i < 7; i++) {
    Joystick.releaseButton(i);
  }

  // pressiona só o botão da marcha atual (1-7)
  if (marcha > 0 && marcha <= 7) {
    Joystick.pressButton(marcha - 1);
  }
}