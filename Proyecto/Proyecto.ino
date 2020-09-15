#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <LiquidCrystal.h>
#include <Keypad.h>
#include "ListaEmpleados.h"
#include <String.h>
#include <Servo.h>
#include <Stepper.h>

//
#define BT2 19
#define BT1 18
//#define MT1 12
//#define MT2 11
#define LB1 10
#define LB2 9
//#define SV 13
#define TMP A0

Stepper ST1(12, 28, 29, 30, 31);
Servo servo;

using namespace std;

//vector para almacenar la clave
int password[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
int password_nuevo_usuario[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
int password_nuevo_usuario_temporal[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
int iden_empleado[8] = { -1, -1, -1, -1, -1, -1, -1, -1};

const int buzzer = 45;

//listado y usuario actual
lista *listado;
Empleado * CURRENT_EMPLOYEE;

String usuario_actual = "", clave_actual = "";
String usuario_nuevo0 = "", clave_nueva = "", clave_temp = "";

int usuario_nuevo, clave_usuario_nuevo;


int indice_clave = 0; //indice para guardar en el vector de claves
int var_cursor = 0;
int id_unico = 0; //identificador autoincrementable

int indice_user = 0;
int var_cursor_user = 0;
int indice_clave_aux = 0;
int var_cursor_aux = 0;

int opcion_actual = 0; //si 0 = control de ingreso; ...

//banderas booleanas
boolean ingresando = true;
boolean registrando = false;
boolean todo_bloqueado = false;
boolean clave_muy_larga = false;
boolean ok = false;

//clave admin 0106
int Admin_D1 = 0;
int Admin_D2 = 1;
int Admin_D3 = 0;
int Admin_D4 = 6;

char f = '*'; //caracter para cubrir la contraseña.

int veces = 0, incorrecto = 0; //seguridad de solo 3 intentos para ingresar la contraseña correcta.
int aviso = 0; //aviso para mostrar los intentos como seguridad para el usuario.

const byte filas = 4;
const byte columnas = 4;

byte pinesFilas[]  = {53, 52, 51, 50}; //A,B,C,D
byte pinesColumnas[] = {49, 48, 47, 46}; //1,2,3,4
//U = up, D = down, S = Second (2da funcion), C = clear, H = help, E = enter
char teclas[4][4] = {{'1', '2', '3', 'U'}, {'4', '5', '6', 'D'}, {'7', '8', '9', 'S'}, {'C', '0', 'H', 'E'}};

/**
   Caracteres especiales en lcd
**/
byte candado_cerrado[8] = {B01110, B10001, B10001, B11111, B11011, B11011, B11111, B00000};
byte candado_abierto[8] = {B01110, B00001, B00001, B11111, B11011, B11011, B11111, B00000};
byte chequeado[8] = {B00000, B00001, B00011, B10110, B11100, B01000, B00000, B00000};
byte caraSonriente[8] = {B00000, B01010, B01010, B01010, B00000, B10001, B01110, B00000};

Keypad mi_teclado = Keypad( makeKeymap(teclas), pinesFilas, pinesColumnas, filas, columnas);
//LiquidCrystal lcd2(27, 26, 25, 24, 23, 22); //RS,E,D4,D5,D6,D7
LiquidCrystal_I2C lcd(0x27, 16, 2);


char obtener_cantidad(int cant) {
  char res;
  switch (cant) {
    case 1:
      res = '1';
      break;
    case 2:
      res = '2';
      break;
    case 3:
      res = '3';
      break;
    case 4:
      res = '4';
      break;
  }
  return res;
}

boolean coinciden(int vector_a[], int vector_b[]) {

  for (int i = 0; i < 8; i++) {
    if (vector_a[i] != vector_a[i]) {
      Serial.println("NO IGUAL");
      return false;
    }
  }
  return true;
}

int obtener_info(int vector[]) {
  String s = "";
  int numero = -1;
  int resultado = -666;
  for (int i = 0; i < 8; i++) {
    numero = vector[i];
    if (numero != -1) {
      s = s + atoi(numero);
    }
  }
  resultado = atoi(s.c_str());
  Serial.println("datos vector");
  Serial.println(s);
  Serial.println("numero");
  Serial.println(resultado);
  return resultado;
}

void resetear_clave(int clave[]) {
  for (int i = 0; i < 8; i++) {
    clave[i] = -1;
  }
}

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2); //LCD (16 COLUMNAS Y 2 FILAS)

  //creando caracteres especiales
  lcd.createChar(0, candado_cerrado);
  lcd.createChar(1, candado_abierto);
  lcd.createChar(2, chequeado);
  lcd.createChar(3, caraSonriente);

  //instancia de listado de empleados y empleado actual
  listado = new lista();
  //listado->agregar(1, 555);
  CURRENT_EMPLOYEE = NULL;

  //LUCES
  pinMode(A2, OUTPUT); //luces lab 1
  pinMode(A3, OUTPUT); //luces lab 2
  pinMode(A4, OUTPUT); //luces entrada
  pinMode(A5, OUTPUT); //luces salida

  pinMode(A8, OUTPUT); //clave correcta led amarillo
  pinMode(A9, OUTPUT); //clave correcta led rojo
  pinMode(A10, OUTPUT); //clave correcta led verde nuevo usuario
  pinMode(buzzer, OUTPUT); //buzzer alarma
  //Serial.println("Terminal de Mensajes");
  //Serial.println();

  //Status lab
  pinMode(LB1, INPUT);
  pinMode(LB2, INPUT);

  //Motores
  servo.attach(13, 1100, 2000);
  servo.write(0);
  ST1.setSpeed(450);

  //temperatura
  pinMode(TMP, INPUT);
  /*  //movimiento servo motor proteus
    void simuladorServo(){
    analogWrite(PwmI,0);
    analogWrite(PwmD,0);
    ps++;
    servo.write(180);
    delay(5000);
    servo.write(0);
    delay(5000);
    }
  */
  opcion_actual = 0;
}

void loop() {

  switch (opcion_actual) {
    case 0:
      mensaje_bienvenida();
      while (ingresando) {
        control_de_ingreso();
      }
      break;
    //agregar acá el resto de opciones de la aplicacion

    case 1:
      mensaje_session();
      //Serial.println("pto el que lo lea jeje");
      opcion_actual = 2;
      break;

    case 2://
      int state = 0;

      if (Serial.available() > 0) {
        state = Serial.read();
        //Serial.println(state);
      }

      if (state == 'A') { //Bluetooth activa si Lab 1 encendido -> mueve banda a izquierda (abajo)
        tipoMovimiento('a');
      } else if (state == 'B') { //Bluetooth activa si Lab 2 encendido -> mueve banda a derecha (arriba)
        tipoMovimiento('b');
      } else if (state == 'C') { //Bluetooth activa el porton de la salida
        simuladorServo();
      } else if (state == 'E') { //Bluetooth desactiva leds de salida
        temperatura();
      } else if (state == '0') { //Bluetooth activa todos los leds
        manejoLeds(0);
      } else if (state == '1') { //Bluetooth desactiva todos los leds
        manejoLeds(1);
      } else if (state == '2') { //Bluetooth activa leds de lab 1
        manejoLeds(2);
      } else if (state == '3') { //Bluetooth desactiva leds de lab 1
        manejoLeds(3);
      } else if (state == '4') { //Bluetooth activa leds de lab 2
        manejoLeds(4);
      } else if (state == '5') { //Bluetooth desactiva leds de lab 2
        manejoLeds(5);
      } else if (state == '6') { //Bluetooth activa leds de entrada
        manejoLeds(6);
      } else if (state == '7') { //Bluetooth desactiva leds de entrada
        manejoLeds(7);
      } else if (state == '8') { //Bluetooth activa leds de salida
        manejoLeds(8);
      } else if (state == '9') { //Bluetooth desactiva leds de salida
        manejoLeds(9);
      }

      /*//Prueba bluetooth
        if(state == 'a'){
        digitalWrite(A4,HIGH);
        delay(1000);
        digitalWrite(A4,LOW);
        Serial.println("b");
        }
      */

      break;
  }
}

void temperatura() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enviando");
  lcd.setCursor(2, 1);
  lcd.print("temperatura");

  float val = analogRead(TMP);
  //Serial.println(val);
  float mv = (val / 1000) * 5000;
  float temp = (mv / 10) - 1;

  Serial.print(temp);
}

void manejoLeds(int i) {
  lcd.clear();
  switch (i) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Todos los leds");
      lcd.setCursor(2, 1);
      lcd.print("encendidos");
      
      digitalWrite(A2, HIGH);//lab1
      digitalWrite(A3, HIGH);//lab2
      digitalWrite(A4, HIGH);//entrada
      digitalWrite(A5, HIGH);//salida
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Todos los leds");
      lcd.setCursor(2, 1);
      lcd.print("apagados");

      digitalWrite(A2, LOW);
      digitalWrite(A3, LOW);
      digitalWrite(A4, LOW);
      digitalWrite(A5, LOW);
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Leds de LAB1");
      lcd.setCursor(2, 1);
      lcd.print("encendidos");

      digitalWrite(A2, HIGH);//lab1
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Leds de LAB1");
      lcd.setCursor(2, 1);
      lcd.print("apagados");

      digitalWrite(A2, LOW);
      break;

    case 4:
      lcd.setCursor(0, 0);
      lcd.print("Leds LAB2");
      lcd.setCursor(2, 1);
      lcd.print("encendidos");

      digitalWrite(A3, HIGH);//lab2
      break;

    case 5:
      lcd.setCursor(0, 0);
      lcd.print("Leds LAB2");
      lcd.setCursor(2, 1);
      lcd.print("apagados");

      digitalWrite(A3, LOW);
      break;

    case 6:
      lcd.setCursor(0, 0);
      lcd.print("Leds de entrada");
      lcd.setCursor(2, 1);
      lcd.print("encendidos");

      digitalWrite(A4, HIGH);//entrada
      break;

    case 7:
      lcd.setCursor(0, 0);
      lcd.print("Leds de entrada");
      lcd.setCursor(2, 1);
      lcd.print("apagados");

      digitalWrite(A4, LOW);
      break;

    case 8:
      lcd.setCursor(0, 0);
      lcd.print("Leds de salida");
      lcd.setCursor(2, 1);
      lcd.print("encendidos");

      digitalWrite(A5, HIGH);//salida
      break;

    case 9:
      lcd.setCursor(0, 0);
      lcd.print("Leds de salida");
      lcd.setCursor(2, 1);
      lcd.print("apagados");

      digitalWrite(A5, LOW);
      break;
  }
}

void simuladorServo() {
  //imprime objeto candado cerrado 0 abierto 1 cheque 2 cara 3

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write((byte)2);
  lcd.print("&ControlPorton&");
  lcd.setCursor(2, 1);
  lcd.write((byte)1);
  lcd.write((byte)3);
  lcd.print("Abriendo");
  lcd.write((byte)3);

  servo.write(180);
  delay(5000);

  int state;

  if (Serial.available() > 0) {
    state = Serial.read();
  }

  double primero = millis();
  double segundo = 0;
  Serial.println("comienza");
  while (state != 'D' && segundo < 6) {
    if (Serial.available() > 0) {
      state = Serial.read();
    }

    segundo = millis();
    segundo = (segundo - primero) / 1000;
  }
  digitalWrite(A8, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write((byte)2);
  lcd.print("&ControlPorton&");
  lcd.setCursor(2, 1);
  lcd.write((byte)0);
  lcd.write((byte)3);
  lcd.print("Cerrando");
  lcd.write((byte)3);

  servo.write(0);

  tone(buzzer, 2000);
  delay(1000);
  noTone(buzzer);

  delay(5000);
  digitalWrite(A8, LOW);

  opcion_actual = 1;
}

void tipoMovimiento(char tipo) {
  if (tipo == 'a') { //Bluetooth activa si Lab 1 encendido -> mueve banda a izquierda (abajo)
    if (digitalRead(LB1) == HIGH) { //Verifica que haya paquete en banda
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Moviendo hacia");
      lcd.setCursor(0, 1);
      lcd.print("izquierda");

      tone(buzzer, 1000);
      delay(1000);
      noTone(buzzer);

      moverBanda(-1);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("La muestra llego");
      lcd.setCursor(0, 1);
      lcd.print("al LAB2");

      tone(buzzer, 1500);
      delay(500);
      noTone(buzzer);

    } else { //Si no hay paquete tira error
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR: no hay");
      lcd.setCursor(0, 1);
      lcd.print("muestra en LAB1");
      delay(2000);
    }

  } else if (tipo == 'b') { //Bluetooth activa si Lab 2 encendido -> mueve banda a derecha (arriba)
    if (digitalRead(LB2) == HIGH) { //Verifica que haya paquete en banda
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Moviendo hacia");
      lcd.setCursor(0, 1);
      lcd.print("derechas");

      tone(buzzer, 3000);
      delay(500);
      noTone(buzzer);

      moverBanda(1);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("La muestra llego");
      lcd.setCursor(0, 1);
      lcd.print("al LAB1");

      tone(buzzer, 2500);
      delay(500);
      noTone(buzzer);

    } else { //Si no hay paquete tira error
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR: no hay ");
      lcd.setCursor(0, 1);
      lcd.print("muestra en LAB2");
      delay(2000);
    }

  }
  opcion_actual = 2;

}

void moverBanda(int i) {
  if (i == -1) {
    while (digitalRead(LB1) == HIGH) {
      ST1.step(256 * i);
    }
  } else {
    while (digitalRead(LB2) == HIGH) {
      ST1.step(256 * i);
    }
  }
}

void mensaje_session() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(" BIENVENIDO: ");
  lcd.setCursor(1, 2);
  lcd.print(usuario_actual);
}

void mensaje_bienvenida() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(byte(0));
  lcd.print(" BIENVENIDO ");
  lcd.write(byte(0));
  delay(2000);
}

void control_de_ingreso() {
  lcd.clear();
  Serial.println("Pidiendo Usuario");
  indice_clave = 0;
  var_cursor = 0;
  usuario_actual = "";
  clave_actual = "";
  while (ok && !registrando) {
    pedir_usuario(); //en este proceso me puede pedir registrar
  }
  Serial.println("ESTE ES EL USUARIO");
  Serial.println(usuario_actual);
  Serial.println();
  lcd.clear();
  Serial.println("Pidiendo clave");
  indice_clave = 0;
  var_cursor = 0;
  ok = true;
  while (ok  && !registrando) {
    pedir_password(); //en este proceso me puede pedir registrar
  }
  if (!registrando) {
    Serial.println("ESTA ES LA CLAVE");
    Serial.println(clave_actual);
    Serial.println();
    verificar_credenciales();
  }
  registrando = false;
  ok = true;
}

void pedir_usuario() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        iden_empleado[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        usuario_actual.remove(indice_clave);
        indice_clave--;
        var_cursor--;
      }
      if (indice_clave == 0) {
        iden_empleado[indice_clave] = -1;
        usuario_actual.remove(indice_clave);
        usuario_actual = "";
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("PRESIONE 0000");
      lcd.setCursor(1, 1);
      lcd.print("ENTER REGISTRAR");
      delay(1000);
      lcd.clear();
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER");
      if (iden_empleado[0] == 0 && iden_empleado[1] == 0 && iden_empleado[2] == 0 && iden_empleado[3] == 0 && iden_empleado[4] == -1 && iden_empleado[5] == -1 && iden_empleado[6] == -1 && iden_empleado[7] == -1) {
        registrando = true;
        registrar_usuario();
      } else {
        ok = false;
      }
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        usuario_actual.concat(tecla_presionada);
        tecla_presionada = tecla_presionada - 48;
        iden_empleado[indice_clave] = tecla_presionada;

        indice_clave++;
        var_cursor++;
        //Serial.println("ENTER");
      }
      else {
        Serial.println("ERROR ID MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_id_muy_largo();
          usuario_actual = "";
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(0, 0), lcd.print("INGRESE ID"); //Pidiendo clave
  }
}

void error_id_muy_largo() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("ID TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(iden_empleado);
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("INGRESE ID");
      clave_muy_larga = false;
    }
  }
}

void pedir_password() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        password[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        clave_actual.remove(indice_clave);
        indice_clave--;
        var_cursor--;

      }
      if (indice_clave == 0) {
        password[indice_clave] = -1;
        clave_actual.remove(indice_clave);
        clave_actual = "";
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("PRESIONE 0000");
      lcd.setCursor(1, 1);
      lcd.print("ENTER REGISTRAR");
      delay(1000);
      lcd.clear();
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER");
      if (password[0] == 0 && password[1] == 0 && password[2] == 0 && password[3] == 0 && password[4] == -1 && password[5] == -1 && password[6] == -1 && password[7] == -1) {
        registrando = true;
        registrar_usuario();
      } else {
        ok = false;
      }
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        clave_actual.concat(tecla_presionada);
        tecla_presionada = tecla_presionada - 48;
        password[indice_clave] = tecla_presionada;

        indice_clave++;
        var_cursor++;
        //Serial.println("ENTER");
      }
      else {
        Serial.println("ERROR ID MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_clave_muy_larga();
          clave_actual = "";
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(0, 0), lcd.print("DIGITE SU CLAVE"); //Pidiendo clave
  }
}

void error_clave_muy_larga() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("PASS TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(password);
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("DIGITE SU CLAVE");
      clave_muy_larga = false;
    }
  }
}

void verificar_credenciales() {
  int id = usuario_actual.toInt();
  int clave = clave_actual.toInt();
  Serial.println("ID USUARIO");
  Serial.println(id);
  Serial.println("CLAVE USUARIO");
  Serial.println(clave);
  CURRENT_EMPLOYEE = listado->buscar_empleado(id, clave);
  if (CURRENT_EMPLOYEE == NULL) {
    Serial.println("DATOS INCORRECTOS");
    veces++;
    if (veces < 4) {
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("  ERROR     "), lcd.print(veces);
      lcd.setCursor(0, 1), lcd.print("CLAVE INCORRECTA");
      digitalWrite(A9, HIGH);
      delay(1500);
      digitalWrite(A9, LOW);
    }
    else {
      todo_bloqueado = true;
      digitalWrite(buzzer, HIGH);
      digitalWrite(A9, HIGH);
      delay(2000);
      digitalWrite(buzzer, LOW);
      indice_clave = 0;
      var_cursor = 0;
      lcd.clear();
      while (todo_bloqueado) {
        desbloquear();
      }
      veces = 0;
      digitalWrite(A9, LOW);

    }

  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0), lcd.print("   OK ACCESO   ");
    lcd.setCursor(0, 1), lcd.print("   PERMITIDO   ");
    digitalWrite(buzzer, HIGH);
    digitalWrite(A8, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A4, HIGH);
    digitalWrite(A5, HIGH);
    delay(2000);
    digitalWrite(buzzer, LOW);
    Serial.println("CLAVE CORRECTA");
    opcion_actual = 1;
    ingresando = false;
  }
}

void registrar_usuario() {
  Serial.println("REGISTRANDO");

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write((byte)0);
  lcd.print(" REGISTRANDO ");
  lcd.write((byte)0);
  delay(2000);
  lcd.clear();

  indice_clave = 0;
  var_cursor = 0;
  ok = true;
  while (ok) {
    pedir_password_nuevo_user();
  }
  indice_clave = 0;
  var_cursor = 0;
  Serial.println("REINSERTANDO CLAVE");
  lcd.clear();
  ok = true;
  while (ok) {
    reingresar_clave();
  }
  indice_clave = 0;
  var_cursor = 0;
  Serial.println("ESPERANDO CLAVE GERENTE");
  lcd.clear();
  ok = true;
  while (ok) {
    pedir_clave_gerente();
  }
  Serial.println("CREANDO EMPLEADO");
  int clave = clave_nueva.toInt();
  listado->agregar(id_unico, clave);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("ID "), lcd.print(id_unico);
  lcd.setCursor(0, 1);
  lcd.print(" PASS "), lcd.print(clave);
  digitalWrite(A10, HIGH);
  delay(1500);
  digitalWrite(A10, LOW);
  Serial.println("CREADO EXITOSAMENTE");
  Serial.println();
  Serial.println("ID USUARIO ");
  Serial.println(id_unico);
  Serial.println("CLAVE ");
  Serial.println(clave);
}

void pedir_password_nuevo_user() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        password_nuevo_usuario[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        clave_nueva.remove(indice_clave);
        indice_clave--;
        var_cursor--;
      }
      if (indice_clave == 0) {
        password_nuevo_usuario[indice_clave] = -1;
        clave_nueva.remove(indice_clave);
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("CLAVE MENOR DE 8");
      lcd.setCursor(1, 1);
      lcd.print("ENTER ACEPTAR");
      delay(1000);
      lcd.clear();
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER");
      ok = false;
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        clave_nueva.concat(tecla_presionada);
        tecla_presionada = tecla_presionada - 48;
        password_nuevo_usuario[indice_clave] = tecla_presionada;
        indice_clave++;
        var_cursor++;
        //Serial.println("ENTER");
      }
      else {
        Serial.println("ERROR ID MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_clave_muy_larga_new_user();
          clave_nueva = "";
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(0, 0), lcd.print("DIGITE SU CLAVE"); //Pidiendo clave
  }
}

void error_clave_muy_larga_new_user() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("PASS TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(password_nuevo_usuario);
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("DIGITE SU CLAVE");
      clave_muy_larga = false;
    }
  }
}

void reingresar_clave() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        password_nuevo_usuario_temporal[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        clave_temp.remove(indice_clave);
        indice_clave--;
        var_cursor--;
      }
      if (indice_clave == 0) {
        password_nuevo_usuario_temporal[indice_clave] = -1;
        clave_temp.remove(indice_clave);
        clave_temp = "";
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("CLAVE MENOR DE 8");
      lcd.setCursor(1, 1);
      lcd.print("ENTER ACEPTAR");
      delay(1000);
      lcd.clear();
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER"); //aceptar usuario
      int pass1 = clave_nueva.toInt();
      int pass2 = clave_temp.toInt();
      boolean coincidencia = coinciden(password_nuevo_usuario, password_nuevo_usuario_temporal);
      //if (coincidencia) {
      if (pass1 == pass2) {
        Serial.println("PASS COINCIDEN");
        clave_usuario_nuevo = obtener_info(password_nuevo_usuario);
        Serial.println();
        ok = false;
      }
      else {
        lcd.clear();
        clave_muy_larga = true;
        while (clave_muy_larga) {
          lcd.setCursor(0, 0), lcd.print("PASS NO COINCIDE");
          lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
          char tecla = mi_teclado.getKey();
          if (tecla == 'E') {
            indice_clave = 0;
            var_cursor = 0;
            clave_temp = "";
            resetear_clave(password_nuevo_usuario_temporal);
            lcd.clear();
            lcd.setCursor(0, 0), lcd.print("REINGRESE CLAVE");
            clave_muy_larga = false;
          }
        }
      }
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        clave_temp.concat(tecla_presionada);
        tecla_presionada = tecla_presionada - 48;
        password_nuevo_usuario_temporal[indice_clave] = tecla_presionada;
        indice_clave++;
        var_cursor++;
      }
      else {
        Serial.println("ERROR CLAVE MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_clave_muy_larga_new_user_reingreso();
          clave_temp = "";
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(0, 0), lcd.print("REINGRESE CLAVE");
  }
}

void error_clave_muy_larga_new_user_reingreso() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("PASS TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(password_nuevo_usuario_temporal);
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("REINGRESE CLAVE");
      clave_muy_larga = false;
    }
  }
}

void pedir_clave_gerente() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        password[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        indice_clave--;
        var_cursor--;
      }
      if (indice_clave == 0) {
        password[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER");
      if (password[0] == Admin_D1 && password[1] == Admin_D2 && password[2] == Admin_D3 && password[3] == Admin_D4 && password[4] == -1 && password[5] == -1 && password[6] == -1 && password[7] == -1) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("BIENVENIDO");
        lcd.setCursor(4, 1);
        lcd.print("GERENTE");
        digitalWrite(A8, HIGH);
        delay(1000);
        lcd.clear();
        digitalWrite(A8, LOW);
        veces = 0;
        ok = false;
        id_unico++;
      }
      else {
        if (veces < 4) {
          veces++;
          lcd.clear();
          lcd.setCursor(0, 0), lcd.print("ERROR    "), lcd.print(veces);
          lcd.setCursor(0, 1), lcd.print("CLAVE INCORRECTO");
          digitalWrite(A9, HIGH);
          //digitalWrite(A11, HIGH);
          delay(1500);
          digitalWrite(A9, LOW);
          //digitalWrite(A11, LOW);
          indice_clave = 0;
          var_cursor = 0;
          resetear_clave(password);
          lcd.clear();
          lcd.setCursor(0, 0), lcd.print("CLAVE GERENTE");
        }
        else {
          lcd.clear();
          clave_muy_larga = true;
          digitalWrite(A9, HIGH);
          while (clave_muy_larga) {
            lcd.setCursor(0, 0), lcd.print("SISTEMA BLOQUEADO");
            lcd.setCursor(0, 1), lcd.print("AVISE AL GERENTE");
            char tecla = mi_teclado.getKey();
            if (tecla == 'E') {
              indice_clave = 0;
              var_cursor = 0;
              resetear_clave(password);
              lcd.clear();
              lcd.setCursor(0, 0), lcd.print("CLAVE GERENTE"); //Pidiendo clave
              clave_muy_larga = false;
              digitalWrite(A9, LOW);
            }
          }
        }
      }
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        tecla_presionada = tecla_presionada - 48;
        password[indice_clave] = tecla_presionada;
        indice_clave++;
        var_cursor++;
        //Serial.println("ENTER");
      }
      else {
        Serial.println("ERROR CLAVE MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_clave_muy_larga_gerente();
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(0, 0), lcd.print("CLAVE GERENTE"); //Pidiendo clave
  }
}

void error_clave_muy_larga_gerente() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("PASS TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(password);
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print("CLAVE GERENTE"); //Pidiendo clave
      clave_muy_larga = false;
    }
  }
}

void desbloquear() {
  char tecla_presionada = mi_teclado.getKey();
  if (tecla_presionada) {
    lcd.setCursor(5 + var_cursor, 1);
    if (tecla_presionada == 'U') {
      Serial.println("UP");
    }
    else if (tecla_presionada == 'D') {
      Serial.println("DOWN");
    }
    else if (tecla_presionada == 'S') {
      Serial.println("2ND");
    }
    else if (tecla_presionada == 'C') {
      Serial.println("CLEAR");
      if (var_cursor > 0 && indice_clave > 0) {
        password[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
        indice_clave--;
        var_cursor--;
      }
      if (indice_clave == 0) {
        password[indice_clave] = -1;
        lcd.print(' '), lcd.setCursor(5 + var_cursor, 1), lcd.print(' '); //imprimimos el caracter en el lcd
      }
    }
    else if (tecla_presionada == 'H') {
      Serial.println("HELP");
    }
    else if (tecla_presionada == 'E') {
      Serial.println("ENTER");
      if (password[0] == Admin_D1 && password[1] == Admin_D2 && password[2] == Admin_D3 && password[3] == Admin_D4 && password[4] == -1 && password[5] == -1 && password[6] == -1 && password[7] == -1) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("BIENVENIDO");
        lcd.setCursor(4, 1);
        lcd.print("GERENTE");
        digitalWrite(A8, HIGH);
        delay(2500);
        lcd.clear();
        digitalWrite(A8, LOW);
        veces = 0;
        todo_bloqueado = false;
      }
      else {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("BAD PASSWORD");
        lcd.setCursor(0, 1);
        lcd.print("AVISE GERENTE");
        digitalWrite(buzzer, HIGH);
        delay(1500);
        digitalWrite(buzzer, LOW);
        lcd.clear();

        indice_clave = 0;
        var_cursor = 0;
        resetear_clave(password);
      }
    }
    else { //se presiono un numero
      Serial.print("Tecla: ");
      Serial.println(tecla_presionada);
      if (indice_clave < 8 && var_cursor < 8) {
        lcd.print(tecla_presionada), lcd.setCursor(5 + var_cursor, 1), lcd.print(tecla_presionada); //imprimimos el caracter en el lcd
        tecla_presionada = tecla_presionada - 48;
        password[indice_clave] = tecla_presionada;
        indice_clave++;
        var_cursor++;
        //Serial.println("ENTER");
      }
      else {
        Serial.println("ERROR CLAVE MUY GRANDE");
        clave_muy_larga = true;
        if (clave_muy_larga) {
          error_clave_muy_larga_gerente();
          Serial.println("indice actual: ");
          Serial.println(indice_clave);
          Serial.println("cursor actual: ");
          Serial.println(var_cursor);
          clave_muy_larga = false;
        }
      }
    }
  }
  if (!tecla_presionada) {
    lcd.setCursor(1, 0), lcd.print("CLAVE GERENTE"); //Pidiendo clave
  }
}

void error_clave_muy_larga_desbloqueo() {
  lcd.clear();
  while (clave_muy_larga) {
    lcd.setCursor(0, 0), lcd.print("PASS TOO LONG");
    lcd.setCursor(3, 1), lcd.print("PRESS ENTER");
    char tecla = mi_teclado.getKey();
    if (tecla == 'E') {
      indice_clave = 0;
      var_cursor = 0;
      resetear_clave(password);
      lcd.clear();
      lcd.setCursor(1, 0), lcd.print("CLAVE GERENTE"); //Pidiendo clave
      clave_muy_larga = false;
    }
  }
}
