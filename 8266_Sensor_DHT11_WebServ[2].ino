//This example for use ESP8266 with Adafruit DHT11 library: 
//https://github.com/adafruit/DHT-sensor-library
#include "DHT.h"
// connect data pin of DHT11 to D2 ESP8266 NodeMCU
#define DHTPIN 2   
#define RT_data0 0 
//#define TXD 1 // GPIO1/TXD01 
//#define RXD 3 // GPIO1/TXD01

// тут используется иной вебсервер

// wifi library 
#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Thread.h> 

MDNSResponder mdns;
WiFiServer  server(80);
String webPage = "";  
String header;
// Auxiliar variables to store the current output state
String output1State = "on";
String outputTXDState = "off";

// массив пока будет для 4 строк получается двух данных
String dataControl[4];
int dataVal[4]; // массив под данные
String strGet = "GET /?"; // заготовка для Get запроса
String strHTTP = " HTTP/1.1"; // тоже нужно удалить
String humidity = "humidity"; // идентификатор влажности
String delayFanStr = "delayOffFan"; // идентификатор задержки
String nS = "\n"; // идентификатор разделителя строк
int humiditySet = 55; // установка вляжности по умолчанию
int delayOffFan = 30; // задержка выключения вентилятора
int checTimeOut = delayOffFan; // счетчик времени так как задержка срабатывания влажности 1 сек будет минута(что бы постоянно не включался и не выключался) 
boolean trigOffdelay = false; // тригер отключения после промежутка времени
boolean trigButton = true; // тригер отключения после промежутка времени
String tmpONOff = "";


// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

// потоки
Thread webServ = Thread();
Thread threadDHT = Thread();
Thread threadSerial = Thread();

// переменные датчиков
float h ;// get humidity 
float t ;// get temperature  
float testVal ;// get humidity 

void setup() {
  // настройка пинов
  //pinMode(TXD, OUTPUT);
  pinMode(RT_data0, OUTPUT);
   
  // initialize serial communication
  Serial.begin(9600);
  
  IPAddress ip(192, 168, 16, 1);
  IPAddress gateway(192, 168, 16, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.mode(WIFI_AP);
 
  boolean result = WiFi.softAP("ESPsoftAP_01", "12345678");
  if(result == true)
  {
    //Serial.println("Ready");
  }
  else
  {
    //Serial.println("Failed!");
  }

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    //Serial.println("mDNS responder started");
  } else {
    //Serial.println("Error setting up MDNS responder!");
  }

 dht.begin();
 server.begin();
  
  // Настройка потоков
  webServ.onRun(webServerF); // назначаем потоку задачу
  webServ.setInterval(10); // задаем интервал срабатывания потока, мсек
    
  threadDHT.onRun(readDHT); //назначаем потоку задачу
  threadDHT.setInterval(1000); //задаем интервал срабатывания потока, ммсек не меньше 2сек как так датчик не быстр

  threadSerial.onRun(sendSerial); //назначаем потоку задачу
  threadSerial.setInterval(1000); //задаем интервал срабатывания потока, ммсек не меньше 2сек как так датчик не быстр
 
}

void loop() {

  //проверим пришло ли время запуска web
    if (webServ.shouldRun())
      webServ.run(); // запускаем поток
      
  //проверим пришло ли время чтения датчика
    if (threadDHT.shouldRun())
      threadDHT.run(); // запускаем поток

 //проверим пришло ли время чтения датчика
    if (threadSerial.shouldRun())
      threadSerial.run(); // запускаем поток

}

void webServerF(){
   WiFiClient client = server.available();                // Получаем данные, посылаемые клиентом 
  
 if (client){
  //Serial.println("New client");                         // Отправка "Новый клиент"
  String currentLine = ""; // make a String to hold incoming data from the client
  boolean blank_line = true;                            // Создаем переменную, чтобы определить конец HTTP-запроса 
  while (client.connected()){                           // Пока есть соединение с клиентом 
    if (client.available()){                            // Если клиент активен 
     /*
     char c = client.read();                            // Считываем посылаемую информацию в переменную "с"
     if (c == '\n' && blank_line){                      // Вывод HTML страницы 
       //float t = dht.readTemperature();                  // Запрос на считывание температуры
       //float h = dht.readHumidity();
       client.println("HTTP/1.1 200 OK");               // Стандартный заголовок HTTP 
       client.println("Content-Type: text/html"); 
       client.println("Connection: close");             // Соединение будет закрыто после завершения ответа
       client.println("Refresh: 10");                   // Автоматическое обновление каждые 10 сек 
       client.println();

       // включение и отключение сигналов
       if (header.indexOf("GET /0/on") >= 0) {
          output1State = "on";
          digitalWrite(RT_data0, HIGH);
          h =0;
       } else if (header.indexOf("GET /0/off") >= 0) {
           output1State = "off";
           digitalWrite(RT_data0, LOW);
           h =0;
       } else if (header.indexOf("GET /2/on") >= 0) {
           outputTXDState = "on";
           digitalWrite(TXD, HIGH);
           h =0;
       } else if (header.indexOf("GET /2/off") >= 0) {
           outputTXDState = "off";
           digitalWrite(TXD, LOW);
           h =0;
         }
         
       client.println("<!DOCTYPE HTML>");               // Веб-страница создается с использованием HTML
       client.println("<html>");                        // Открытие тега HTML 
       client.println("<head>");
       client.print("<title>ESP8266 TEMP</title>");     // Название страницы
       client.println("</head>");
       client.println("<body>");
       client.println("<h1>ESP8266 - Temperature & Humidity</h1>"); 
       client.println("<h3>Temperature = ");
       client.println(t);                               // Отображение температуры
       client.println("*C</h3>");
       client.println("</head>");
       
       client.println("<body>");
       client.println("<h3>Humidity = ");
       client.println(h);                               // Отображение влажности
       client.println("</h3>");
       client.println("</body>");
       
       client.println("<a href=\"/2/on\"><button>ON</button></a>&nbsp; <a href=\"/2/off\"><button>OFF</button></a>");
       
       client.println("</html>");                       // Закрытие тега HTML 
       break;                                           // Выход
       }                                          
       
        if (c == '\n'){                                 // Если "с" равен символу новой строки                                             
         blank_line = true;                             // Тогда начинаем новую строку
        }                                          
         else if (c != '\r'){                           // Если "с" не равен символу возврата курсора на начало строки                                        
          blank_line = false;                           // Тогда получаем символ на текущей строке 
         }   
         */ 
           char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        
        if (c == '\n') {                    // if the byte is a newline character
          //Serial.println(header);  // странно при таком выводе подвисает
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          
          if (currentLine.length() == 0) {
          Serial.println(header);  // тут не виснет
          if (header.indexOf("GET /?") >= 0) {
              //Serial.println(header);  // тут не виснет (проверить что за нах)
              cutString(header); // проверям что пришло от пользователя.
              //readMassData();// читаем выборку данных что обработали от пользователя
          }
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            //client.println("Refresh: 10"); // Автоматическое обновление каждые 10 сек 
            client.println();
            
            client.println("<h3>Temperature = ");
            client.println(t);                               // Отображение температуры
            client.println("*C</h3>");
            client.println("</head>");
            //client.println("<body>");
            client.println("<h3>Humidity = ");
            client.println(h);                               // Отображение влажности
            client.println("<h3>  StepEnableHumidity = ");
            client.println(humiditySet);    
            client.println("</h3>");

            client.println("<h3>delayfan = ");
            client.println(delayOffFan);                               // Отображение задержки выключения вентилятора
            client.println("</h3>");
            
            // форма отправки данных
            client.println("<form action='http://192.168.16.1' method='get'>");
            client.println("<div>");
            client.println("<label for='" + delayFanStr +"'>delayOffSecond</label>");
            client.println("<input name='" + delayFanStr +"' id='" + delayFanStr +"' value='" + delayOffFan + "'>");
            client.println("</div>");
            client.println("<div>");
            client.println("<label for='humidity'>set step humidity</label>");
            // парсит не по id а по name
            client.println("<input name='" + humidity +"' id='" + humidity +"' value='" + humiditySet + "'>");
            client.println("</div>");
            client.println("<div><button>set config</button></div>");
            client.println("</form>");
            
            //для автоперехода по нажатию
            boolean equiv = false;
            // turns the GPIOs on and off
            if (header.indexOf("GET /0/on") >= 0) {
              output1State = "on";
              digitalWrite(RT_data0, HIGH);
              h = 20;
              equiv = true;
              trigButton = true;
            } else if (header.indexOf("GET /0/off") >= 0) {
              output1State = "off";
              digitalWrite(RT_data0, LOW);
              h =30;
              equiv = true;
              trigButton = false;
            } else if (header.indexOf("GET /1/on") >= 0) {
              outputTXDState = "on";
               //digitalWrite(TXD, HIGH);
               h =40;
               equiv = true;
            } else if (header.indexOf("GET /1/off") >= 0) {
              outputTXDState = "off";
              //digitalWrite(TXD, LOW);
              h =50;
              equiv = true;
            }
                        
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            if(equiv){
              client.println("<meta http-equiv=\"Refresh\" content=\"1; URL=http://192.168.16.1\">"); // автопереход на начало
              }
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>GPIO 0 - Fan State " + output1State + "</p>");
            
            // If the output5State is off, it displays the ON button       
            if (output1State=="off") {
              client.println("<p><a href=\"/0/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 1 (TXD led) - State " + outputTXDState + "</p>");
            // If the output4State is off, it displays the ON button       
            if (outputTXDState=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }                                    
    }
  }  
    header = "";
    client.stop(); 
 }                       
}

// чтение влажности и температуры а так же проверки новых данных в массиве
void readDHT(){
  h = dht.readHumidity();
  t = dht.readTemperature(); 
  readMassData();// читаем выборку данных что обработали от пользователя
  viewTH(); // логика включения и отключения вентилятора

}

// Фукция парсера запроса от пользователя
void cutString(String enStr)
{ 
  int linArray = 0; // идентификатор длинны массива возвращаемых значений.
  
  boolean nf = true;
  int f =0; // сколько найдено
  String fStr;
  String findingD;
  boolean summMass = true; // триггер выхода из цыкла по внесению данных
  boolean AddDataMas = false; // триггер внесению данных в массив
  
  // так создаем динамический массив
  //String *retMass = new String[0];
 
  do { // для просчета сколько должен быть массив 
    boolean trigAnalData = true;
    while(nf){
      int indexValF =enStr.indexOf(nS, f);
      if(indexValF >= 0){
        //Serial.println(String(indexValF));
        fStr = enStr.substring(f, indexValF); // режим от индекса до первого наеденного и длинны по чему ищем
        f = indexValF + nS.length(); // поиск от новой позиции
      }else {
        fStr = enStr.substring(f, indexValF); 
        nf = false;
      }
     
      // теперь ищем подстроку в подстроке GET запрос
        int delG = fStr.indexOf(strGet); // это начало поиска от куда совпадения может тоже поможет
        if(delG >= 0){
          String fG = fStr.substring(delG + strGet.length(), fStr.length());  // и вырезаем Get оставля только данные
          //Serial.println(fG);
          int delH = fG.indexOf(strHTTP);
          fG = fG.substring(0, delH); // вырезаем HTTP из конца
          Serial.println("finag_str " + fG);
          
          // анализ самих данный
          int fD =0; // индекс расположения и вхождения данных
          while(trigAnalData){
            int indexValData =fG.indexOf("&", fD); // разделитель данных &
            if(indexValData >= 0){
              //Serial.println("IdexD " + String(indexValData));
              findingD = fG.substring(fD, indexValData); // режим от индекса до первого наеденного и длинны по чему ищем
              fD = indexValData + 1; // поиск от новой позиции +1 так как один символ разделитель
              }else {
                findingD = fG.substring(fD, indexValData); 
                trigAnalData = false;
              }
            
              // разбиваем данные на идентификаторы и значения
              String id = findingD.substring(0, findingD.indexOf("="));
              String val = findingD.substring(findingD.indexOf("=") + 1, findingD.length());
              
              if(!AddDataMas){
                Serial.print("id -> " + id );
                Serial.println(" | val -> " + val );
                                
                if(sizeof(dataControl)/sizeof(String) >= linArray){  // проверка на переполнение если данных больше
                  dataControl[linArray] = id; //в этот массив id
                  // тут будет определение идентификаторов примем только два значения пока
                  if (id.equals(humidity)){ // находим идентификатор влажности
                    int tmp = val.toInt();
                    dataVal[linArray] = tmp;  //в этот массив атрибуты
                    }
                  if (id.equals(delayFanStr)){ // находим идентификатор задержки
                    int tmp = val.toInt();
                    dataVal[linArray] = tmp;  //в этот массив атрибуты
                    }
                  linArray++;
                }
                //linArray = linArray + 2; // длинна на два так как ключ значение обязательно
              }else{
                // Сюда не входим совсем на втором круге(похер на этот вход)
                  Serial.println("inter else !!!");
                  if(sizeof(dataControl)/sizeof(String) <= linArray){  // проверка на переполнение если данных больше
                    dataControl[linArray] = id; // не заносит
                    linArray++;
                    dataControl[linArray] = val;
                    linArray++;
                  }
                }
          }
        }
        //Serial.println(fStr);
      } // while закончился
       
      // доп проверка если нечего не получили
      if(linArray == 0){
        summMass = false;
      }
      if(AddDataMas){ // если сработал триггер по внесению данных значит они занесены и выходим
        summMass = false;
      }else{
        trigAnalData = true; // взводим триггер для второго прохода
        nf = true;
        linArray =0;
        AddDataMas = true;// при первом проходе можно данные заносить
      }
  }
  while (summMass);
}

// Читаем данные из памяти ( для проверки )
void readMassData(){
  //Serial.println(sizeof(dataControl)/sizeof(String));
  for (int i=0; i< sizeof(dataControl)/sizeof(String); i++){
    // если тут находим значение влажности то по такомуже адресу но в другом массива
   // Serial.println(dataVal[i]);
    if (dataControl[i].equals(humidity)){ 
      humiditySet = dataVal[i];
    }
    if (dataControl[i].equals(delayFanStr)){ 
      delayOffFan = dataVal[i];
    }
  }  
  }

  // фукция слежения влажности и температуры и включения и выключения сигнала
  void viewTH(){  
    // если нажата кнопка выключения то он вообще сюда не заходит
    if(trigButton){  
      if (h >= humiditySet & trigOffdelay == false){// установили таймаут на превышение влажности
        checTimeOut = 0; 
        trigOffdelay = true;
      }
    
        if ( trigOffdelay | checTimeOut < delayOffFan){
          digitalWrite(RT_data0, HIGH); 
          tmpONOff = "Enable_Light";
          }else{
            digitalWrite(RT_data0, LOW);
            tmpONOff = "OFF_Light";
          }
     
        if (h <= humiditySet){// сбросить триггер
          trigOffdelay = false;
          checTimeOut++; // и только когда влажность упала начали отчет
        }
        // обнуление счетчика
        if(checTimeOut > 60) checTimeOut = delayOffFan; // что бы не входило в условие выше<= delayOffFan( нужно обдумать )
      }else trigOffdelay = false; // перестраховка что бы не был включен счетчик
    }

    void sendSerial(){
      Serial.print("h | ");
      Serial.println(h);
      Serial.print("humiditySet | ");
      Serial.println(humiditySet);
      Serial.print("checTimeOut | ");
      Serial.println(checTimeOut);
      Serial.print("trigOffdelay | ");
      Serial.println(trigOffdelay);
      Serial.print("tmpONOff | ");
      Serial.println(tmpONOff);
      Serial.print("delayOffFan | ");
      Serial.println(delayOffFan);
      
   }
