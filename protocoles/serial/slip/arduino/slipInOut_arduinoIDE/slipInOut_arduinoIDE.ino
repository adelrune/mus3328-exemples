// Variables globales
// Ces variables servent à l'encodage en SLIP.
const byte END=192;
const byte ESC=219; 
const byte ESC_END=220;
const byte ESC_ESC=221;

const int sensorPin = 0; //L'entrée analogique utilisée.
const int outPins[] = { 9, 10, 11 }; // Les pins utilisées en sortie (PWM).

// La taille maximum d'un paquet SLIP.  Ce nombre doit être plus grande que le
// paquet le plus grand attendu.
byte slipPacket[256]; // Cet array contiendra le paquet SLIP reçu.

void setup() {
  Serial.begin(115200);
  for( int i=0 ; i < (sizeof(outPins)/sizeof(int)) ; i++) {
    pinMode(outPins[i], OUTPUT); // Les outPins sont initialisées en OUTPUT.
  }
}

void loop() {
  // Réception des paquets SLIP (sans aucun délai).
	int packetSize = 0;
  int i;
	int sensor = analogRead(sensorPin);
  packetSize = SLIPSerialRead( slipPacket );
  for (i=0 ; i < packetSize; i++) {
    analogWrite(outPins[i], slipPacket[i]);
  }
	// On lit le capteur et on envoie un paquet SLIP.
  Serial.write(END);   // On commence le paquet.
  SLIPSerialWrite( byte(sensor >> 8) );  // On envoie le MSB
  SLIPSerialWrite( byte(sensor & 255) ); // On envoie le LSB
  Serial.write(END);   // On termine le paquet.
	delay(3);  // On attend un peu pour le port série et l'ADC. 
}

//Fonction pour encoder les paquets SLIP.
void SLIPSerialWrite(byte value){
  if(value == END) { // Si c'est la valeur 192, on remplace par 219 et 220.
    Serial.write(ESC);
    Serial.write(ESC_END);
    return;
  } else if (value == ESC) {  // Si c'est la valeur 219, on fait suivre un 221.
    Serial.write(ESC);
    Serial.write(ESC_ESC);
    return;
  } else { // On envoie toutes les autres valeurs normalement.
    Serial.write(value);
    return;
  }
}

// Fonction pour recevoir et décoder les paquets SLIP.
// Retourne la taille du paquet SLIP.
// Prend un paramètre: le tableau (array) d'octets (bytes) qui contiendra le paquet SLIP.
int SLIPSerialRead(byte * slipPacket) { 
	int packetIndex = 0;  // L'index du paquet SLIP reçu.  Cette valeur sera
	// incrémentée à chaque octet reçu.  Elle sera remise à
	// zéro quand un paquet SLIP sera complet.

	// Deux témoins (flags)
	boolean escape = false; // Ce témoin devient vrai quand un octet ESC
	// est reçu.  Il redevient faux quand l'octet
	// suivant (ESC_ESC ou ESC_END) est reçu.
	boolean packetComplete = false; // Ce témoin devient vrai quand un
	// paquet SLIP est complet,
	// c'est-à-dire quand l'octet END est
	// reçu.

  // S'il n'y a rien sur le port série, on retourne zéro immédiatement.
  if ( Serial.available() == 0 ) {
    return 0;
  }

  // S'il y a quelque chose, on attend un paquet complet.
	while( !packetComplete ) {
		if( Serial.available() > 0) {
			byte b = Serial.read();
			if (escape) { // Si l'octet précédent était ESC (219)
				if (b == ESC_END) {  // On ajoute END (192)...
					slipPacket[packetIndex] = END;
				} else if (b == ESC_ESC) { // ...ou ESC (219)
					slipPacket[packetIndex] = ESC;
				}
				packetIndex++;  
				escape = 0; // On remet le témoin escape à faux.
			} else if (b == END) { // Le paquet est terminé.
				packetComplete = true;  // Le témoin packetComplete est maintenant vrai.
			} else if (b == ESC) { // Si l'octet est ESC (219)
				escape = 1; // Le témoin escape est maintenant vrai. (On n'incrémente pas packetIndex.)
			} else { // Cas normal.
				slipPacket[packetIndex] = b; // On ajoute l'octet au paquet et
				packetIndex ++; // on incrémente packetIndex.
			}
		}
	}
  // On retourne la taille du paquet.
  return packetIndex;
}
