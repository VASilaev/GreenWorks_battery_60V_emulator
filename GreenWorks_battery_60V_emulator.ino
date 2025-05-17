#define IO_PIN 17
#define LED_PIN 13

//Выберите свой аккумулятор, должна быть расскоментирована только одна строка

//82V
//const uint32_t[] SendDataList = {0xFFFFFF, 0xE6EBFA, 0xA9AFF9, 0xB7C3F3, 0xE1E8F8};
//const uint32_t[] SendDataList = {0xFFFFFF, 0xE6EBFA, 0xD1D7F9, 0xF8FFF8};
//const uint32_t[] SendDataList = {0xFFFFFF, 0xE6EBFA, 0xD1D7F9, 0чF8FFF8};

//60V
const uint32_t[] SendDataList = {0xFFFFFF, 0xEBF0FA, 0xF8FEF9, 0xD0DCF3, 0xDDE4F8};
//40V
//const uint32_t[] SendDataList = {0xFFFFFF, 0xF0F5FA, 0xF7FDF9, 0xC6D2F3, 0xE1E8F8};

//24V
//const uint32_t[] SendDataList = {0xFFFFFF, 0xF4F9FA, 0xF8FEF9, 0xF8FFF8};


 
// служебные макросы
//Быстрый доступ к портам ввода/вывода - работает только с константами
#define digitalWriteC(pin,val)\
 if (val) { *((volatile uint8_t *) port_to_output_PGM[digital_pin_to_port_PGM[pin]]) |= (digital_pin_to_bit_mask_PGM[pin]);}\
 else {*((volatile uint8_t *) port_to_output_PGM[digital_pin_to_port_PGM[pin]]) &= ~(digital_pin_to_bit_mask_PGM[pin]);}

#define digitalToggleC(pin)\
  port_to_input_PGM[digital_pin_to_port_PGM[pin]]) = (digital_pin_to_bit_mask_PGM[pin])

#define pinModeC(pin,mode)\
  if (mode == INPUT) { \
    *((volatile uint8_t *) port_to_mode_PGM[digital_pin_to_port_PGM[pin]]) &= ~(digital_pin_to_bit_mask_PGM[pin]);\
    *((volatile uint8_t *) port_to_output_PGM[digital_pin_to_port_PGM[pin]]) &= ~(digital_pin_to_bit_mask_PGM[pin]);\
  } else if (mode == INPUT_PULLUP) {\
    *((volatile uint8_t *) port_to_mode_PGM[digital_pin_to_port_PGM[pin]]) &= ~(digital_pin_to_bit_mask_PGM[pin]);\
    *((volatile uint8_t *) port_to_output_PGM[digital_pin_to_port_PGM[pin]]) |= (digital_pin_to_bit_mask_PGM[pin]);\
  } else {\
    *((volatile uint8_t *) port_to_mode_PGM[digital_pin_to_port_PGM[pin]]) |= (digital_pin_to_bit_mask_PGM[pin]);\
  };

inline uint8_t digitalReadC (uint8_t pin) __attribute__((always_inline));
uint8_t digitalReadC (uint8_t pin)
 {
   if (*((volatile uint8_t *) port_to_input_PGM[digital_pin_to_port_PGM[pin]]) & (digital_pin_to_bit_mask_PGM[pin])) {return HIGH;} else {return LOW;};  
 };

#define IO_START_TX pinModeC(IO_PIN, OUTPUT)

#define IO_OUTPUT_LOW  digitalWriteC(IO_PIN, LOW)

#define IO_OUTPUT_HIGH digitalWriteC(IO_PIN, HIGH)

#define IO_START_RX pinModeC(IO_PIN, INPUT_PULLUP)

#define IO_READ (digitalReadC(IO_PIN))

#define LED_ON digitalWriteC(LED_PIN, HIGH)

#define LED_OFF digitalWriteC(LED_PIN, LOW)


#if defined(__LGT8FX8P__)
  #define LED_TOGGLE digitalToggleC(LED_PIN)
#else
  #define LED_TOGGLE if(digitalReadC(LED_PIN)) {digitalWriteC(LED_PIN, LOW);} else {digitalWriteC(LED_PIN, HIGH);}
#endif

#if defined(TCNT2) 
 #define LSYSTYCK TCNT2
#elif defined(TCNT2L) 
 #define LSYSTYCK TCNT2L
#else
 #error TIMER 2 not defined
#endif

#define usToTick(val) (val / 4)

uint8_t state = 0;
uint16_t timeout = 0;
uint16_t timeoutCMD = 0;
uint8_t tryCount;
uint32_t code;
uint8_t prevT;

#include <SoftwareSerial.h>
  //SoftwareSerial mySerial (4, 5);

void setup() {

  IO_START_RX;
  pinModeC(LED_PIN, OUTPUT);
  state = 0;
  prevT = LSYSTYCK;

  //mySerial.begin(19200);
}


 uint8_t prevState;

#define TimeOut(val) while((uint8_t)((uint8_t) LSYSTYCK - (uint8_t) prevT_local) < (uint8_t) usToTick(val)) {}; prevT_local = LSYSTYCK;


void debug(uint16_t bufer2) {
  //mySerial.print(bufer2, HEX);
}

const uint32_t[] TimeOutList = {90000, 90000, 90000, 90000, 180000};



uint8_t current_msg;


void loop () {

  //Уменьшаем таймаут  
  do {
    uint8_t delta = LSYSTYCK - prevT;  
    prevT += delta;
    if (timeout && delta) {
      if (timeout > delta) timeout-=delta; else timeout = 0;
    }
  } while(false);

  if (state != prevState) {
    prevState = state;
    debug(state);
  };

  //ОСновная машина состояний

  
  switch (state) {
    case 0: //На линии низкий уровень, ждем высокий
      if (IO_READ) {

        current_msg = 0; 
        code = SendDataList[current_msg];
        timeout = TimeOutList[current_msg]
        tryCount = 4;     
        state = 2;
      }; 
      if (!timeout) {
         timeout = usToTick(250000);
         LED_TOGGLE;
      }
      break;  
    case 2:
      //Высокий уровень
      if (!IO_READ) {
        timeout = usToTick(100);
        state = 3;        
      } else if (!timeout) {     
        state=4;
      };
      break;
    case 3:
      //Неожиданный низкий уровень перед командой 
      if (IO_READ) { 
        timeout = TimeOutList[current_msg]
        state=2;        
      } else if (!timeout) {     
        state=0;
      };
      break;

    case 4:
      LED_ON;
      noInterrupts();
      IO_START_TX;
      IO_OUTPUT_LOW; 
  
      uint8_t prevT_local = LSYSTYCK;

      TimeOut(400);
      IO_OUTPUT_HIGH; 
  
      do {
        uint32_t bufer = code;
        for (uint8_t i = 24;i!=0;i--) {
          TimeOut(100);    
          IO_OUTPUT_LOW;
          
          if ((bufer & 0x01)) {
            TimeOut(100);    
          } else {
            TimeOut(200);    
          }
          bufer = bufer >> 1; 
          IO_OUTPUT_HIGH; 
        }
      } while (false);

      TimeOut(100);  

      //Считываем ответ от инструмента  
      IO_START_RX;        
      do {
        uint16_t timeout_local = usToTick(1250);
        prevT_local = LSYSTYCK;

        while (timeout_local) {
          if (!IO_READ) {
            //начало передачи 
            uint8_t current;
            uint8_t endCondition;
            uint8_t delta;
            uint16_t bufer = 0x8000;  
            
            do {
              delta = 0;
              while (!IO_READ && (delta < (uint8_t)usToTick(300))) {delta = (current = LSYSTYCK) - prevT_local;};
              prevT_local = current;
              
              endCondition = bufer & 0x01;
              bufer = bufer >> 1;
    
              if ((delta < usToTick(60)) || (delta >= (uint8_t) usToTick(300))) {
                bufer = 0xFFFF;
                break;
              } else {if (delta < usToTick(150)) {
                bufer = bufer | 0x8000;
              };};
              
              delta = 0;              
              while (IO_READ && (delta < (uint8_t) usToTick(300))) {delta = (current = LSYSTYCK) - prevT_local;};
              prevT_local = current;
              
              if ((delta <  usToTick(60))) {
                bufer = 0xFFFF;
                break;                
              } else if ((delta >= (uint8_t) usToTick(300)) && !endCondition  ) {
                bufer = 0xFFFF;
                break;
              };
            } while (!endCondition);
  
            interrupts();

            if (bufer == 0x2DD2) {

              if (current_msg < (sizeof(SendDataList) / sizeof(SendDataList[0]))) current_msg ++; 

              code = SendDataList[current_msg];
              timeout = TimeOutList[current_msg]
              state = 2;

              timeout_local = 1;
              LED_OFF; //Погасим в случае успеха
            } else {
              timeout_local = 0;
            };
            break;
          };
   
          do { //Обновление timeout_local
            uint8_t delta = LSYSTYCK - prevT_local;  
            prevT_local += delta;
            if (timeout_local && delta) {
              if (timeout_local > delta) timeout_local-=delta; else timeout_local = 0;
            }
          } while(false);
  
        };

        interrupts();

        //Если ответ не получили вовремя повторить, если кончились попытки повтора все начинаем снова
        if (!timeout_local) {
          if (tryCount) { 
            state = 2;
          } else {
            state = 0;              
          };
        }       
      } while(false);

      prevT = LSYSTYCK; 
      break; 
  };
};
