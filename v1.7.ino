 //Occhio Clorophilla Carnival 2025
// Definizione dei pin per la CNC Shield V3 e i driver A4988
#define STEP_PIN1 2   // STEP per motore 1
#define DIR_PIN1 5    // DIR per motore 1
#define STEP_PIN2 3   // STEP per motore 2
#define DIR_PIN2 6    // DIR per motore 2
#define ENABLE_PIN 8   // Enable per tutti i motori
#define BUTTON_PIN 13   // Pin per il pulsante

// Contatori dei passi per i due motori
int stepCount1 = 0;
int stepCount2 = 0;

// Stato per il motore 1 e stato generale del sistema
int faseMotore1 = 0;
int statoSistema = -1;  // -1 = Stato di avvio, 0 = Primo stato, 1 = Pausa, 2 = Secondo stato, 8 = Arresto e ritorno a 0

// Variabili di temporizzazione
unsigned long lastStepTime1 = 0;
unsigned long lastStepTime2 = 0;
unsigned long currentTime;

// Variabili per il motore 1
int direzione1 = HIGH; // Direzione iniziale del motore 1


// Variabili per il motore 2
int delayStep2 = 50;     // Velocità iniziale del motore 2
int direzione2 = HIGH;   // Direzione iniziale del motore 2

// Variabili per la gestione della velocità e accelerazione del motore 1
int maxSpeed1 = 80;
int minSpeed1 = 30;
int acceleration1 = 3;
int currentDelayStep1 = maxSpeed1;

// Variabili per la gestione della velocità e accelerazione del motore 2
int maxSpeed2 = 100;
int minSpeed2 = 60;
int acceleration2 = 3;
int currentDelayStep2 = maxSpeed2;

void setup() {
  // Configurazione dei pin come output
  pinMode(STEP_PIN1, OUTPUT);
  pinMode(DIR_PIN1, OUTPUT);
  pinMode(STEP_PIN2, OUTPUT);
  pinMode(DIR_PIN2, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  // Abilita i motori
  digitalWrite(ENABLE_PIN, LOW);

  digitalWrite(DIR_PIN1, HIGH);   // Direzione iniziale del motore 1
  digitalWrite(DIR_PIN2, HIGH);   // Direzione iniziale del motore 2

  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  currentTime = millis();

  // Gestione del pulsante per cambiare stato
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(100);  // Debounce
    if (statoSistema == -1) {
      statoSistema = 0;  // Avvia il programma se era in stato di avvio
      Serial.println("Sistema avviato");
    } else if (stepCount1 >= 5 || stepCount1 <= -5) {
      statoSistema = 8;  // Cambia in stato di arresto se il programma è in esecuzione
      Serial.println("Sistema in arresto, ritorno alla posizione iniziale");
    }
    while (digitalRead(BUTTON_PIN) == HIGH);  // Attende il rilascio del pulsante
  } 

  switch (statoSistema) {
    case -1:
      // Stato di avvio: i motori sono fermi, in attesa di avvio con il pulsante
      break;
    
    case 0:
      // Primo ciclo del motore 1 con le specifiche di movimento
      gestisciMotore1();
      motore2SmoothMove();
      break;
      
    case 1:
      // Pausa di 1 secondo dopo completamento del primo ciclo
      //delay(200);
      //stepCount1 = 0;          // Resetta il contatore del motore 1
      //faseMotore1 = 0;         // Resetta la fase del motore 1
      statoSistema = 2;        // Passa allo stato 2
      break;
      
    case 2:
      // Secondo ciclo del motore 1 con le specifiche di movimento
      gestisciMotore1();
      motore2SmoothMove();
      break;
      
    case 8:
      // Stato di arresto e ritorno a 0
      gestisciArresto();
      return; 
  }
}

// Funzione per gestire le fasi del motore 1
void gestisciMotore1() {
  if (statoSistema == 0) {
    // Movimento del motore 1 in stato 0
    switch (faseMotore1) {
      case 0: // Va avanti fino a 45 step
        if (stepCount1 < 30) {
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1++;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } else {
          Serial.print("Pausa al passo 45, stato 0: ");
          Serial.println(stepCount1);
          faseMotore1 = 1;
          //delay(1000); // Pausa di 0,5 secondi
          currentDelayStep1 = maxSpeed1; // Resetta la velocità per la fase successiva
        }
        break;

      case 1: // Cambia direzione, va fino a -45 step
        if (stepCount1 > -30) {
          digitalWrite(DIR_PIN1, LOW); // Cambia direzione
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1--;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } 
        else if (stepCount1 >= -30) {
          Serial.print("Pausa al passo 150, stato 0: ");
          Serial.println(stepCount1);
          faseMotore1 = 2;
          //delay(100); // Pausa di 1 secondo
          currentDelayStep1 = maxSpeed1; // Resetta la velocità per la fase successiva
        } 
        break;
        
      case 2: // Cambia direzione, torna a 0 step
        if (stepCount1 < 0) {
          digitalWrite(DIR_PIN1, HIGH); // Cambia direzione
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1++;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } else if (stepCount1 >= 0) {
          Serial.print("Motore 1 completato a 200 step, stato 0.");
          Serial.println();
          statoSistema = 1; // Passa allo stato 1
          Serial.print("Cambio di stato: stato 1.");
          Serial.println();
          
        }
        break;
    }
  } else if (statoSistema == 2) {
    // Movimento del motore 1 in stato 2
    switch (faseMotore1) {
      case 0: // Va avanti fino a 40 step
        if (stepCount1 < 30) {
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1++;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } else {
          Serial.print("Pausa al passo 70, stato 2: ");
          Serial.println(stepCount1);
          faseMotore1 = 1;
          //delay(100); // Pausa
          currentDelayStep1 = maxSpeed1; // Resetta la velocità per la fase successiva
        }
        break;

      case 1: // Cambia direzione, va fino a -40 step
        if (stepCount1 > -30) {
          digitalWrite(DIR_PIN1, LOW); // Cambia direzione
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1--;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } 
         else if (stepCount1 >= -30) {
          Serial.print("Pausa al passo 150, stato 0: ");
          Serial.println(stepCount1);
          faseMotore1 = 2;
          //delay(100); // Pausa di 1 secondo
          currentDelayStep1 = maxSpeed1; // Resetta la velocità per la fase successiva
        } 
        break;

      case 2: // Cambia direzione, torna a 0 step
        if (stepCount1 < 0) {
          digitalWrite(DIR_PIN1, HIGH); // Cambia direzione
          if (currentTime - lastStepTime1 >= currentDelayStep1) {
            stepMotor(STEP_PIN1);
            stepCount1++;
            lastStepTime1 = currentTime;

            // Aumenta la velocità (decrementa il ritardo) fino a maxSpeed1
            if (currentDelayStep1 > minSpeed1) {
              currentDelayStep1 -= acceleration1;
              if (currentDelayStep1 < minSpeed1) currentDelayStep1 = minSpeed1;
            }
          }
        } else if (stepCount1 == 0) {
          Serial.println("Motore 1 completato a 250 step, stato 2.");
          //stepCount1 = 0;
          faseMotore1 = 0;
          statoSistema = 0; // Ritorna allo stato 0 dopo il completamento dello stato 2
          Serial.println("Cambio di stato: Stato 0");
          //delay(150); // Pausa
        }
        break;
    }
  }
}

// Funzione per muovere il motore 2 con accelerazione e decelerazione fluida
void motore2SmoothMove() {
  // Controlla se il motore 2 deve accelerare o decelerare
  if (currentTime - lastStepTime2 >= currentDelayStep2) {
    // Verifica se il motore ha raggiunto i limiti e cambia direzione se necessario
    if (stepCount2 >= 20) {
      direzione2 = LOW;  // Cambia direzione verso il basso
      digitalWrite(DIR_PIN2, direzione2);
      Serial.println("Limite superiore raggiunto: cambio direzione a sinistra");
    } 
    else if (stepCount2 <= -20) {
      direzione2 = HIGH; // Cambia direzione verso l'alto
      digitalWrite(DIR_PIN2, direzione2);
      Serial.println("Limite inferiore raggiunto: cambio direzione a destra");
    }

    // Esegue un passo del motore 2
    stepMotor(STEP_PIN2);

    // Aggiorna il conteggio dei passi per il motore 2
    if (direzione2 == HIGH) {
      stepCount2++;
    } else {
      stepCount2--;
    }

    // Aggiornamento della velocità del motore 2
    if (currentDelayStep2 > minSpeed2) {
      currentDelayStep2 -= acceleration2; // Accelerazione
      if (currentDelayStep2 < minSpeed2) currentDelayStep2 = minSpeed2;
    }
    else if (currentDelayStep2 < maxSpeed2) {
      currentDelayStep2 += acceleration2; // Decelerazione
      if (currentDelayStep2 > maxSpeed2) currentDelayStep2 = maxSpeed2;
    }

    // Aggiorna il tempo dell'ultimo passo per il motore 2
    lastStepTime2 = currentTime;
  }
}

// Funzione per il ritorno dei motori alla posizione di origine
void gestisciArresto() {
  // Movimento del motore 1 per tornare a 0
  if (stepCount1 != 0) {
    if (stepCount1 > 0) {
      direzione1 = LOW; // Imposta la direzione per tornare indietro
    } else {
      direzione1 = HIGH; // Imposta la direzione per andare avanti verso 0
    }
    digitalWrite(DIR_PIN1, direzione1);

    if (currentTime - lastStepTime1 >= currentDelayStep1) {
      stepMotor(STEP_PIN1);
      lastStepTime1 = currentTime;

      // Aggiorna il conteggio dei passi per il motore 1 in base alla direzione
      if (direzione1 == HIGH) {
        stepCount1++;
      } else {
        stepCount1--;
      }
    }
  } else {
    stepCount1 = 0; // Reset dello step counter per il motore 1
    Serial.println("Motore 1 fermato a step 0");
  }

  // Movimento del motore 2 per tornare a 0 (già esistente)
  if (stepCount2 != 0) {
    if (stepCount2 > 0) {
      direzione2 = LOW; // Imposta la direzione per tornare indietro
    } else {
      direzione2 = HIGH; // Imposta la direzione per andare avanti verso 0
    }
    digitalWrite(DIR_PIN2, direzione2);

    if (currentTime - lastStepTime2 >= currentDelayStep2) {
      stepMotor(STEP_PIN2);
      lastStepTime2 = currentTime;

      // Aggiorna il conteggio dei passi per il motore 2 in base alla direzione
      if (direzione2 == HIGH) {
        stepCount2++;
      } else {
        stepCount2--;
      }
    }
  } else {
    stepCount2 = 0; // Reset dello step counter per il motore 2
    Serial.println("Motore 2 fermato a step 0");
  }

  // Controllo dello stato dei motori
  if (stepCount1 == 0 && stepCount2 == 0) {
    statoSistema = -1; // Torna allo stato di avvio
    Serial.println("Entrambi i motori a 0, sistema in attesa di avvio");
  }
}

// Funzione per eseguire un passo del motore
void stepMotor(int stepPin) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(5);
}
