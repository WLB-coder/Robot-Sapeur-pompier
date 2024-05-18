#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_system.h"

// Remplacez par vos informations d'identification réseau
const char* ssid = "RobotAP";
const char* password = "AP_Password";

// Définissez l'adresse IP fixe de votre choix pour le point d'accès
IPAddress apIP(192, 168, 1, 10);
IPAddress subnet(255, 255, 255, 0);

// Épinglez pour le moteur
#define MOTOR_1_PIN_1 14
#define MOTOR_1_PIN_2 15
#define MOTOR_2_PIN_1 13
#define MOTOR_2_PIN_2 12

// Créez un objet serveur web asynchrone
AsyncWebServer server(80);

const char* index_html = 
  "<html>"
  "  <head>"
  "    <title>ESP32-CAM Robot</title>"
  "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "    <style>"
  "      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}"
  "      table { margin-left: auto; margin-right: auto; }"
  "      td { padding: 8 px; }"
  "      .button {"
  "        background-color: #2f4468;"
  "        border: none;"
  "        color: white;"
  "        padding: 10px 20px;"
  "        text-align: center;"
  "        text-decoration: none;"
  "        display: inline-block;"
  "        font-size: 18px;"
  "        margin: 6px 3px;"
  "        cursor: pointer;"
  "        -webkit-touch-callout: none;"
  "        -webkit-user-select: none;"
  "        -khtml-user-select: none;"
  "        -moz-user-select: none;"
  "        -ms-user-select: none;"
  "        user-select: none;"
  "        -webkit-tap-highlight-color: rgba(0,0,0,0);"
  "      }"
  "      img {  width: auto ;"
  "        max-width: 100% ;"
  "        height: auto ; "
  "      }"
  "    </style>"
  "  </head>"
  "  <body>"
  "    <h1>ESP32-CAM Robot</h1>"
  "    <img src=\"\" id=\"photo\" >"
  "    <table>"
  "      <tr><td colspan=\"3\" align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('forward');\" ontouchstart=\"toggleCheckbox('forward');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Forward</button></td></tr>"
  "      <tr><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('left');\" ontouchstart=\"toggleCheckbox('left');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Left</button></td><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('stop');\" ontouchstart=\"toggleCheckbox('stop');\">Stop</button></td><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('right');\" ontouchstart=\"toggleCheckbox('right');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Right</button></td></tr>"
  "      <tr><td colspan=\"3\" align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('backward');\" ontouchstart=\"toggleCheckbox('backward');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Backward</button></td></tr>"                   
  "    </table>"
  "   <script>"
  "   function toggleCheckbox(x) {"
  "     var xhr = new XMLHttpRequest();"
  "     xhr.open(\"GET\", \"/action?go=\" + x, true);"
  "     xhr.send();"
  "   }"
  "   window.onload = document.getElementById(\"photo\").src = window.location.href.slice(0, -1) + \":81/stream\";"
  "  </script>"
  "  </body>"
  "</html>";

void setup() {
  // Désactivez le détecteur de coupure de courant
  WRITE_PERI_REG(CONFIG_BROWNOUT_DET, 0);

  // Configurez les broches pour les moteurs
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);

  // Configurez la caméra (à l'aide de votre configuration)
camera_config_t config;
config.ledc_channel = LEDC_CHANNEL_0;
config.ledc_timer = LEDC_TIMER_0;
config.pin_d0 = 5;    // GPIO5 (Notez le changement ici)
config.pin_d1 = 18;   // GPIO18 (Notez le changement ici)
config.pin_d2 = 19;   // GPIO19 (Notez le changement ici)
config.pin_d3 = 21;   // GPIO21 (Notez le changement ici)
config.pin_d4 = 36;   // GPIO36 (Notez le changement ici)
config.pin_d5 = 39;   // GPIO39 (Notez le changement ici)
config.pin_d6 = 34;   // GPIO34 (Notez le changement ici)
config.pin_d7 = 35;   // GPIO35 (Notez le changement ici)
config.pin_xclk = 0;  // GPIO0 (Notez le changement ici)
config.pin_pclk = 22; // GPIO22 (Notez le changement ici)
config.pin_vsync = 25; // GPIO25 (Notez le changement ici)
config.pin_href = 23;  // GPIO23 (Notez le changement ici)
config.pin_sscb_sda = 26;  // GPIO26 (Notez le changement ici)
config.pin_sscb_scl = 27;  // GPIO27 (Notez le changement ici)
config.pin_pwdn = 32;     // GPIO32 (Notez le changement ici)
config.pin_reset = 15;    // GPIO15 (Notez le changement ici)
config.xclk_freq_hz = 20000000;
config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialisez la caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Commencez le point d'accès WiFi avec une adresse IP fixe
  WiFi.softAPConfig(apIP, apIP, subnet);
  WiFi.softAP("RobotAP", "AP_Password");

  Serial.println("WiFi AP mode activated");
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.softAPIP());

  // Servez une page HTML à la racine ("/")
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

  // Gérez les actions via des requêtes AJAX
  server.on("/action", HTTP_GET, [](AsyncWebServerRequest *request){
    String param = request->arg("go");
    if (param == "forward") {
      // Gérer l'action "forward"
      digitalWrite(MOTOR_1_PIN_1, 1);
      digitalWrite(MOTOR_1_PIN_2, 0);
      digitalWrite(MOTOR_2_PIN_1, 1);
      digitalWrite(MOTOR_2_PIN_2, 0);
    } else if (param == "left") {
      // Gérer l'action "left"
      digitalWrite(MOTOR_1_PIN_1, 0);
      digitalWrite(MOTOR_1_PIN_2, 1);
      digitalWrite(MOTOR_2_PIN_1, 1);
      digitalWrite(MOTOR_2_PIN_2, 0);
    } else if (param == "right") {
      // Gérer l'action "right"
      digitalWrite(MOTOR_1_PIN_1, 1);
      digitalWrite(MOTOR_1_PIN_2, 0);
      digitalWrite(MOTOR_2_PIN_1, 0);
      digitalWrite(MOTOR_2_PIN_2, 1);
    } else if (param == "backward") {
      // Gérer l'action "backward"
      digitalWrite(MOTOR_1_PIN_1, 0);
      digitalWrite(MOTOR_1_PIN_2, 1);
      digitalWrite(MOTOR_2_PIN_1, 0);
      digitalWrite(MOTOR_2_PIN_2, 1);
    } else if (param == "stop") {
      // Gérer l'action "stop"
      digitalWrite(MOTOR_1_PIN_1, 0);
      digitalWrite(MOTOR_1_PIN_2, 0);
      digitalWrite(MOTOR_2_PIN_1, 0);
      digitalWrite(MOTOR_2_PIN_2, 0);
    }
    request->send(200, "text/plain", "Action effectuée !");
  });

  // Commencez le serveur web asynchrone
  server.begin();
}

void loop() {
  // Boucle principale (peut être vide, car le serveur web est asynchrone)
}
