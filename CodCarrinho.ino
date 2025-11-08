#define va 5
#define m1A 8
#define m2A 9

#define vb 6
#define m1B 10 
#define m2B 11

int velocidadeRecebida = 255;

void setup() {
  pinMode(va, OUTPUT);
  pinMode(m1A, OUTPUT);
  pinMode(m2A, OUTPUT);
  pinMode(vb, OUTPUT);
  pinMode(m1B, OUTPUT); 
  pinMode(m2B, OUTPUT);

  Serial.begin(9600); 
}

void loop() {
  if(Serial.available()>0){
    char valorRecebido = Serial.read();
    Serial.println(valorRecebido);
    
    switch(valorRecebido){
      case 'F':
        frente();
      break;
      case 'B':
        tras();
      break;
      case 'L':
        direita();
      break;
      case 'R':
        esquerda();
      break;
      case 'S':
       parar();
      break;
    }
  }
}

void parar(){
  digitalWrite(m1A, LOW);
  digitalWrite(m2A, LOW);
  digitalWrite(m1B, LOW);
  digitalWrite(m2B, LOW);
  velocidade(velocidadeRecebida);
}

void frente(){
  digitalWrite(m1A, HIGH);
  digitalWrite(m2A, LOW);
  digitalWrite(m1B, LOW);
  digitalWrite(m2B, HIGH);
  velocidade(velocidadeRecebida);
}

void tras(){
  digitalWrite(m1A, LOW);
  digitalWrite(m2A, HIGH);
  digitalWrite(m1B, HIGH);
  digitalWrite(m2B, LOW);
  velocidade(velocidadeRecebida);
}

void direita(){
  digitalWrite(m1A, LOW);
  digitalWrite(m2A, HIGH);
  digitalWrite(m1B, LOW);
  digitalWrite(m2B, HIGH);
  velocidade(velocidadeRecebida);
}

void esquerda(){
  digitalWrite(m1A, HIGH);
  digitalWrite(m2A, LOW);
  digitalWrite(m1B, HIGH);
  digitalWrite(m2B, LOW);
  velocidade(velocidadeRecebida);
}

void velocidade(int velocidade){
  analogWrite(va, velocidadeRecebida); 
  analogWrite(vb, velocidadeRecebida); 
}
