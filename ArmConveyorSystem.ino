#include <PS2X_lib.h>  //for v1.6
#include <Servo.h> //servo
#include <Wire.h> //I/O
#include <LiquidCrystal_I2C.h>//lcd
#include <Keypad.h> //Keypad
//PS2
PS2X ps2x; // create PS2 Controller Class
int error = 0;
byte type = 0;
byte vibrate = 0;


//DELAY
int DELAY_1 = 10; //change set delay
int DELAY_2 = 700;
int delay_smooth = 10;

//SERVO
Servo servo_1;
Servo servo_2;
Servo servo_3;
Servo servo_4;
Servo servo_5;
Servo servo_6;
Servo servo_7;

int degree_change = 15;//step

int degree_servo_1 = 90;//MID
int degree_servo_2 = 120;
int degree_servo_3 = 90;
int degree_servo_4 = 90;
int degree_servo_5 = 90;
int degree_servo_6 = 90;
int degree_servo_7 = 90;

int max_degree_servo_1 = 180;//MAX
int max_degree_servo_2 = 180;
int max_degree_servo_3 = 180;
int max_degree_servo_4 = 180;
int max_degree_servo_5 = 180;
int max_degree_servo_6 = 180;
int max_degree_servo_7 = 180;

int min_degree_servo_1 = 90;//MIN
int min_degree_servo_2 = 0;
int min_degree_servo_3 = 0;
int min_degree_servo_4 = 0;
int min_degree_servo_5 = 0;
int min_degree_servo_6 = 0;
int min_degree_servo_7 = 0;

//LCD
//LiquidCrystal_I2C lcd(0x3F, 16, 2); //16x2 col row ID 3f
LiquidCrystal_I2C lcd(0x27, 20, 4); //20x4 col row ID 3f

//KEYPAD
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

int Set;
String Value = "";
int A = 180;
int B = 90;
int C = 60;

byte rowPins[ROWS] = {22, 24, 26, 28}; // connect D2,D3,D4,D5 to Rows 1-4 (the left four pins)
byte colPins[COLS] = {30, 32, 34, 36}; // connect D8,D9,D10,D11 to Column 1-4 (the right four pins)

char key;
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int countB = 0;
int countC = 0;
int MAXB = 5;
int MAXC = 5;

//STATUS
int mode = 3; //1 Ready /2 stop /3 don't care

//Sensor metal
int monitoring;
int metalDetection = 1;//pin A1
int check_metal = 0; //metal 1 /non metal 2

//sensor IR
int IR;
int IRDetection = 0;//pin A0
int check_IR = 0;

//sorinoid and conver
int soli_1 = 53;
int soli_2 = 51;
int conver = 49;


void setup()
{
  Serial.begin(57600);//Baud rate
  mode =3;//don't care
  //PS2

  error = ps2x.config_gamepad(13, 11, 10, 12, true, true); //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error

  if (error == 0)
  {
    Serial.println("Found Controller, configured successful");
  }
  else if (error == 1)
  {
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
  }
  else if (error == 2)
  {
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  }
  else if (error == 3)
  {
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  }
  type = ps2x.readType();
  switch (type)
  {
    case 0:
      Serial.println("Unknown Controller type");
      break;
    case 1:
      Serial.println("DualShock Controller Found");
      break;
    case 2:
      Serial.println("GuitarHero Controller Found");
      break;
  }

  //set pin and write servo
  servo_1.attach(2);  //the pin for the servo control
  servo_1.write(degree_servo_1); //set initial servo position if desired
  servo_2.attach(3);
  servo_2.write(degree_servo_2);
  servo_3.attach(4);
  servo_3.write(degree_servo_3);
  servo_4.attach(5);
  servo_4.write(degree_servo_4);
  servo_5.attach(6);
  servo_5.write(degree_servo_5);
  servo_6.attach(7);
  servo_6.write(degree_servo_6);
  servo_7.attach(8);
  servo_7.write(degree_servo_7);

  //set KEYPAD
  keypad.setDebounceTime( 20 ); // 20 msec

  //set start LCD
  lcd.begin();
  lcd.print("Project");
  lcd.setCursor(0, 1);
  lcd.print("Mechanical Arm");
  delay(3000);
  PRINT();

 
  //set solinoid
  pinMode(soli_1, OUTPUT);
  pinMode(soli_2, OUTPUT);
  pinMode(conver, OUTPUT);
  
  Serial.println("end setup");
}

void loop()
{ 
  if (mode != 1)//ถ้ายังไม่กดสตาร์ท ป้อนคีย์แพด
  {
    KEYPAD();
  }
  if (error == 1) //skip loop if no controller found
  {
    return;
  }

  if (type == 2)
  { //Guitar Hero Controller
    Serial.println("Guitar");
  }
  else
  { //DualShock Controller Button/ButtonPressed/ButtonReleased
    ps2x.read_gamepad(false, vibrate);
    if (ps2x.ButtonPressed(PSB_START)&& mode ==3) //กดสตาร์ท
    {
      Serial.println("Ready");
      mode = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ready");
      delay(1000);
      PRINT();
      
      ///////////START SYSTEM///////////////////
      
      while (CHECK_END() ==0) //ถ้าของยังไม่ครบ
      {
        SOLENOID(); //ปล่อย 1 ชิ้น
        
        CONVEYOR(1); //เดินสายพาน
        
        while(CHECK_IR() ==0)//เช็ควัตถุผ่าน
        {
          Serial.println("Not object");
        }
        delay(1500);//เวลาก่อนหยุดสายพาน
        Serial.println("GG");
        
        CONVEYOR(0); //เจอวัตถุหยุดสายพาน
        delay(1000);
        
        int type = CHECK_TYPE();//เช็คชนิด
        if (type ==1 && countB != MAXB) //โลหะ กล่องยังไม่เต็ม
        {
          METAL();//คีบและวาง
          countB= countB+1;
          PRINT();
        }
        else if (type == 2 && countC != MAXC)//อโลหะ กล่องยังไม่เต็ม
        {
          NON_METAL();//คีบและวาง
          countC= countC+1;
          PRINT();
        }
        else//กล่องเต็มสำหรับวุตถุนั้นแล้ว เดินสายพานไปที่อื่น
        {
          
          CONVEYOR(1);
          delay(3000);
          CONVEYOR(0);
        }
      }
      //ครบแล้วรอรับคำสั่ง
      PRINT();
      mode = 2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stop");
      delay(2000);
      mode = 3;
      countB =0;
      countC = 0;
      PRINT();
    }
    
    /*
    ps2x.read_gamepad(false, vibrate);
    if (ps2x.ButtonPressed(PSB_SELECT))
    {
      Serial.println("Stop");
      mode = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stop");
      delay(1000);
      PRINT();
    }
    delay(300);*/
    
   }
  
  delay(50);
}

void smooth_1(int degree)
{
  Serial.println("FN_1");
  if (degree > degree_servo_1)
  {
    for (degree_servo_1; degree_servo_1 < degree; degree_servo_1++)
    {
      servo_1.write(degree_servo_1);
      delay(delay_smooth);
    }
  }
  else if (degree < degree_servo_1)
  {
    for (degree_servo_1; degree_servo_1 > degree; degree_servo_1--)
    {
      servo_1.write(degree_servo_1);
      delay(delay_smooth);
    }
  }
}

void smooth_2(int degree)
{
  Serial.println("FN_2");
  if (degree > degree_servo_2)
  {
    for (degree_servo_2; degree_servo_2 < degree; degree_servo_2++)
    {
      //Serial.println(degree_servo_1);
      servo_2.write(degree_servo_2);
      delay(delay_smooth);
    }
  }
  else if (degree < degree_servo_2)
  {
    for (degree_servo_2; degree_servo_2 > degree; degree_servo_2--)
    {
      //Serial.println(degree_servo_1);
      servo_2.write(degree_servo_2);
      delay(delay_smooth);
    }
  }
}

void smooth_3(int degree)
{
  Serial.println("FN_3");
  if (degree > degree_servo_3)
  {
    for (degree_servo_3; degree_servo_3 < degree; degree_servo_3++)
    {
      //Serial.println(degree_servo_1);
      servo_3.write(degree_servo_3);
      delay(delay_smooth);
    }
  }
  else if (degree < degree_servo_3)
  {
    for (degree_servo_3; degree_servo_3 > degree; degree_servo_3--)
    {
      //Serial.println(degree_servo_1);
      servo_3.write(degree_servo_3);
      delay(delay_smooth);
    }
  }
}

void smooth_4(int degree)
{
  Serial.println("FN_4");
  if (degree > degree_servo_4)
  {
    for (degree_servo_4; degree_servo_4 < degree; degree_servo_4++)
    {
      //Serial.println(degree_servo_1);
      servo_4.write(degree_servo_4);
      delay(delay_smooth);
    }
  }
  else if (degree < degree_servo_4)
  {
    for (degree_servo_4; degree_servo_4 > degree; degree_servo_4--)
    {
      //Serial.println(degree_servo_1);
      servo_4.write(degree_servo_4);
      delay(delay_smooth);
    }
  }
}

void smooth_56(int degree)
{
  Serial.println("FN_56");
  if (degree > degree_servo_5)
  {
    for (degree_servo_5; degree_servo_5 < degree; degree_servo_5++)
    {
      //Serial.println(degree_servo_1);
      servo_5.write(degree_servo_5);
      delay(delay_smooth);
    }
  }
  else if (degree < degree_servo_5)
  {
    for (degree_servo_5; degree_servo_5 > degree; degree_servo_5--)
    {
      //Serial.println(degree_servo_1);
      servo_5.write(degree_servo_5);
      delay(delay_smooth);
    }
  }
}

void smooth_6(int degree)
{
  Serial.println("FN_6");
  if (degree > degree_servo_6)
  {
    for (degree_servo_6; degree_servo_6 < degree; degree_servo_6++)
    {
      //Serial.println(degree_servo_1);
      servo_6.write(degree_servo_6);
      delay(delay_smooth);
    }

  }

  else if (degree < degree_servo_6)
  {
    for (degree_servo_6; degree_servo_6 > degree; degree_servo_6--)
    {
      //Serial.println(degree_servo_1);
      servo_6.write(degree_servo_6);
      delay(delay_smooth);
    }
  }
}

void smooth_7(int degree)
{
  Serial.println("FN_7");
  if (degree > degree_servo_7)
  {
    for (degree_servo_7; degree_servo_7 < degree; degree_servo_7++)
    {
      //Serial.println(degree_servo_1);
      servo_7.write(degree_servo_7);
      delay(delay_smooth);
    }

  }

  else if (degree < degree_servo_7)
  {
    for (degree_servo_7; degree_servo_7 > degree; degree_servo_7--)
    {
      //Serial.println(degree_servo_1);
      servo_7.write(degree_servo_7);
      delay(delay_smooth);
    }
  }
}


void take()
{
  Serial.println("FN_TAKE");
  smooth_1(180);
  degree_servo_1 = 180;

  Serial.println(degree_servo_1);
  //servo_1.write(degree_servo_1);
  delay(DELAY_1);
}

void drop()
{
  Serial.println("FN_DROP");
  smooth_1(min_degree_servo_1);
  degree_servo_1 = min_degree_servo_1;
  Serial.println(degree_servo_1);
  // servo_1.write(degree_servo_1);
  delay(DELAY_1);
}

void UnderReTake()
{
  Serial.println("FN_UNDER_RE_TAKE");
  smooth_3(30);
  degree_servo_3 = 30;
  delay(10);
  smooth_4(20);
  degree_servo_4 = 20;
  delay(200);
  smooth_56(165);
  degree_servo_5 = 165;
  delay(10);
}

void UnderTake()
{
  Serial.println("FN_UNDER_TAKE");
  smooth_3(150);
  degree_servo_3 = 150;
  delay(10);
  smooth_4(170);
  degree_servo_4 = 170;
  delay(200);
  smooth_56(15);
  degree_servo_5 = 15;
  delay(10);
}

void UnderReDrop()
{
  Serial.println("FN_UNDER_RE_DROP");
  smooth_3(30);
  degree_servo_3 = 30;
  delay(10);
  smooth_4(20);
  degree_servo_4 = 20;
  delay(200);
  smooth_56(150);
  degree_servo_5 = 150;
  delay(10);
}

void UnderDrop()
{
  Serial.println("FN_UNDER_DROP");
  smooth_3(150);
  degree_servo_3 = 150;
  delay(10);
  smooth_4(170);
  degree_servo_4 = 170;
  delay(200);
  smooth_56(30);
  degree_servo_5 = 30;
  delay(10);
}
void Ready()
{
  ("FN_READY");
  smooth_56(90);
  degree_servo_5 = 90;
  smooth_2(120);
  degree_servo_2 = 120;
  smooth_3(90);
  degree_servo_3 = 90;
  smooth_4(90);
  degree_servo_4 = 90;
}

void METAL()
{
  Serial.println("FN_METAL");
  if (A > 180)
  {
    smooth_7(A - 180);
    degree_servo_7 = A - 180;
    drop();
    UnderReTake();
    take();
    Ready();
  }
  else
  {
    smooth_7(A);
    degree_servo_7 = A;
    drop();
    UnderTake();
    take();
    Ready();
  }
  if (B > 180)
  { smooth_7(B - 180);
    degree_servo_7 = B - 180;
    UnderReDrop();
    drop();
    Ready();
  }
  else
  {
    smooth_7(B);
    degree_servo_7 = B;

    UnderDrop();
    drop();
    Ready();
  }
}


void NON_METAL()
{
  Serial.println("FN_NON_METAL");
  if (A > 180)
  {
    smooth_7(A - 180);
    degree_servo_7 = A - 180;
    drop();
    UnderReTake();
    take();
    Ready();
  }
  else
  {
    smooth_7(A);
    degree_servo_7 = A;
    drop();
    UnderTake();
    take();
    Ready();
  }

  if (C > 180)
  {
    smooth_7(C - 180);
    degree_servo_7 = C - 180;
    UnderReDrop();
    drop();
    Ready();
  }
  else
  {
    smooth_7(C);
    degree_servo_7 = C;
    UnderDrop();
    drop();
    Ready();
  }
}


int CHECK_TYPE()//เช็คชนิดวัตถุ
{
  Serial.println("FN_CHECK_TYPE");
  monitoring = analogRead(metalDetection);

  if (monitoring == 0)
  {
    Serial.println("Metal");
    return 1;
  }
  else if (monitoring > 250)
  {
    Serial.println("Non-Metal");
    return 2;
  }
}

int CHECK_IR()//เช็ควัตถุผ่าน
{
  //Serial.println("FN_CHECK_IR");
  IR = analogRead(IRDetection);
  if (IR == 0)
  {
    Serial.println("HIT");
    return 1;
  }
  else
  {
    return 0;
  }
}

int CHECK_END()//เช็คของครบ
{
  if (countB == MAXB && countC == MAXC  )
  {
    return 1; //ครบ
  }
  else
  {
    return 0;
  }
}

void PRINT() //ปริ้น LCD
{
  //Serial.println("FN_PRINT");
  lcd.clear();
  //ROW 1
  lcd.setCursor(0, 0);
  lcd.print(String("A:") + String(A));
  lcd.setCursor(8, 0);
  lcd.print(String("B:") + String(B));
  lcd.setCursor(15, 0);
  lcd.print(String("C:") + String(C));
  //ROW 2
  lcd.setCursor(0, 1);
  lcd.print(String("1:") + String(degree_servo_1));
  lcd.setCursor(8, 1);
  lcd.print(String("2:") + String(degree_servo_2));
  //ROW 3
  lcd.setCursor(0, 2);
  lcd.print(String("3:") + String(degree_servo_3));
  lcd.setCursor(8, 2);
  lcd.print(String("4:") + String(degree_servo_4));
  lcd.setCursor(13, 2);
  lcd.print(String("M:") + String(countB)+ String("/") + String(MAXB));
  //ROW 4
  lcd.setCursor(0, 3);
  lcd.print(String("5/6:") + String(degree_servo_5));
  lcd.setCursor(8, 3);
  lcd.print(String("7:") + String(degree_servo_7));
  lcd.setCursor(13, 3);
  lcd.print(String("N:") + String(countC)+ String("/") + String(MAXC));

  delay(20);
}

void KEYPAD()//เซตค่า keypad
{
  Serial.println("FN_KEYPAD");
  if ( (key = keypad.getKey()) != NO_KEY)//put
  { 
    if (key != '#')
    {
    Serial.println( key ); // show the key pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(key);
    delay(500);
    PRINT();
    }
    if (key == 'A')
    {
      Set = 1;
      Value = '0';
    }
    else if (key == 'B')
    {
      Set = 2;
      Value = '0';
    }
    else if (key == 'C')
    {
      Set = 3;
      Value = '0';
    }
    else if (key == 'D')
    {
      Set = 5;
      Value = '0';
    }
    else if (key == '*')
    {
      Set = 4;
      Value = '0';
    }
    else if (key == '#')
    {
      if (Set == 1)
      {
        if ( Value.toInt() >= 0 && Value.toInt() <= 360)
        {
          A = Value.toInt();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set A");
          delay(1000);
          PRINT();
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Plese set 0-360");
          delay(1000);
          PRINT();
        }
      }
      else if (Set == 2)
      {
        if ( Value.toInt() >= 0 && Value.toInt() <= 360)
        {
          B = Value.toInt();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set B");
          delay(1000);
          PRINT();
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Plese set 0-360");
          delay(1000);
          PRINT();
        }
      }
      else if (Set == 3)
      {
        if ( Value.toInt() >= 0 && Value.toInt() <= 360)
        {
          C = Value.toInt();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set C");
          delay(1000);
          PRINT();
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Plese set 0-360");
          delay(1000);
          PRINT();
        }
      }
      else if (Set == 4)
      {
        if (Value.toInt() > 0 )
        {
          MAXB = Value.toInt();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set Max Metal");
          delay(1000);
          PRINT();
        }
      }
      else if (Set == 5)
      {
        if (Value.toInt() > 0 )
        {
          MAXC = Value.toInt();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set Max Non-Metal");
          delay(1000);
          PRINT();
        }
      }
    }
    else
    {
      Value = Value + String(key);
    }

  }
  
  delay(20);
}
void SOLENOID()//ปล่อยของหนึ่งชิ้น
{
  Serial.println("FN_SOLENOID");
  digitalWrite(soli_1,HIGH);  //ปล่อยล่าง 
  delay(1000);                //รอ
  digitalWrite(soli_1,LOW);   //ปิดล่าง
  delay(2000);                //รอ
  digitalWrite(soli_2,HIGH);  //ปล่อยบน
  delay(120);                 //รอ
  digitalWrite(soli_2,LOW);   //ปิดบน
}

void CONVEYOR(int RUN)
{
  Serial.println("FN_CONVEYOR");
  if (RUN == 1) //เดินสายพาน
  {
    digitalWrite(conver,HIGH);
    Serial.println("Run_con");
  }
  else if (RUN == 0)//หยดสายพาน
  {
    digitalWrite(conver,LOW);
    Serial.println("stop_con");
  }
}
