
struct servo_angulos_t {
  uint8_t min; // angulo minimo [0;180]
  uint8_t max; // angulo maximo [0;180]
  uint8_t atual; // posicao atual
};

// Variaveis

// web server assincrono na porta 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// LED
const uint8_t PIN_LED = 15;

// JSON aliases
const char *ALIAS_ANGULO = "angulo";
const char *ALIAS_POSICAO = "posicao";
const char *ALIAS_SERVO = "servo";
const char *ALIAS_VELOCIDADE = "velocidade";
const char *ALIAS_VBAT = "vbat";

// variaveis da Vespa
VespaMotors motores;
VespaServo servos[4];
const uint16_t SERVO_MAX = 2500;
const uint16_t SERVO_MIN = 500;
enum Motor { Base = 0 , Alcance , Elevacao , Garra };
servo_angulos_t sangulos[4] = { { 0, 180, 90 },
                                { 40, 180, 90 },
                                { 80, 180, 140 },
                                { 70, 160, 100 } }; 
VespaBattery vbat;
uint8_t vbat_critico = 0xFF;
const uint32_t INTERVALO_ATUALIZACAO_VBAT = 5000; // [ms]
const uint32_t INTERVALO_ATUALIZACAO_DESCONEXAO = 100; // [ms]
const uint32_t INTERVALO_LED_VBAT_HIGH = 1000; // [ms]
const uint32_t INTERVALO_LED_VBAT_LOW = 500; // [ms]
uint32_t timeout_vbat, timeout_desconexao, timeout_led_vbat;
bool habilitar_reset_motores = true;

// --------------------------------------------------
// Pagina web principal

const char html_index[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>
        RoboCore Joystick + RoboARM
    </title>

    <!-- Última atualização: 24/07/2023 -->

    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=0">
    <style>

        html, body {width: 100%; height: 100%; padding: 0; margin: 0; overscroll-behavior: none;}
      
        body {
          overflow: hidden;
          -moz-user-select: none; 
          -webkit-user-select: none;
          -ms-user-select:none; 
          user-select:none;
          -o-user-select:none;
        }

        .container {
          height: 26px;
          width: 50px;
          position: relative;
        }
        .container * {
          position: absolute;
        }

        .battery_warning {
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          height: 20px;
          width: 40px;
          border: 2px solid black;
          border-radius: 5px;
          padding: 1px;
        }
        .battery_warning::before {
          content: '';
          position: absolute;
          height: 13px;
          width: 3px;
          background: black;
          left: 44px;
          top: 50%;
          transform: translateY(-50%);
          border-radius: 0 3px 3px 0;
        }
        
        .battery {
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          height: 20px;
          width: 40px;
          border: 2px solid #F1F1F1;
          border-radius: 5px;
          padding: 1px;
        }
        .battery::before {
          content: '';
          position: absolute;
          height: 13px;
          width: 3px;
          background: #F1F1F1;
          left: 44px;
          top: 50%;
          transform: translateY(-50%);
          border-radius: 0 3px 3px 0;
        }
        .part {
          background: #0F0;
          top: 1px;
          left: 1px;
          bottom: 1px;
          border-radius: 3px;
        }

        @keyframes animate {
          0% {
            width: 0%;
            background: #F00;
          }
          50% {
            width: 48%;
            background:orange;
          }
          100% {
            width: 95%;
            background: #0F0;
          }
        }

        .slidecontainer {
          width: 100%; /* Width of the outside container */
        }

        /* The slider itself */
        .slider {
          -webkit-appearance: none;  /* Override default CSS styles */
          appearance: none;
          width: calc(100% - 5px); /* Full-width */
          height: 50px; /* Specified height */
          background: #ECE5E5; /* Grey background */
          outline: none; /* Remove outline */
          opacity: 0.7; /* Set transparency (for mouse-over effects on hover) */
          -webkit-transition: .2s; /* 0.2 seconds transition on hover */
          transition: opacity .2s;
        }

        /* Mouse-over effects */
        .slider:hover {
          opacity: 1; /* Fully shown on mouse-over */
        }

        /* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */
        .slider::-webkit-slider-thumb {
          -webkit-appearance: none; /* Override default look */
          appearance: none;
          width: 50px; /* Set a specific slider handle width */
          height: 50px; /* Slider handle height */
          background: lightgray; /* gray background */
          cursor: pointer; /* Cursor on hover */
        }

        .slider::-moz-range-thumb {
          width: 50px; /* Set a specific slider handle width */
          height: 50px; /* Slider handle height */
          background: #04AA6D; /* Green background */
          cursor: pointer; /* Cursor on hover */
        }
        .slider_vertical_container {
                height: 150px; overflow: hidden; border: 0px solid; width: 50px; position: relative;
        }
        .slider_vertical {
            transform: rotate(270deg); position: absolute; top: 50px; left: -50px; width: 150px;
        }
        
    </style>
</head>

<body style="height: 100%;  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif ;">
    <div style="display: none; width: 100%; height: 100%; position: absolute; align-content: center; justify-content: center; align-items: center; z-index: 10;" onclick="reinicia();" id="ad">
        <img src="https://robocore-eventos.s3.sa-east-1.amazonaws.com/public/explorer_ad.jpg" style="width: 100%; height: 100%;" alt="alt"/>
    </div>
    <div style="display: none; width: 100%; height: 100%; position: absolute; align-content: center; justify-content: center; align-items: center; z-index: 10;" id="bat">
        <div style="background-color: yellow; width: 100%; display: flex; align-content: center; justify-content: center; align-items: center;">
            <div class="container" style="margin-right: 10px;">
                <div class="battery_warning">
                    <div class="part"></div>
                </div>
            </div>
            <h1>Bateria fraca</h1>
        </div>
    </div>
    <div style="line-height: 26px; background-color: black; padding: 10px; padding-bottom: 0px;">
        <div class="container" style="float: right; margin-right: 10px;">
            <div class="battery">
                <div id="lbat" class="part"></div>
            </div>
        </div>
        <div style="float: right; color: white; font-size: 18px; line-height: 26px; margin-right: 5px;">
            <span id="vbat">0</span> V
        </div>

        <div style="width: 100%; border: 0px solid red; text-align: center;">

            <svg version="1.0" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
            height="30px" viewBox="0 0 1420 225" xml:space="preserve">
            <g id="Layer_1" fill="#f0be00">
                <g>
                    <path id="Robo" fill-rule="evenodd" clip-rule="evenodd" d="M175.7,94.9c0,11.7-6.2,20.2-18.5,25.6c-9.8,4-21,5.9-33.8,5.9H84.5
                    l78.8,33.5c4.2,1.8,6.3,4.6,6.3,8.2c0,5.3-4.6,8-13.9,8c-3.7,0-6.6-0.3-8.7-0.8l-107.6-46v38.4c0,5.6-2.8,8.4-8.3,8.4
                    c-5.9,0-9.3-0.3-10.2-0.7c-3.4-1.1-5.1-3.6-5.1-7.7V54.8c0-5.6,2.8-8.3,8.3-8.3h95c14.7,0,26.8,1.9,36.3,5.7
                    c13.5,5.5,20.3,14.5,20.3,27V94.9L175.7,94.9z M152.1,95.1V78.7c0-8.7-11.1-13-33.1-13H39.4v41.5h83.8
                    C142.4,107.2,152.1,103.2,152.1,95.1L152.1,95.1z M343.9,141.2c0,23.2-20,34.7-60,34.7h-36.8c-16.2,0-29-2.1-38.3-6.2
                    c-14.3-5.8-21.5-15.3-21.5-28.5V81.6c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9c13.7,5.3,20.6,15.4,20.6,30.2
                    V141.2L343.9,141.2z M320.3,140.6V82.1c0-10.9-11.9-16.4-35.8-16.4H247c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8
                    h37.5C308.4,156.5,320.3,151.2,320.3,140.6L320.3,140.6z M512.2,141.5c0,12.4-5.9,21.6-17.6,27.5c-9.1,4.6-20.5,6.9-34.2,6.9
                    H360.2c-5.5,0-8.3-2.8-8.3-8.4V54.8c0-5.5,2.8-8.3,8.3-8.3H459c8.8,0,16.9,0.7,24.2,2.1c7.9,1.9,14.5,5.4,20,10.5
                    c6,5.7,9,12.4,9,19.9c0,7.4-0.3,12.7-0.8,16c-0.1,0.6-0.6,1.7-1.4,3.2c-0.8,1.5-1.9,3.3-3.2,5.6c-2.6,4.5-3.9,7-3.9,7.5
                    c0,0.4,1.3,2.6,3.9,6.7c1.3,2.1,2.4,3.8,3.2,5.2c0.8,1.4,1.2,2.5,1.4,3.3c0.3,1.1,0.5,2.9,0.6,5.4
                    C512.1,134.4,512.2,137.6,512.2,141.5L512.2,141.5z M489.2,90.1c0-2.4-0.2-5.9-0.7-10.8c-0.7-9-10.9-13.5-30.6-13.5H376v36.3h82.8
                    C479.1,102.1,489.2,98.1,489.2,90.1L489.2,90.1z M488.5,134.9c0-8.9-9.8-13.4-29.2-13.4h-83.8v35h82.2c19,0,29.3-4.1,30.6-12.2
                    c0.1-0.4,0.1-1.4,0.2-3C488.5,139.7,488.5,137.6,488.5,134.9L488.5,134.9z M676.8,141.2c0,23.2-20,34.7-60,34.7H580
                    c-16.2,0-29-2.1-38.3-6.2c-14.3-5.8-21.5-15.3-21.5-28.5V81.6c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9
                    c13.7,5.3,20.6,15.4,20.6,30.2V141.2L676.8,141.2z M653.1,140.6V82.1c0-10.9-11.9-16.4-35.8-16.4h-37.4
                    c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8h37.5C641.2,156.5,653.1,151.2,653.1,140.6z"/>
                    <polygon id="Bolt" fill-rule="evenodd" clip-rule="evenodd" points="724,11.6 765,11.6 726,104.8 779.6,76.9 750.7,189.8 
                    765,182.8 733.6,230.4 722,172.8 732.9,184 744.9,111.7 689.2,140.7     "/>
                    <path id="Core" fill-rule="evenodd" clip-rule="evenodd" d="M935.5,57.4c0,5.6-2.8,8.4-8.3,8.4h-84c-20.4,0-30.6,4.1-30.6,12.2
                    v64.1c0,9.6,11.3,14.4,33.9,14.4h80.2c5.8,0,8.6,3.2,8.6,9.7c0,6.5-2.9,9.8-8.6,9.8h-80.2c-10.8,0-18.6-0.4-23.4-1.3
                    c-5.2-1-10.6-2.9-16.1-5.7c-6.4-3.3-10.8-6.8-13.4-10.4c-3-4.1-4.6-9.5-4.6-16.1V78.1c0-8.2,3.7-15.1,11.1-20.8
                    c6.2-4.8,13.8-7.9,22.7-9.6c2.3-0.4,5-0.6,8.3-0.8c3.3-0.2,7.1-0.3,11.6-0.3h84.5C932.7,46.5,935.5,50.2,935.5,57.4L935.5,57.4z
                    M1101.8,141.2c0,23.2-20,34.7-60,34.7H1005c-16.2,0-29-2.1-38.3-6.2c-14.3-5.8-21.5-15.3-21.5-28.5V81.6
                    c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9c13.7,5.3,20.6,15.4,20.6,30.2V141.2L1101.8,141.2z M1078.1,140.6V82.1
                    c0-10.9-11.9-16.4-35.8-16.4h-37.4c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8h37.5
                    C1066.2,156.5,1078.1,151.2,1078.1,140.6L1078.1,140.6z M1267.8,94.9c0,11.7-6.2,20.2-18.5,25.6c-9.8,4-21,5.9-33.8,5.9h-38.9
                    l78.8,33.5c4.2,1.8,6.3,4.6,6.3,8.2c0,5.3-4.6,8-13.9,8c-3.7,0-6.6-0.3-8.7-0.8l-107.6-46v38.4c0,5.6-2.8,8.4-8.3,8.4
                    c-5.9,0-9.3-0.3-10.2-0.7c-3.4-1.1-5.1-3.6-5.1-7.7V54.8c0-5.6,2.8-8.3,8.3-8.3h95c14.7,0,26.8,1.9,36.3,5.7
                    c13.5,5.5,20.3,14.5,20.3,27V94.9L1267.8,94.9z M1244.2,95.1V78.7c0-8.7-11.1-13-33.1-13h-79.6v41.5h83.8
                    C1234.6,107.2,1244.2,103.2,1244.2,95.1L1244.2,95.1z M1416.5,111.2c0,6.9-2.9,10.3-8.8,10.3h-113.6v21.3
                    c0,9.1,10.1,13.7,30.3,13.7h83.3c5.5,0,8.3,2.8,8.3,8.4c0,7.4-2.8,11.1-8.3,11.1h-84.4c-12.3,0-23.6-2.2-33.9-6.6
                    c-12.6-5.7-18.9-14.3-18.9-25.7V79.5c0-6.2,2.1-11.8,6.3-16.8c9.1-10.8,25.3-16.1,48.6-16.1h23.1c5.4,0,8.2,3.6,8.2,10.8
                    c0,5.6-2.8,8.4-8.2,8.4h-3.4h-2.1h-1.2c-0.8,0-1.2,0-1.3-0.1c-1.8-0.1-3.3-0.1-4.6-0.1c-1.3,0-2.3-0.1-3.2-0.1
                    c-25.9,0-38.8,4.7-38.8,14v22.6h113.9C1413.6,102.1,1416.5,105.1,1416.5,111.2z"/>
                </g>
            </g>
        </svg>
    </div>

</div>

<div style="color:rgb(128, 128, 128); font-size: medium; text-align: left; width: 300px; border: 0px solid red; position: absolute; top: 0px; left: 0px; visibility: hidden;">
    DEBUG: Vel: <span id="speed">0</span>% | 
    Ang: <span id="angle">0</span> | 
    Botão: <span id="button">0</span>
</div>

<div style="color:rgb(128, 128, 128); font-size: medium; text-align: left; width: 300px; border: 0px solid red; position: absolute; top: 20px; left: 0px; display: none;">
    DEBUG: S1: <span id="ds1">0</span> | 
    S2: <span id="ds2">0</span> | 
    S3: <span id="ds3">0</span> |
    S4: <span id="ds4">0</span>
</div>

<div style="display: table; width:100%; height: calc(100% - 80px); border: 0px solid green; font-size: 12px; color: rgb(157, 150, 142);">
    <div style="display: table-cell; vertical-align: middle;">
        <div style="display: flex; align-items: center; justify-content: space-evenly; align-content: center; flex-direction: row; flex-wrap: wrap;">
            <canvas id="canvas_joystick" style="border: 0px solid red;"></canvas>
            <div class="slidecontainer" style="display: flex; align-items: center; justify-content: space-evenly; align-content: center; flex-direction: row; flex-wrap: wrap; max-width: 260px; margin: 10px;">
                <div>GARRA</div>
                <div style="width: 100%; white-space: nowrap;">
                    <input type="range" min="0" max="180" value="90" class="slider" style="width: calc(50% - 5px);" id="s1">
                    <input type="range" min="0" max="180" value="90" class="slider" style="width: calc(50% - 5px);" id="s1c">
                </div>
                <div style="width: 50px; text-align: right;">BRAÇO<br>ALTURA</div>
                <div class="slider_vertical_container">
                    <input type="range" min="0" max="180" value="90" class="slider slider_vertical" id="s2">
                </div>
                
                <div class="slider_vertical_container">
                    <input type="range" min="0" max="180" value="90" class="slider slider_vertical" style="transform: rotate(90deg);" id="s3">
                </div>
                <div style="width: 50px;">BRAÇO<br>DISTÂNCIA</div>
                <input type="range" min="0" max="180" value="90" class="slider" id="s4">
                <div>BASE</div>
            </div>
        </div>
    </div>
</div>
<script>
    function sliderMirror(from,to) {
        document.getElementById(to).value = 180 - document.getElementById(from).value;
    }
    
    var connection = new WebSocket(`ws://${window.location.hostname}/ws`);
    let lbat = 0;
    
    connection.onopen = function () {
      console.log('Connection opened to ' + window.location.hostname);
    };
    connection.onerror = function (error) {
      console.log('WebSocket Error ' + error);
      alert('WebSocket Error #' + error);
    };
    connection.onmessage = function (e) {
      console.log('Server: ' + e.data);
      const data = JSON.parse(e.data);

      if (data["vbat"]){
        document.getElementById("vbat").innerText = (data["vbat"] / 1000).toFixed(1);
        lbat = (data["vbat"] * 100 / 8400).toFixed(0);
        if(lbat > 100){ lbat = 100; }
        if(lbat < 2){ lbat = 2; }
        console.log("lbat=" + lbat); // debug
        document.getElementById("lbat").style.width = lbat + '%';
        if (lbat < 20){
          document.getElementById("lbat").style.backgroundColor = "#F00";
        } else if (lbat < 70){
          document.getElementById("lbat").style.backgroundColor = "orange";
        } else {
          document.getElementById("lbat").style.backgroundColor = "#0F0";
        }
      }
    };

    function send_slider(slider_id, value){
      var data = {'servo': slider_id, 'posicao': parseInt(value)};
      data = JSON.stringify(data);
      console.log('Slider data: ', data);
      connection.send(data);
    }

    function send_joystick(speed, angle){
      var data = {'velocidade': speed, 'angulo': angle};
      data = JSON.stringify(data);
      console.log('Send joystick: ', data);
      connection.send(data);
    }
</script>

<script>

    var s1 = document.getElementById("s1");
    var s2 = document.getElementById("s2");
    var s3 = document.getElementById("s3");
    var s4 = document.getElementById("s4");

    document.getElementById('s1').addEventListener('input', (e) => { sliderMirror('s1','s1c'); send_slider(1, e.target.value);  });
    document.getElementById('s1c').addEventListener('input', (e) => { sliderMirror('s1c','s1'); send_slider(1, s1.value); });
    document.getElementById('s2').addEventListener('input', (e) => { send_slider(2, e.target.value); });
    document.getElementById('s3').addEventListener('input', (e) => { send_slider(3, e.target.value); });
    document.getElementById('s4').addEventListener('input', (e) => { send_slider(4, e.target.value); });

    document.addEventListener("input", function() {
        document.getElementById("ds1").innerHTML = s1.value;
        document.getElementById("ds2").innerHTML = s2.value;
        document.getElementById("ds3").innerHTML = s3.value;
        document.getElementById("ds4").innerHTML = s4.value;
    }, false);
    
    var canvas_joystick, ctx_joystick;
    var ctx_button;
            // setup the controls for the page
            window.addEventListener('load', () => {
                canvas_joystick = document.getElementById('canvas_joystick');
                ctx_joystick = canvas_joystick.getContext('2d');
                resize();
                canvas_joystick.addEventListener('mousedown', startDrawing);
                canvas_joystick.addEventListener('mouseup', stopDrawing);
                canvas_joystick.addEventListener('mousemove', Draw);
                canvas_joystick.addEventListener('touchstart', startDrawing);
                canvas_joystick.addEventListener('touchend', stopDrawing);
                canvas_joystick.addEventListener('touchcancel', stopDrawing);
                canvas_joystick.addEventListener('touchmove', Draw);
                window.addEventListener('resize', resize);
                document.getElementById("speed").innerText = 0;
                document.getElementById("angle").innerText = 0;
                document.getElementById("button").innerText = 0;
            });
            var width, height, radius, button_size;
            let origin_joystick = { x: 0, y: 0};
            let origin_button = { x: 0, y: 0};
            const width_to_radius_ratio = 0.04;
            const width_to_size_ratio = 0.15;
            const radius_factor = 7;
            function resize() {
                if (window.innerWidth > window.innerHeight){
            width = (window.innerWidth / 2) - 2; // half the window for two canvases
        } else {
            width = (window.innerWidth) - 2;
        }

        radius = width_to_radius_ratio * width;
        button_size = width_to_size_ratio * width;
            height = radius * radius_factor * 2 + 100; // use the diameter

            // configure and draw the joystick canvas
            ctx_joystick.canvas.width = width;
            ctx_joystick.canvas.height = height;
            origin_joystick.x = width / 2;
            origin_joystick.y = height / 2;
            joystick(origin_joystick.x, origin_joystick.y);
        }


            // Draw the background/outer circle of the joystick
        	function joystick_background() {
            // clear the canvas
            ctx_joystick.clearRect(0, 0, canvas_joystick.width, canvas_joystick.height);
            // draw the background circle
            ctx_joystick.beginPath();
            ctx_joystick.arc(origin_joystick.x, origin_joystick.y, radius * radius_factor, 0, Math.PI * 2, true);
            ctx_joystick.fillStyle = '#ECE5E5';
            ctx_joystick.fill();

            //seta esquerda
            ctx_joystick.beginPath();
            ctx_joystick.moveTo(origin_joystick.x - (radius * radius_factor) - 50 , origin_joystick.y);
            ctx_joystick.lineTo(origin_joystick.x - (radius * radius_factor) - 25, origin_joystick.y+25);
            ctx_joystick.lineTo(origin_joystick.x - (radius * radius_factor) - 25, origin_joystick.y-25);
            ctx_joystick.fill();
            
            //seta superior
            ctx_joystick.beginPath();
            ctx_joystick.moveTo(origin_joystick.x, origin_joystick.y - (radius * radius_factor) - 50);
            ctx_joystick.lineTo(origin_joystick.x+25, origin_joystick.y - (radius * radius_factor) - 25);
            ctx_joystick.lineTo(origin_joystick.x-25, origin_joystick.y - (radius * radius_factor) - 25);
            ctx_joystick.fill();
            
            //seta direita
            ctx_joystick.beginPath();
            ctx_joystick.moveTo(origin_joystick.x + (radius * radius_factor) + 50 , origin_joystick.y);
            ctx_joystick.lineTo(origin_joystick.x + (radius * radius_factor) + 25, origin_joystick.y+25);
            ctx_joystick.lineTo(origin_joystick.x + (radius * radius_factor) + 25, origin_joystick.y-25);
            ctx_joystick.fill();
            
            //seta inferior
            ctx_joystick.beginPath();
            ctx_joystick.moveTo(origin_joystick.x, origin_joystick.y + (radius * radius_factor) + 50);
            ctx_joystick.lineTo(origin_joystick.x+25, origin_joystick.y + (radius * radius_factor) + 25);
            ctx_joystick.lineTo(origin_joystick.x-25, origin_joystick.y + (radius * radius_factor) + 25);
            ctx_joystick.fill();
        }

            // Draw the main circle of the joystick
            function joystick(x, y) {
            // draw the background
            joystick_background();
            // draw the joystick circle
            ctx_joystick.beginPath();
            ctx_joystick.arc(x, y, radius * 3, 0, Math.PI * 2, true);
            ctx_joystick.fillStyle = 'lightgray';
            ctx_joystick.fill();
            ctx_joystick.strokeStyle = 'lightgray';
            ctx_joystick.lineWidth = 2;
            ctx_joystick.stroke();
        }

        let coord = { x: 0, y: 0 };
        let paint = false;
        var movimento = 0;
            // Get the position of the mouse/touch press (joystick canvas)
            function getPosition_joystick(event) {
                var mouse_x = event.clientX || event.touches[0].clientX || event.touches[1].clientX;
                var mouse_y = event.clientY || event.touches[0].clientY || event.touches[1].clientY;
                coord.x = mouse_x - canvas_joystick.offsetLeft;
                coord.y = mouse_y - canvas_joystick.offsetTop;
            }

            // Check if the mouse/touch was pressed inside the background/outer circle of the joystick
            function in_circle() {
                var current_radius = Math.sqrt(Math.pow(coord.x - origin_joystick.x, 2) + Math.pow(coord.y - origin_joystick.y, 2));
            if ((radius * radius_factor) >= current_radius) { // consider the outer circle
                console.log("INSIDE circle");
                return true;
            } else {
                console.log("OUTSIDE circle");
                return false;
            }
        }

            // Handler: on press for the joystick canvas
            function startDrawing(event) {
                paint = true;
                getPosition_joystick(event);
                if (in_circle()) {
            // draw the new graphics
            joystick(coord.x, coord.y);
            Draw(event);
        }
    }

            // Handler: on release for the joystick canvas
            function stopDrawing() {
            paint = false; // reset

            // update to the default graphics
            joystick(origin_joystick.x, origin_joystick.y);
            document.getElementById("speed").innerText = 0;
            document.getElementById("angle").innerText = 0;
            // update the WebSocket client
            if (movimento == 1) {
                send_joystick(0, 0);
                movimento = 0;
            }

        }

        var last_update = 0;

        // Semi-handler: update the drawing of the joystick canvas
        function Draw(event) {
            if (paint) {
                // update the position
                getPosition_joystick(event);
                var angle_in_degrees, x, y, speed;
                // calculate the angle
                var angle = Math.atan2((coord.y - origin_joystick.y), (coord.x - origin_joystick.x));
                if (in_circle()) {
                x = coord.x - radius / 2; // correction to center on the tip of the mouse, by why? (Thought for another time.)
                y = coord.y - radius / 2; // correction to center on the tip of the mouse, by why? (Thought for another time.)
            } else {
                x = radius * radius_factor * Math.cos(angle) + origin_joystick.x; // consider the outer circle
                y = radius * radius_factor * Math.sin(angle) + origin_joystick.y; // consider the outer circle
            }

            // calculate the speed (radial coordinate) in percentage [0;100]
            var speed = Math.round(100 * Math.sqrt(Math.pow(x - origin_joystick.x, 2) + Math.pow(y - origin_joystick.y, 2)) / (radius * radius_factor)); // consider the outer circle
            if (speed > 100){
                speed = 100; // limit
            }

            // convert the angle to degrees [0;360]
            if (Math.sign(angle) == - 1) {
                angle_in_degrees = Math.round( - angle * 180 / Math.PI);
            }
            else {
                angle_in_degrees = Math.round(360 - angle * 180 / Math.PI);
            }

            // update the elements
            joystick(x, y);
            document.getElementById("speed").innerText = speed;
            document.getElementById("angle").innerText = angle_in_degrees;
            // send the data
            if((Date.now() - last_update) > 100){
                last_update = Date.now(); // update
                send_joystick(speed, angle_in_degrees);
            }
            movimento = 1;
        }
    }
</script>
</body>
</html>
)rawliteral";
   
// --------------------------------------------------
// Pagina web (ocupado)

const char html_busy[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>
        RoboCore Joystick
    </title>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />

    <style>

        html, body {width: 100%; height: 100%; padding: 0; margin: 0; }

        body {
            overflow: hidden;
            -moz-user-select: none; 
            -webkit-user-select: none;
            -ms-user-select:none; 
            user-select:none;
            -o-user-select:none;
        }

        .container {
            height: 26px;
            width: 50px;
            position: relative;
        }

        .container * {
            position: absolute;
        }
    </style>
</head>

<body style="height: 100%;  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif ;">
    <div style="line-height: 26px; background-color: black; padding: 10px; padding-bottom: 0px;">
        <div style="width: 100%; border: 0px solid red; text-align: center;">
            <svg version="1.0" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"

            height="30px" viewBox="0 0 1420 225" xml:space="preserve">
            <g id="Layer_1" fill="#f0be00">
                <g>
                    <path id="Robo" fill-rule="evenodd" clip-rule="evenodd" d="M175.7,94.9c0,11.7-6.2,20.2-18.5,25.6c-9.8,4-21,5.9-33.8,5.9H84.5
                    l78.8,33.5c4.2,1.8,6.3,4.6,6.3,8.2c0,5.3-4.6,8-13.9,8c-3.7,0-6.6-0.3-8.7-0.8l-107.6-46v38.4c0,5.6-2.8,8.4-8.3,8.4
                    c-5.9,0-9.3-0.3-10.2-0.7c-3.4-1.1-5.1-3.6-5.1-7.7V54.8c0-5.6,2.8-8.3,8.3-8.3h95c14.7,0,26.8,1.9,36.3,5.7
                    c13.5,5.5,20.3,14.5,20.3,27V94.9L175.7,94.9z M152.1,95.1V78.7c0-8.7-11.1-13-33.1-13H39.4v41.5h83.8
                    C142.4,107.2,152.1,103.2,152.1,95.1L152.1,95.1z M343.9,141.2c0,23.2-20,34.7-60,34.7h-36.8c-16.2,0-29-2.1-38.3-6.2
                    c-14.3-5.8-21.5-15.3-21.5-28.5V81.6c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9c13.7,5.3,20.6,15.4,20.6,30.2
                    V141.2L343.9,141.2z M320.3,140.6V82.1c0-10.9-11.9-16.4-35.8-16.4H247c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8
                    h37.5C308.4,156.5,320.3,151.2,320.3,140.6L320.3,140.6z M512.2,141.5c0,12.4-5.9,21.6-17.6,27.5c-9.1,4.6-20.5,6.9-34.2,6.9
                    H360.2c-5.5,0-8.3-2.8-8.3-8.4V54.8c0-5.5,2.8-8.3,8.3-8.3H459c8.8,0,16.9,0.7,24.2,2.1c7.9,1.9,14.5,5.4,20,10.5
                    c6,5.7,9,12.4,9,19.9c0,7.4-0.3,12.7-0.8,16c-0.1,0.6-0.6,1.7-1.4,3.2c-0.8,1.5-1.9,3.3-3.2,5.6c-2.6,4.5-3.9,7-3.9,7.5
                    c0,0.4,1.3,2.6,3.9,6.7c1.3,2.1,2.4,3.8,3.2,5.2c0.8,1.4,1.2,2.5,1.4,3.3c0.3,1.1,0.5,2.9,0.6,5.4
                    C512.1,134.4,512.2,137.6,512.2,141.5L512.2,141.5z M489.2,90.1c0-2.4-0.2-5.9-0.7-10.8c-0.7-9-10.9-13.5-30.6-13.5H376v36.3h82.8
                    C479.1,102.1,489.2,98.1,489.2,90.1L489.2,90.1z M488.5,134.9c0-8.9-9.8-13.4-29.2-13.4h-83.8v35h82.2c19,0,29.3-4.1,30.6-12.2
                    c0.1-0.4,0.1-1.4,0.2-3C488.5,139.7,488.5,137.6,488.5,134.9L488.5,134.9z M676.8,141.2c0,23.2-20,34.7-60,34.7H580
                    c-16.2,0-29-2.1-38.3-6.2c-14.3-5.8-21.5-15.3-21.5-28.5V81.6c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9
                    c13.7,5.3,20.6,15.4,20.6,30.2V141.2L676.8,141.2z M653.1,140.6V82.1c0-10.9-11.9-16.4-35.8-16.4h-37.4
                    c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8h37.5C641.2,156.5,653.1,151.2,653.1,140.6z"/>

                    <polygon id="Bolt" fill-rule="evenodd" clip-rule="evenodd" points="724,11.6 765,11.6 726,104.8 779.6,76.9 750.7,189.8 
                    765,182.8 733.6,230.4 722,172.8 732.9,184 744.9,111.7 689.2,140.7      "/>

                    <path id="Core" fill-rule="evenodd" clip-rule="evenodd" d="M935.5,57.4c0,5.6-2.8,8.4-8.3,8.4h-84c-20.4,0-30.6,4.1-30.6,12.2
                    v64.1c0,9.6,11.3,14.4,33.9,14.4h80.2c5.8,0,8.6,3.2,8.6,9.7c0,6.5-2.9,9.8-8.6,9.8h-80.2c-10.8,0-18.6-0.4-23.4-1.3
                    c-5.2-1-10.6-2.9-16.1-5.7c-6.4-3.3-10.8-6.8-13.4-10.4c-3-4.1-4.6-9.5-4.6-16.1V78.1c0-8.2,3.7-15.1,11.1-20.8
                    c6.2-4.8,13.8-7.9,22.7-9.6c2.3-0.4,5-0.6,8.3-0.8c3.3-0.2,7.1-0.3,11.6-0.3h84.5C932.7,46.5,935.5,50.2,935.5,57.4L935.5,57.4z
                    M1101.8,141.2c0,23.2-20,34.7-60,34.7H1005c-16.2,0-29-2.1-38.3-6.2c-14.3-5.8-21.5-15.3-21.5-28.5V81.6
                    c0-23.4,19.9-35.1,59.8-35.1h36.8c18.6,0,31.8,1.6,39.4,4.9c13.7,5.3,20.6,15.4,20.6,30.2V141.2L1101.8,141.2z M1078.1,140.6V82.1
                    c0-10.9-11.9-16.4-35.8-16.4h-37.4c-24.1,0-36.1,5.4-36.1,16.4v58.5c0,10.6,12,15.8,36.1,15.8h37.5
                    C1066.2,156.5,1078.1,151.2,1078.1,140.6L1078.1,140.6z M1267.8,94.9c0,11.7-6.2,20.2-18.5,25.6c-9.8,4-21,5.9-33.8,5.9h-38.9
                    l78.8,33.5c4.2,1.8,6.3,4.6,6.3,8.2c0,5.3-4.6,8-13.9,8c-3.7,0-6.6-0.3-8.7-0.8l-107.6-46v38.4c0,5.6-2.8,8.4-8.3,8.4
                    c-5.9,0-9.3-0.3-10.2-0.7c-3.4-1.1-5.1-3.6-5.1-7.7V54.8c0-5.6,2.8-8.3,8.3-8.3h95c14.7,0,26.8,1.9,36.3,5.7
                    c13.5,5.5,20.3,14.5,20.3,27V94.9L1267.8,94.9z M1244.2,95.1V78.7c0-8.7-11.1-13-33.1-13h-79.6v41.5h83.8
                    C1234.6,107.2,1244.2,103.2,1244.2,95.1L1244.2,95.1z M1416.5,111.2c0,6.9-2.9,10.3-8.8,10.3h-113.6v21.3
                    c0,9.1,10.1,13.7,30.3,13.7h83.3c5.5,0,8.3,2.8,8.3,8.4c0,7.4-2.8,11.1-8.3,11.1h-84.4c-12.3,0-23.6-2.2-33.9-6.6
                    c-12.6-5.7-18.9-14.3-18.9-25.7V79.5c0-6.2,2.1-11.8,6.3-16.8c9.1-10.8,25.3-16.1,48.6-16.1h23.1c5.4,0,8.2,3.6,8.2,10.8
                    c0,5.6-2.8,8.4-8.2,8.4h-3.4h-2.1h-1.2c-0.8,0-1.2,0-1.3-0.1c-1.8-0.1-3.3-0.1-4.6-0.1c-1.3,0-2.3-0.1-3.2-0.1
                    c-25.9,0-38.8,4.7-38.8,14v22.6h113.9C1413.6,102.1,1416.5,105.1,1416.5,111.2z"/>

                </g>
            </g>
        </svg>
    </div>
</div>

<div style="display: table; width:100%; height: calc(100% - 80px); border: 0px solid green;">
    <div style="padding: 10px; background-color: yellow; text-align: center;">Outro usuário já está conectado neste robô</div>
</div>
</body>

</html>
)rawliteral";

// --------------------------------------------------
// Prototipos

void configurar_servidor_web(void);
void handleWebSocketMessage(uint32_t, void *, uint8_t *, size_t);
void onEvent(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType,
             void *, uint8_t *, size_t);

// --------------------------------------------------
// --------------------------------------------------

void setup(){
  // configura a comunicacao serial
  Serial.begin(115200);
  Serial.println("RoboCore - Vespa Rocket + RoboARM");
  Serial.println("\t(v1.1 - 26/03/2022)\n");

  // configura o LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // configura os servos
  servos[0].attach(VESPA_SERVO_S1, SERVO_MIN, SERVO_MAX);
  servos[1].attach(VESPA_SERVO_S2, SERVO_MIN, SERVO_MAX);
  servos[2].attach(VESPA_SERVO_S3, SERVO_MIN, SERVO_MAX);
  servos[3].attach(VESPA_SERVO_S4, SERVO_MIN, SERVO_MAX);
  // atualiza os motores para as posicoes iniciais
  for(uint8_t i=0 ; i < 4 ; i++){
    servos[i].write(sangulos[i].atual);
  }

  // configura o ponto de acesso (Access Point)
  Serial.print("Configurando a rede Wi-Fi... ");
  const char *mac = WiFi.macAddress().c_str(); // obtem o MAC
  char ssid[] = "Vespa-xxxxx"; // mascara do SSID (ate 63 caracteres)
  char *senha = "robocore"; // senha padrao da rede (no minimo 8 caracteres)
  // atualiza o SSID em funcao do MAC
  for(uint8_t i=6 ; i < 11 ; i++){
    ssid[i] = mac[i+6];
  }
  WiFi.mode(WIFI_AP);
  if(!WiFi.softAP(ssid, senha)){
    Serial.println("ERRO");
    // trava a execucao
    while(1){
      digitalWrite(PIN_LED, HIGH);
      delay(100);
      digitalWrite(PIN_LED, LOW);
      delay(100);
    }
  }
  Serial.println("OK");
  Serial.printf("A rede \"%s\" foi gerada\n", ssid);
  Serial.print("IP de acesso: ");
  Serial.println(WiFi.softAPIP());

  // configura e iniciar o servidor web
  configurar_servidor_web();
  server.begin();
  Serial.println("Servidor iniciado\n");
}

// --------------------------------------------------

void loop() {
  // le a tensao da bateria e envia para o cliente
  if(millis() > timeout_vbat){
    // le a tensao da bateria
    uint32_t tensao = vbat.readVoltage();

    // verificar se a tensao esta critica
    if((tensao < 7000) && (vbat_critico == 0xFF)){
      Serial.printf("Tensao critica (%u mV)\n", tensao);
      vbat_critico = LOW;
      digitalWrite(PIN_LED, vbat_critico);
      timeout_led_vbat = millis() + INTERVALO_LED_VBAT_LOW;
    } else if((tensao >= 7000) && (vbat_critico < 0xFF)){
      vbat_critico = 0xFF; // reset
      // atualiza o estado do LED em funcao da conexao ativa
      if(ws.count() > 0){
        digitalWrite(PIN_LED, HIGH);
      } else {
        digitalWrite(PIN_LED, LOW);
      }
    }
    
    // atualiza se houver clientes conectados
    if(ws.count() > 0){
      // cria a mensagem
      const int json_tamanho = JSON_OBJECT_SIZE(1); // objeto JSON com um membro
      StaticJsonDocument<json_tamanho> json;
      json[ALIAS_VBAT] = tensao;
      size_t mensagem_comprimento = measureJson(json);
      char mensagem[mensagem_comprimento + 1];
      serializeJson(json, mensagem, (mensagem_comprimento+1));
      mensagem[mensagem_comprimento] = 0; // EOS (mostly for debugging)
  
      // send the message
      ws.textAll(mensagem, mensagem_comprimento);
      Serial.printf("Tensao atualizada: %u mV\n", tensao);
    }
    
    timeout_vbat = millis() + INTERVALO_ATUALIZACAO_VBAT; // atualiza
  }

  // pisca o LED se a tensao estiver critica
  if(millis() > timeout_led_vbat){
    if(vbat_critico < 0xFF){
      if(vbat_critico == LOW){
        vbat_critico = HIGH;
        timeout_led_vbat = millis() + INTERVALO_LED_VBAT_HIGH;
      } else {
        vbat_critico = LOW;
        timeout_led_vbat = millis() + INTERVALO_LED_VBAT_LOW;
      }
      digitalWrite(PIN_LED, vbat_critico);
    }
  }

  // verifica se e para parar os motores porque nao ha clientes conectados
  if(millis() > timeout_desconexao){
    if((ws.count() == 0) && habilitar_reset_motores){
      Serial.println("Reset dos motores");
      motores.stop();
      // atualiza os motores para as posicoes iniciais
      for(uint8_t i=0 ; i < 4 ; i++){
        servos[i].write(sangulos[i].atual);
      }
      habilitar_reset_motores = false; // reset
    }
    
    timeout_desconexao = millis() + INTERVALO_ATUALIZACAO_DESCONEXAO; // atualiza
  }
}

// --------------------------------------------------
// --------------------------------------------------

// Configurar o servidor web
void configurar_servidor_web(void) {
  ws.onEvent(onEvent); // define o manipulador do evento do WebSocket
  server.addHandler(&ws); // define o manipulador do WebSocket no servidor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ // define a resposta da pagina padrao
    if(ws.count() == 0){
      request->send_P(200, "text/html", html_index);
    } else {
      request->send_P(200, "text/html", html_busy);
    }
  });
}

// --------------------------------------------------

// Manipulador para mensagens WebSocket
//  @param (client) : ID do cliente [uint32_t]
//         (arg) : xxx [void *]
//         (data) : xxx [uint8_t *]
//         (length) : xxx [size_t]
void handleWebSocketMessage(uint32_t client, void *arg, uint8_t *data, size_t length) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == length && info->opcode == WS_TEXT) {
    data[length] = 0;

    // verifica se e para controlar os motores
    if(strstr(reinterpret_cast<char*>(data), ALIAS_VELOCIDADE) != nullptr){
      // cria um documento JSON
      const int json_tamanho = JSON_OBJECT_SIZE(2); // objeto JSON com dois membros
      StaticJsonDocument<json_tamanho> json;
      DeserializationError erro = deserializeJson(json, data, length);
      
      // extrai os valores do JSON
      int16_t angulo = json[ALIAS_ANGULO]; // [0;360]
      int16_t velocidade = json[ALIAS_VELOCIDADE]; // [0;100]

      // debug
      Serial.print("Velocidade: ");
      Serial.print(velocidade);
      Serial.print(" | Angulo: ");
      Serial.println(angulo);

      // atualiza os motores
   
      //curva frente para a esquerda
      if((angulo >= 90) && (angulo <= 180)){
        motores.turn(velocidade * (135 - angulo) / 45 , velocidade);

      //curva frente para a direita  
      } else if((angulo >= 0) && (angulo < 90)){
        motores.turn(velocidade, velocidade * (angulo - 45) / 45);

      //curva tras esquerda   
      } else if((angulo > 180) && (angulo <= 270)){
        motores.turn(-1 * velocidade, -1 * velocidade * (angulo - 225) / 45);

      //curva tras direita   
      } else if(angulo > 270){
        motores.turn(-1 * velocidade * (315 - angulo) / 45, -1 * velocidade);

      } else {
        motores.stop();
      }
   
    }
    // verifica se e para controlar um servo motor
    else if(strstr(reinterpret_cast<char*>(data), ALIAS_SERVO) != nullptr){
      // cria um documento JSON
      const int json_tamanho = JSON_OBJECT_SIZE(2); // objeto JSON com dois membros
      StaticJsonDocument<json_tamanho> json;
      DeserializationError erro = deserializeJson(json, data, length);

      // extrai os valores do JSON
      int16_t angulo = json[ALIAS_POSICAO]; // [0;180]
      int16_t servo = json[ALIAS_SERVO]; // [1-4]

      // debug
      Serial.print("Servo: ");
      Serial.print(servo);
      Serial.print(" | Angulo: ");
      Serial.println(angulo);

      // verifica os valores
      if((servo < 1) && (servo > 4)){
        Serial.printf("Servo invalido (%u)\n", servo);
        return;
      }
      if((angulo < sangulos[servo-1].min) && (angulo > sangulos[servo-1].max)){
        Serial.printf("Angulo invalido (%u)\n", servo);
        return;
      }

      // atualiza o servo
      servos[servo-1].write(angulo);
    }
    // dados invalidos
    else {
      Serial.printf("[%i] Recebidos dados invalidos (%s)\n", client, data);
    }
  } else {
    Serial.printf("[%i] Frame invalido: [%i]\n", client, info->opcode);
  }
}

// --------------------------------------------------

// Manipulador dos eventos do WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t length) {
  switch (type) {
    case WS_EVT_CONNECT: {
      digitalWrite(PIN_LED, HIGH); // acende o LED
      // permitir apenas um cliente conectado
      if(ws.count() == 1){ // o primeiro cliente ja e considerado como conectado
        Serial.printf("Cliente WebSocket #%u conectado de %s\n", client->id(), client->remoteIP().toString().c_str());
      } else {
        Serial.printf("Cliente WebSocket #%u de %s foi rejeitado\n", client->id(), client->remoteIP().toString().c_str());
        ws.close(client->id());
      }
      break;
    }
    case WS_EVT_DISCONNECT: {
      if(ws.count() == 0){
        digitalWrite(PIN_LED, LOW); // apaga o LED
      }
      Serial.printf("Cliente WebSocket #%u desconectado\n", client->id());
      break;
    }
    case WS_EVT_DATA: {
      handleWebSocketMessage(client->id(), arg, data, length);
      break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
