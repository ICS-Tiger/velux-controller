#include "web_server.h"
#include "config.h"

WebServerHandler::WebServerHandler() : server(WEB_SERVER_PORT) {}

void WebServerHandler::begin() {
    // Root
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Velux Controller</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; background: #1a1a1a; color: #fff; padding: 20px; }
        .container { max-width: 1200px; margin: 0 auto; }
        h1 { text-align: center; margin-bottom: 30px; color: #4CAF50; }
        .motors { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        .motor { background: #2a2a2a; border-radius: 10px; padding: 20px; border: 2px solid #333; }
        .motor h2 { color: #4CAF50; margin-bottom: 15px; }
        .controls { display: flex; gap: 10px; margin-bottom: 15px; }
        button { flex: 1; padding: 15px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; font-weight: bold; transition: 0.3s; }
        .btn-open { background: #4CAF50; color: white; }
        .btn-open:hover { background: #45a049; }
        .btn-close { background: #f44336; color: white; }
        .btn-close:hover { background: #da190b; }
        .btn-stop { background: #ff9800; color: white; }
        .btn-stop:hover { background: #e68900; }
        .slider { width: 100%; margin: 15px 0; }
        .status { background: #333; padding: 10px; border-radius: 5px; margin: 10px 0; font-size: 14px; }
        .learn { margin-top: 15px; padding-top: 15px; border-top: 1px solid #444; }
        .learn button { background: #2196F3; }
        .learn button:hover { background: #0b7dda; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏠 Velux Rolladen Controller</h1>
        <div class="motors" id="motors"></div>
    </div>
    
    <script>
        const motors = [1, 2, 3, 4];
        
        function createMotorCard(id) {
            return "<div class=\"motor\">" +
                "<h2>Motor " + id + "</h2>" +
                "<div class=\"controls\">" +
                "<button class=\"btn-open\" onclick=\"control(" + id + ", 'OPEN')\">AUF</button>" +
                "<button class=\"btn-stop\" onclick=\"control(" + id + ", 'STOP')\">STOP</button>" +
                "<button class=\"btn-close\" onclick=\"control(" + id + ", 'CLOSE')\">ZU</button>" +
                "</div>" +
                "<input type=\"range\" class=\"slider\" min=\"0\" max=\"100\" value=\"50\" onchange=\"control(" + id + ", this.value)\" id=\"slider" + id + "\">" +
                "<div class=\"status\" id=\"status" + id + "\">Position: --%</div>" +
                "<div class=\"learn\">" +
                "<button onclick=\"learn(" + id + ", 'open')\">Oeffnungszeit lernen</button>" +
                "<button onclick=\"learn(" + id + ", 'close')\">Schliesszeit lernen</button>" +
                "</div>" +
                "</div>";
        }
        
        motors.forEach(function(id) {
            document.getElementById("motors").innerHTML += createMotorCard(id);
        });
        
        function control(motor, cmd) {
            fetch("/motor" + motor + "/control?cmd=" + cmd)
                .then(function(r) { return r.text(); })
                .then(function(data) { console.log(data); });
        }
        
        function learn(motor, type) {
            if (confirm("Motor " + motor + ": " + (type === "open" ? "Oeffnungszeit" : "Schliesszeit") + " lernen?")) {
                fetch("/motor" + motor + "/learn?type=" + type)
                    .then(function(r) { return r.text(); })
                    .then(function(data) { alert(data); });
            }
        }
        
        function updateStatus() {
            fetch("/status")
                .then(function(r) { return r.json(); })
                .then(function(data) {
                    motors.forEach(function(id) {
                        const m = data["motor" + id];
                        if (m) {
                            document.getElementById("status" + id).innerHTML = 
                                "Position: " + m.position + "%, Kalibriert: " + (m.calibrated ? "JA" : "NEIN");
                            document.getElementById("slider" + id).value = m.position;
                        }
                    });
                });
        }
        
        setInterval(updateStatus, 1000);
        updateStatus();
    </script>
</body>
</html>
        )");
    });
    
    // Motor Control
    for (int i = 1; i <= 4; i++) {
        String path = "/motor" + String(i) + "/control";
        server.on(path.c_str(), HTTP_GET, [this, i](AsyncWebServerRequest *request){
            if (request->hasParam("cmd")) {
                String cmd = request->getParam("cmd")->value();
                
                if (i == 1 && onMotor1Command) onMotor1Command(cmd.c_str());
                else if (i == 2 && onMotor2Command) onMotor2Command(cmd.c_str());
                else if (i == 3 && onMotor3Command) onMotor3Command(cmd.c_str());
                else if (i == 4 && onMotor4Command) onMotor4Command(cmd.c_str());
                
                request->send(200, "text/plain", "OK");
            } else {
                request->send(400, "text/plain", "Missing cmd");
            }
        });
    }
    
    // Learn
    for (int i = 1; i <= 4; i++) {
        String path = "/motor" + String(i) + "/learn";
        server.on(path.c_str(), HTTP_GET, [this, i](AsyncWebServerRequest *request){
            if (request->hasParam("type")) {
                String type = request->getParam("type")->value();
                
                if (i == 1 && onMotor1Learn) onMotor1Learn(type.c_str());
                else if (i == 2 && onMotor2Learn) onMotor2Learn(type.c_str());
                else if (i == 3 && onMotor3Learn) onMotor3Learn(type.c_str());
                else if (i == 4 && onMotor4Learn) onMotor4Learn(type.c_str());
                
                request->send(200, "text/plain", "Learn gestartet");
            } else {
                request->send(400, "text/plain", "Missing type");
            }
        });
    }
    
    // Status
    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (getStatusJson) {
            String json = getStatusJson();
            request->send(200, "application/json", json);
        } else {
            request->send(500, "text/plain", "Status nicht verfügbar");
        }
    });
    
    server.begin();
    Serial.println("Webserver: Gestartet auf Port " + String(WEB_SERVER_PORT));
}
