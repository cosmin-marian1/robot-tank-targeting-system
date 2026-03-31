#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// --- 1. PIN DEFINITIONS ---
#define IN1 5   // D1
#define IN2 4   // D2
#define ENA 14  // D5
#define IN3 0   // D3
#define IN4 2   // D4
#define ENB 12  // D6

#define SERVO_PIN 13 // D7
#define LASER_PIN 15 // D8

// Ultrasonic sensor connected to serial pins (TX/RX)
#define TRIG_PIN 1   // TX
#define ECHO_PIN 3   // RX

// --- 2. NETWORK SETTINGS (OPEN ACCESS POINT) ---
const char* ssid = "Tanc";
// No password

ESP8266WebServer server(80);
Servo myServo;

// --- 3. GLOBAL VARIABLES ---
double servoPos = 90.0;
long duration;
int distance = 0;
unsigned long laserTimer = 0; // Timer for firing effect
bool isFiring = false;        // Firing state

// --- 4. WEB INTERFACE (HTML/CSS/JS) ---
const char PROGMEM htmlPage[] = R"=====(

<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>L.A.R.S. Control</title>
  <style>
    body { background-color: #121212; color: #eee; text-align: center; font-family: sans-serif; margin: 0; padding: 0; overflow: hidden; touch-action: none; }
    .container { display: flex; flex-direction: row; justify-content: space-between; align-items: center; height: 100vh; padding: 10px; box-sizing: border-box; }
    .joy-area { flex: 1; display: flex; justify-content: center; align-items: center; }
    .center-panel { flex: 0.6; display: flex; flex-direction: column; justify-content: center; align-items: center; gap: 20px; }
    canvas { background: #222; border-radius: 50%; box-shadow: 0 0 15px #000; border: 2px solid #444; }

    .btn-stop {
      width: 70px; height: 70px;
      background-color: #d32f2f; color: white;
      border: 3px solid #ff5252;
      border-radius: 50%;
      font-weight: bold;
      cursor: pointer;
      box-shadow: 0 5px 10px rgba(0,0,0,0.5);
    }

    .btn-shoot {
      width: 90px; height: 90px;
      background-color: #ff9800;
      color: white;
      border: 4px solid #fff;
      border-radius: 50%;
      font-size: 16px; font-weight: 900;
      box-shadow: 0 0 20px #ff9800;
      cursor: pointer;
      transition: all 0.2s;
    }

    .btn-shoot:active { transform: scale(0.9); background-color: #ffb74d; }

    .btn-shoot:disabled {
      background-color: #333;
      border-color: #555;
      box-shadow: none;
      color: #777;
      cursor: not-allowed;
      opacity: 0.8;
    }

    .radar-box { font-family: monospace; font-size: 18px; font-weight: bold; min-height: 60px; }
    #distVal { font-size: 26px; transition: color 0.3s; display:block; }
    #warnMsg {
      color: #ff1744;
      font-size: 14px;
      display: none;
      text-transform: uppercase;
      animation: blink 0.5s infinite;
      margin-top:5px;
    }

    @keyframes blink { 50% { opacity: 0; } }

    .label {
      position: absolute;
      bottom: 10px;
      color: #666;
      font-size: 10px;
      pointer-events: none;
    }
  </style>
</head>
<body>
  <div class="container">

    <div class="joy-area">
      <canvas id="joyDrive" width="200" height="200"></canvas>
      <div class="label">MOTORS</div>
    </div>

    <div class="center-panel">
      <div class="radar-box">
        RADAR<br>
        <span id="distVal">-- cm</span>
        <div id="warnMsg">TOO CLOSE!</div>
      </div>

      <button id="shootBtn" class="btn-shoot" onclick="fireLaser()">SHOOT</button>
      <button class="btn-stop" onclick="emergencyStop()">STOP</button>
    </div>

    <div class="joy-area">
      <canvas id="joyTurret" width="200" height="200"></canvas>
      <div class="label">TURRET</div>
    </div>

  </div>

  <script>
    class Joystick {
      constructor(canvasId, color, callback) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.width = this.canvas.width; this.height = this.canvas.height;
        this.centerX = this.width / 2; this.centerY = this.height / 2;
        this.radius = 35; this.baseRadius = 80;
        this.color = color; this.callback = callback;
        this.isDragging = false;
        this.draw(this.centerX, this.centerY);

        const events = ['touchstart', 'touchmove', 'touchend', 'mousedown', 'mousemove', 'mouseup'];
        events.forEach(evt => this.canvas.addEventListener(evt, (e) => this.handleEvent(e)));
      }

      draw(x, y) {
        this.ctx.clearRect(0, 0, this.width, this.height);
        this.ctx.beginPath(); this.ctx.arc(this.centerX, this.centerY, this.baseRadius, 0, Math.PI * 2);
        this.ctx.fillStyle = 'rgba(255, 255, 255, 0.05)'; this.ctx.fill(); this.ctx.stroke();
        this.ctx.beginPath(); this.ctx.arc(x, y, this.radius, 0, Math.PI * 2);
        this.ctx.fillStyle = this.color; this.ctx.fill(); this.ctx.stroke();
      }

      handleEvent(e) {
        if (e.type === 'touchstart' || e.type === 'mousedown') this.isDragging = true;
        if (e.type === 'touchend' || e.type === 'mouseup') {
          this.isDragging = false;
          this.draw(this.centerX, this.centerY);
          this.callback(0, 0);
          return;
        }

        if (!this.isDragging) return;

        e.preventDefault();

        let cx = e.touches ? e.touches[0].clientX : e.clientX;
        let cy = e.touches ? e.touches[0].clientY : e.clientY;
        let rect = this.canvas.getBoundingClientRect();

        let nx = this.centerX + Math.min(Math.sqrt(Math.pow((cx - rect.left) - this.centerX, 2) + Math.pow((cy - rect.top) - this.centerY, 2)), this.baseRadius) * Math.cos(Math.atan2((cy - rect.top) - this.centerY, (cx - rect.left) - this.centerX));
        let ny = this.centerY + Math.min(Math.sqrt(Math.pow((cx - rect.left) - this.centerX, 2) + Math.pow((cy - rect.top) - this.centerY, 2)), this.baseRadius) * Math.sin(Math.atan2((cy - rect.top) - this.centerY, (cx - rect.left) - this.centerX));

        this.draw(nx, ny);
        this.callback(
          Math.round((nx - this.centerX) / (this.baseRadius / 100)),
          Math.round(-(ny - this.centerY) / (this.baseRadius / 100))
        );
      }
    }

    var lastD = 0, lastT = 0;

    function sendD(x, y) {
      if (Date.now() - lastD < 100) return;
      lastD = Date.now();
      fetch("/drive?x=" + x + "&y=" + y);
    }

    function sendT(x, y) {
      if (Date.now() - lastT < 100) return;
      lastT = Date.now();
      fetch("/turret?val=" + x);
    }

    const joyD = new Joystick("joyDrive", "#2196F3", sendD);
    const joyT = new Joystick("joyTurret", "#FF9800", sendT);

    function emergencyStop() {
      joyD.isDragging = false;
      joyT.isDragging = false;
      joyD.draw(100, 100);
      joyT.draw(100, 100);
      fetch("/stop");
      navigator.vibrate(300);
    }

    function fireLaser() {
      var btn = document.getElementById("shootBtn");
      if (!btn.disabled) {
        fetch("/shoot");
        navigator.vibrate(50);
      }
    }

    setInterval(function() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.status == 200) {
          var cm = parseInt(this.responseText);
          var label = document.getElementById("distVal");
          var msg = document.getElementById("warnMsg");
          var btn = document.getElementById("shootBtn");

          label.innerText = cm + " cm";

          if (cm > 0 && cm < 20) {
            label.style.color = "#ff1744";
            msg.style.display = "block";
            btn.disabled = true;
            btn.innerText = "NO!";
          } else {
            label.style.color = "#00e676";
            msg.style.display = "none";
            btn.disabled = false;
            btn.innerText = "SHOOT";
          }
        }
      };
      xhr.open('GET', "/dist", true);
      xhr.send();
    }, 500);
  </script>
</body>
</html>

)=====";

// --- 5. HARDWARE FUNCTIONS ---

void controlMotor(int pin1, int pin2, int pinPWM, int val) {
  int pwmSpeed = map(abs(val), 0, 100, 0, 1023);
  if (pwmSpeed < 50) pwmSpeed = 0;

  analogWrite(pinPWM, pwmSpeed);

  if (val > 0) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
  } else if (val < 0) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
  } else {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
}

void handleDrive() {
  if (server.hasArg("x") && server.hasArg("y")) {
    int x = -server.arg("x").toInt(); // Inverted controls fix
    int y = server.arg("y").toInt();

    controlMotor(IN1, IN2, ENA, constrain(y + x, -100, 100));
    controlMotor(IN3, IN4, ENB, constrain(y - x, -100, 100));
  }

  server.send(200, "text/plain", "OK");
}

void handleTurret() {
  if (server.hasArg("val")) {
    int val = -server.arg("val").toInt(); // Inverted turret fix
    double step = val * 0.15; // Fine movement speed

    servoPos += step;
    servoPos = constrain(servoPos, 0, 180);
    myServo.write((int)servoPos);
  }

  server.send(200, "text/plain", "OK");
}

void handleShoot() {
  // Double-check safety on the server side
  if (distance > 20 || distance == 0) {
    isFiring = true;
    laserTimer = millis();
    digitalWrite(LASER_PIN, HIGH);
    server.send(200, "text/plain", "BANG");
  } else {
    server.send(200, "text/plain", "BLOCKED");
  }
}

void handleStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
  server.send(200, "text/plain", "STOPPED");
}

void handleDist() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 25000);
  distance = (duration == 0) ? 0 : (duration * 0.034 / 2);

  server.send(200, "text/plain", String(distance));
}

void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(90);

  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.softAP(ssid);

  server.on("/", []() { server.send(200, "text/html", htmlPage); });
  server.on("/drive", handleDrive);
  server.on("/turret", handleTurret);
  server.on("/shoot", handleShoot);
  server.on("/stop", handleStop);
  server.on("/dist", handleDist);

  server.begin();
}

void loop() {
  server.handleClient();

  if (isFiring) {
    if (millis() - laserTimer > 500) {
      digitalWrite(LASER_PIN, LOW);
      isFiring = false;
    }
  }
}
