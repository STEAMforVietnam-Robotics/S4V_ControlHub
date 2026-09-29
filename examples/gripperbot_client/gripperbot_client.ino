/*
 * GripperBot Digital Twin — ESP32-S3 + ControlHub (PCA9685+DRV8874)
 * M1=Left, M4=Right, M3=Gripper | WiFi Manager + Auto-Discovery
 * Libs: Adafruit PWM Servo Driver, ArduinoJson >=6, WebSockets >=2.4
 */
#include <WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
// #include <Adafruit_PWMServoDriver.h>
#define CH_NO_BLE
#include <ControlHub.h>

#define LED_NUM 160

Motor leftMotor (M2, true);    // M1
Motor rightMotor(M4, false);    // M4
Motor armMotor  (M1);    // M2
Servo gripper (S1);    // S1
Led   led(P3, LED_NUM);    // P3

// === CONFIG ===
const char* DEVICE_NAME = "GripperBot-ControlHub";
const char* AP_SSID     = "Clawbot_Digital_Twin";
uint16_t BRIDGE_PORT    = 8765;
char bridgeHost[32]     = "192.168.1.100";

const char* SSID = "NextGen_Robotics_2.4G";
const char* PASS = "NextGenRobotics";

// // === MOTORS (PCA9685 channels) ===
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// const uint8_t M1[2]={3,2}, M3[2]={0,1}, M4[2]={13,12};

// void mset(const uint8_t c[2], int16_t s) {
//     s = constrain(s,-100,100);
//     uint16_t d = map(abs(s),0,100,0,4095);
//     pwm.setPWM(c[0],0,s>0?d:0); pwm.setPWM(c[1],0,s<0?d:0);
// }
// void drive(int16_t l, int16_t r) { mset(M1,l); mset(M4,r); }
void stopAll() { 
    Robot.stop();
    armMotor.stop(STOP_BRAKE);
    }

// === WEBSOCKET ===
WebSocketsClient ws;
bool wsOk = false;
unsigned long lastStatus = 0;

void sendJson(const char* type, const char* cmd=nullptr) {
    if (!wsOk) return;
    StaticJsonDocument<128> d; char buf[128];
    d["type"]=type;
    if (cmd) { d["cmd"]=cmd; d["ok"]=true; }
    else if (!strcmp(type,"status")) { d["uptime"]=millis()/1000; d["device"]=DEVICE_NAME; d["wifi_rssi"]=WiFi.RSSI(); }
    else d["device"]=DEVICE_NAME;
    serializeJson(d,buf); ws.sendTXT(buf);
}

void onCmd(const char* json) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc,json)) return;
    const char* c = doc["cmd"]; if (!c) return;
    int s=doc["speed"]|70, a=doc["speed"]|60;
    if      (!strcmp(c,"go_forward"))  Robot.go_forward(70);
    else if (!strcmp(c,"go_backward")) Robot.go_backward(70);
    else if (!strcmp(c,"turn_left"))   Robot.turn_left(70);
    else if (!strcmp(c,"turn_right"))  Robot.turn_right(70);
    else if (!strcmp(c,"stop"))        Robot.stop();
    else if (!strcmp(c,"arm_up"))      armMotor.rotate(80);
    else if (!strcmp(c,"arm_down"))    armMotor.rotate(-45);
    else if (!strcmp(c,"lift_stop"))   armMotor.stop(STOP_BRAKE);
    else if (!strcmp(c,"home")||!strcmp(c,"stop_all")) stopAll();
    Serial.printf("[CMD] %s\n",c); sendJson("ack",c);
}

void wsEvent(WStype_t t, uint8_t* p, size_t) {
    if (t==WStype_CONNECTED)    { wsOk=true;  sendJson("hardware_hello"); }
    else if (t==WStype_DISCONNECTED) { wsOk=false; stopAll(); }
    else if (t==WStype_TEXT)    onCmd((const char*)p);
}

void startWS() {
    ws.begin(bridgeHost,BRIDGE_PORT,"/hardware");
    ws.onEvent(wsEvent); ws.setReconnectInterval(3000);
}

// === UDP DISCOVERY ===
WiFiUDP udp;
bool discover(unsigned long ms=10000) {
    udp.begin(0);
    unsigned long t0=millis();
    while (millis()-t0<ms) {
        udp.beginPacket(IPAddress(255,255,255,255),8766);
        udp.print("S4VN_BRIDGE_DISCOVER"); udp.endPacket();
        unsigned long w=millis();
        while (millis()-w<1000) {
            if (udp.parsePacket()>0) {
                char b[64]; int l=udp.read(b,63); b[l]='\0';
                if (!strncmp(b,"S4VN_BRIDGE_HERE:",17)) {
                    char*p=b+17,*c=strrchr(p,':');
                    if(c){*c='\0';strncpy(bridgeHost,p,31);BRIDGE_PORT=atoi(c+1);}
                    else strncpy(bridgeHost,p,31);
                    udp.stop(); return true;
                }
            } delay(50);
        }
    }
    udp.stop(); return false;
}

// === WIFI MANAGER ===
Preferences prefs; WebServer srv(80); DNSServer dns;
bool apMode=false; String nets="";

void scanNets() { int n=WiFi.scanNetworks(); nets="";
    for(int i=0;i<n;i++){if(i)nets+="\n";nets+=WiFi.SSID(i)+"|"+String(WiFi.RSSI(i))+"|"+String(WiFi.encryptionType(i)!=WIFI_AUTH_OPEN?1:0);}
    WiFi.scanDelete();
}

const char PAGE[] PROGMEM = R"HTML(<!DOCTYPE html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Clawbot WiFi</title><style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui;background:#0f172a;color:#e2e8f0;padding:16px;max-width:400px;margin:0 auto}
h2{color:#38bdf8;margin-bottom:12px;text-align:center}
.i{background:#1e293b;padding:10px;border-radius:8px;margin-bottom:12px;font-size:13px;text-align:center;color:#94a3b8}
.n{background:#1e293b;padding:10px 12px;margin:4px 0;border-radius:8px;cursor:pointer;display:flex;justify-content:space-between;border:1px solid transparent}
.n:hover{border-color:#38bdf8;background:#334155}
.n b{font-weight:600}.n span{color:#94a3b8;font-size:12px}
.l::after{content:" 🔒";font-size:11px}
input,button{width:100%;padding:10px;border-radius:8px;border:1px solid #334155;font-size:14px;margin-top:6px}
input{background:#1e293b;color:#e2e8f0}input:focus{outline:none;border-color:#38bdf8}
button{background:#0ea5e9;color:#fff;border:none;font-weight:700;cursor:pointer;margin-top:12px}
button:hover{background:#0284c7}.b2{background:#6366f1}.b2:hover{background:#4f46e5}
#m{text-align:center;margin-top:8px;font-size:13px;color:#fbbf24}
hr{border:0;border-top:1px solid #334155;margin:12px 0}
</style></head><body>
<h2>🤖 Clawbot Digital Twin</h2>
<div class="i" id="d">...</div><div id="w">Dang quet...</div><hr>
<form onsubmit="return sv()"><input id="s" placeholder="Ten WiFi" required>
<input id="p" type="password" placeholder="Mat khau">
<button type="submit">Ket noi</button></form>
<button class="b2" onclick="sc()">Quet lai</button><div id="m"></div>
<script>
function ld(){fetch('/info').then(r=>r.json()).then(j=>{document.getElementById('d').innerHTML='<b>'+j.device+'</b><br>MAC: '+j.mac});
fetch('/scan').then(r=>r.text()).then(t=>{let e=document.getElementById('w');if(!t){e.innerHTML='<div class="i">Khong co mang</div>';return}
let h='';t.trim().split('\n').forEach(l=>{let p=l.split('|');h+='<div class="n'+(p[2]==='1'?' l':'')+'" onclick="pk(\''+p[0]+'\')"><b>'+p[0]+'</b><span>'+p[1]+' dBm</span></div>'});e.innerHTML=h})}
function pk(s){document.getElementById('s').value=s;document.getElementById('p').focus()}
function sv(){let s=document.getElementById('s').value,p=document.getElementById('p').value;document.getElementById('m').innerText='Dang ket noi...';
fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'ssid='+encodeURIComponent(s)+'&pass='+encodeURIComponent(p)}).then(r=>r.text()).then(t=>{document.getElementById('m').innerText=t});return false}
function sc(){document.getElementById('w').innerHTML='Dang quet...';fetch('/rescan').then(()=>setTimeout(ld,500))}
onload=ld</script></body></html>)HTML";

void setupRoutes() {
    srv.on("/",HTTP_GET,[]{srv.send(200,"text/html",PAGE);});
    srv.on("/info",HTTP_GET,[]{char b[100];snprintf(b,99,"{\"device\":\"%s\",\"mac\":\"%s\"}",DEVICE_NAME,WiFi.macAddress().c_str());srv.send(200,"application/json",b);});
    srv.on("/scan",HTTP_GET,[]{srv.send(200,"text/plain",nets);});
    srv.on("/rescan",HTTP_GET,[]{scanNets();srv.send(200,"text/plain","OK");});
    srv.on("/save",HTTP_POST,[]{
        String s=srv.arg("ssid"),p=srv.arg("pass");
        if(!s.length()){srv.send(400,"text/plain","SSID trong");return;}
        prefs.begin("wifi",false);prefs.putString("ssid",s);prefs.putString("pass",p);prefs.end();
        srv.send(200,"text/plain","Da luu! Khoi dong lai...");delay(1000);ESP.restart();
    });
}

void startAP() {
    apMode=true; WiFi.mode(WIFI_AP_STA); WiFi.softAP(AP_SSID);
    dns.start(53,"*",WiFi.softAPIP()); scanNets(); setupRoutes();
    srv.onNotFound([]{srv.sendHeader("Location","http://192.168.4.1",true);srv.send(302,"text/plain","");});
    srv.begin(); Serial.printf("[AP] %s -> http://192.168.4.1\n",AP_SSID);
}

void stopAP() { if(apMode){dns.stop();srv.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);apMode=false;} }

bool connectWiFi() {
    // prefs.begin("wifi",true); String s=prefs.getString("ssid",""),p=prefs.getString("pass",""); prefs.end();
    // if(!s.length()) return false;
    //Serial.printf("[WIFI] -> %s",s.c_str()); 
    WiFi.mode(WIFI_STA); WiFi.begin(SSID,PASS);
    unsigned long t0=millis();
    while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");if(millis()-t0>15000){Serial.println(" FAIL");return false;}}
    Serial.printf("\n[WIFI] OK %s\n",WiFi.localIP().toString().c_str()); return true;
}

// === SETUP ===
void setup() {
    Serial.begin(115200); delay(300);
    Serial.println("\n[GripperBot] Starting...");

    ControlHub.begin(false, false, false, CH_BLE_DEFAULT_NAME);               // disable USB + BLE + RS485
    leftMotor.begin();
    rightMotor.begin();
    armMotor.begin();
    gripper.begin();
    led.begin();

    Robot.setDriveTrain(&leftMotor, &rightMotor);
    gripper.setLimits(70, 120);        // servo angle limits
    led.setBrightness(255);
    led.setColor(0, 0, 255);    // blue

    // Wire.begin(47,21,400000UL);
    // pwm.begin(); pwm.setPWMFreq(1500); stopAll();

    if (!connectWiFi()) { startAP(); return; }
    if(discover()) startWS();
}

// === LOOP ===
unsigned long lastDisc=0, lastWC=0;
void loop() {
    if (apMode) {
        dns.processNextRequest(); srv.handleClient();
        if (WiFi.status()==WL_CONNECTED) { stopAP(); if(discover(10000))startWS(); }
        return;
    }
    if (WiFi.status()!=WL_CONNECTED) {
        if (millis()-lastWC>30000) { lastWC=millis(); stopAll(); WiFi.reconnect(); delay(5000);
            if(WiFi.status()!=WL_CONNECTED){startAP();return;} }
        return;
    }
    if (!bridgeHost[0]) { if(millis()-lastDisc>5000){lastDisc=millis();if(discover(3000))startWS();} return; }
    ws.loop();
    if (wsOk&&millis()-lastStatus>5000) { sendJson("status"); lastStatus=millis(); }
}
