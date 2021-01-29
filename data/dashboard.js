// Purely a debug option
const rfx_wifi_modal_inner_text = /*html*/`
    <!--<link rel="stylesheet" type="text/css" href="styles.css">-->
    <div class="rfx-modal" style="display:none">
        <div class="rfx-modal-content rfx-panel">
            <div id="modal-header" class="dark-gray2">
                <Label id="header-label">WiFi Settings</Label>
                <div id="close-button" class="rfx-clickable rfx-display-right" onclick="this.parentNode.parentNode.parentNode.style.display='none'">&times;</div>
            </div>
            <div id="modal-body">
                <label><b>Name on network</b></label>
                <input id="dns_name" class="rfx-input" type="text" placeholder="Access server via name" style="width:100%">
                <label><b>SSID</b></label>
                <input id="ssid" class="rfx-input" type="text" placeholder="Wifi Network Name" style="width:100%">
                <label><b>Password</b></label>
                <input id="password" class="rfx-input" type="password" placeholder="Wifi Password" style="width:100%">
                <button id="submit-button" class="rfx-clickable" style="width:100%;">Submit</button>
            </div>
        </div>
    </div>
    `
const rfx_firmware_modal_inner_text = /*html*/`
    <!--<link rel="stylesheet" type="text/css" href="styles.css">-->
    <div class="rfx-modal" style="display:none">
        <div class="rfx-modal-content rfx-panel">
            <div id="modal-header" class="dark-gray2">
                <Label id="header-label">Firmware</Label>
                <div id="close-button" class="rfx-clickable rfx-display-right" onclick="this.parentNode.parentNode.parentNode.style.display='none'">&times;</div>
            </div>
            <div id="modal-body">
                <div>
                    <button id="OTA_file_select" class="rfx-clickable" style="width:100%">
                    <input id="fileinput" type='file' name='update'>Select Firmware for Update</button>
                    <div id= "progress_container" class="progress" style="margin-top:8px; text-align: center; position: relative; height:26px; background-color: lightgray;">
                        <div id="progress" style="position: absolute; background-color: rgb(74, 128, 172); width:0%; height:100% "></div>
                        <div id="progress_label" style="position: absolute; top:0px; width:100%;text-align: center;"></div>
                    </div>
                    <button id="OTA_Execute" class="rfx-clickable disabled" style="margin-top:8px; width:100%">Upload</button>
                </div>
            </div>
        </div>
    </div>
    `
const rfx_console_inner_text = /*html*/`
    <html>
    <!--<link rel="stylesheet" type="text/css" href="styles.css">-->
        <div class="rfx-panel">
            <div id="panel-header" class="rfx-warning" style="height:40px;">
                <div class="rfx-display-left">
                    <label>Console</label>
                </div>
                <div class="rfx-display-right">
                    <button id="scroll_console" class="rfx-clickable blue5 rfx-active">Auto Scroll</button>
                    <button id="clr_console" class="rfx-clickable blue5">Clear</button>
                </div>
            </div>
            <textarea  readonly  id="console_view" class="rfx-input rfx-console green" rows="10" cols="50" onchange="scroll()" style="resize: none; ">
            </textarea>
            <div style="padding:0 4px 0 4px; height:38px">
                <div style="width:90px; float:right">
                    <button class="rfx-clickable blue5" id="code_button" style="height: 32px; width:100%;">Send</button>  
                </div>
                <div style="height:32px; margin-right: 100px">
                    <input id="code" class="rfx-console orange" type="text" placeholder="code to send to machine" style="width:100%; font-size:1.5rem">
                </div>                              
            </div>
        </div>
    </html>
    `
// Map the above content to HTML classes    
let keys = ["rfx-wifi-modal", "rfx-firmware-modal", "rfx-console"]
let values = [rfx_wifi_modal_inner_text, rfx_firmware_modal_inner_text, rfx_console_inner_text]

const rfx_dashboard_inner_text = /*html*/`
    <rfx-wifi-modal id="rfx-wifi-modal"></rfx-wifi-modal>
    <rfx-firmware-modal id="rfx-firmware"></rfx-firmware-modal>
    <link rel="stylesheet" type="text/css" href="styles.css">
    <div id="dashboard" class="rfx-tab" style="max-width:1200px; margin: auto; ">
    <div style=" display:flex; flex-wrap:wrap; margin: -4px">
        <div class="rfx-panel" style="width: 350px; min-width: 350px; flex: 3; margin:4px">
        <div id="network-status-bar" class="dark-gray1" style="height:32px">
            <label>WiFi Connection</label>
            <div id="wifi-connection-settings-button" class="rfx-clickable rfx-display-right"
            style="height:24px; padding:0 10px 0 10px">&#9881;</div>
        </div>
        <div>
            <div class="rfx-nutrition">
            <div>
                <label><b>IP / Name: </b></label>
                <div class="rfx-display-right">
                <label id="ip" style="width:auto">NULL</label>
                <label style="padding:0 4px 0 4px"> - </label>
                <label id="dns_name">NULL</label>
                </div>
            </div>
            <div>
                <label><b>Mode: </b></label>
                <label class="rfx-display-right" id="mode">NULL</label>
            </div>
            <div>
                <label><b>WiFi SSID: </b></label>
                <label id="ssid" class="rfx-display-right">NULL</label>
            </div>
            <div>
                <label><b>Ping (msec): </b></label>
                <label class="rfx-display-right"><b id="server_ping" style="filter: brightness(80%);">NULL</b></label>
            </div>
            </div>
        </div>
        </div>
        <div class="rfx-panel" style="width: 350px; min-width: 350px; flex: 3; margin:4px">
        <div class="dark-gray1">
            <label>CPU</label>
        </div>
        <div class="rfx-nutrition">
            <div>
            <label><b>CPU Freq (MHz): </b></label>
            <label id="freq" class="rfx-display-right">NULL</label>
            </div>
            <div>
            <label><b>Up Time (sec): </b></label>
            <label id="up_time" class="rfx-display-right">NULL</label>
            </div>
            <div>
            <label><b>Filesystem Used/Total (kb): </b></label>
            <div class="rfx-display-right">
                <label id="used_kbytes">NULL</label>
                <label style="padding:0 4px 0 4px"> / </label>
                <label id="total_kbytes">NULL</label>
            </div>
            </div>
            <div>
            <label><b>RAM Used/Total(kb): </b></label>
            <div class="rfx-display-right">
                <label id="used_heap">NULL</label>
                <label style="float:right; padding:0 4px 0 4px"> / </label>
                <label id="total_heap">NULL</label>
            </div>
            </div>
        </div>
        </div>
        <div class="rfx-panel" style="width: 350px; min-width: 350px; flex: 3; margin:4px">
        <div class="dark-gray1">
        <label>System</label>          
        <div id="system-settings-button" class="rfx-clickable rfx-display-right"
            style="height:24px; padding:0 10px 0 10px">&#9881;</div>
        </div>
        <div class="rfx-nutrition">
        <div>
            <label><b>Model-Rev: </b></label>
            <div  class="rfx-display-right">
            <label id="model">NULL</label>
            <label style="float:right; padding:0 4px 0 4px">-</label>
            <label id="revision">NULL</label>
            </div>
        </div>
        <div>
            <label><b>Firmware-Vev: </b></label>
            <div  class="rfx-display-right">
            <label id="firmware" >NULL</label>
            <label style="float:right; padding:0 4px 0 4px">-</label>
            <label id="version">NULL</label>
            </div>
        </div>
        <div>
            <label><b>Cores: </b></label>
            <div id="cores"  class="rfx-display-right">NULL</div>
        </div>
        <div>
            <label><b>Sketch (kbytes): </b></label>
            <div  class="rfx-display-right">
            <label id="used_sketch_space" >NULL</label>
            <label style="float:right; padding:0 4px 0 4px">/</label>
            <label id="total_sketch_space">NULL</label>
            </div>
        </div>
        <div>
            <label><b>Pseudo Static Ram (kbytes): </b></label>
            <div  class="rfx-display-right">
            <label id="used_psram" >NULL</label>
            <label style="float:right; padding:0 4px 0 4px">/</label>
            <label id="total_psram">NULL</label>
            </div>
        </div>
        </div>
    </div>
        <div style="width:500px; min-width: 500px; flex: 3;">
        <rfx-console></rfx-console>
        </div>
    </div>
    </div>
    `

let current_ip = ""

if (window.location.hostname == "127.0.0.1")
    current_ip = "http://192.168.4.62"

let route_to_websocket = "ws://" + window.location.hostname + ":81"
if (!current_ip == "")
    route_to_websocket = "ws://" + current_ip.substr(7) + ":81"

let route_to_status     = current_ip + "/server/status"
let route_to_ping       = current_ip + "/server/ping"
let route_to_settings   = current_ip + "/server/settings"
let route_to_update     = current_ip + "/update"

let caution_style = get_css_style_from_class("rfx-caution")
let warning_style = get_css_style_from_class("rfx-warning")
let good_style = get_css_style_from_class("rfx-good")

//const shadowRoot = this.attachShadow({ mode: 'open' });
function get_css_style_from_class(class_name) {
    const parent = document.createElement('div');
    parent.classList.add(class_name);
    document.body.appendChild(parent);
    let style = { ...getComputedStyle(parent) } // Make a deep copy
    parent.remove();
    return style;
}
function mark(el, style) {
    if (!el)
        return
    el.style.backgroundColor = style.backgroundColor
    el.style.color = style.color
}

function run_after_load() {
    document.getElementById("main-view").innerHTML += /*html*/`
        <rfx-dashboard id="dashboard" class="rfx-tab"></rfx-dashboard>
    `
}

if (window.addEventListener) // W3C standard
    window.addEventListener('load', run_after_load, false); // NB **not** 'onload'
else if (window.attachEvent) // Microsoft
    window.attachEvent('onload', run_after_load);


class dashboard extends HTMLElement {
    constructor() {
        super();
        const shadowRoot = this.attachShadow({ mode: 'open' });

    }

    connectedCallback() {
        let root = this.shadowRoot;
        root.innerHTML = rfx_dashboard_inner_text

        // Add the custom elements to the shadowdom
        for(let i = 0; i < keys.length;i++){
            let elements = root.querySelectorAll(keys[i])
            elements.forEach(function(el){
                el.innerHTML = values[i]
            })
        }

        function update_server_status(status) {
            //let color = color_warning;
            let el = document.getElementById("network-status-bar");
            if (status == "ok")
                mark(el, good_style)
            else if (status == "caution")
                mark(el, caution_style)
            else {
                mark(el, warning_style)
                root.querySelectorAll("rfx-console").forEach(function (el) {
                    el.setAttribute("wifi", "false")
                })
            }
        }
        //Ping
        let ping_error_count = 0
        let block_request = false
        let pingTime = new Date();
        let statusTime = new Date();
        setInterval(() => {
            let currentTime = new Date();
            if(currentTime.getTime()-pingTime.getTime()>=1000)
                ping();
            else if(currentTime.getTime()-statusTime.getTime()>=2000)
                getServerInfo();
        }, 500);
        function ping() {
            pingTime = new Date();
            var xhr = new XMLHttpRequest();
            let start_date = new Date();
            xhr.onload = () => {
                let end_date = new Date();
                let millis = end_date.getTime() - start_date.getTime();
                root.getElementById("server_ping").innerText = millis;
                if (millis < 250)
                    update_server_status("ok");
                else if (millis < 750)
                    update_server_status("caution");
                else
                    update_server_status("warning");
                root.getElementById("dashboard").classList.remove("rfx-disabled");
                ping_error_count = 0;
            };
            xhr.onerror = () => {
                ping_error_count++
                if (ping_error_count >= 2) {
                    update_server_status("warning");
                    root.getElementById("server_ping").innerText = "error";
                    root.getElementById("dashboard").classList.add("rfx-disabled");
                }
            }
            xhr.ontimeout = () => {
                update_server_status("warning");
                root.getElementById("server_ping").innerText = "timeout";
                root.getElementById("dashboard").classList.add("rfx-disabled");
            }
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    //setTimeout(ping, 1000);
                }
            };
            xhr.open("GET", route_to_ping, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.send();
        };

        function getServerInfo() {  
            statusTime = new Date();       
            var xhr = new XMLHttpRequest();
            let start_date = new Date();
            xhr.onload = () => {
                let response = JSON.parse(xhr.responseText)
                Object.entries(response).forEach(([key, value]) => {
                    let elements = root.querySelectorAll('#'+key);
                    elements.forEach(function(el){
                        if(el.nodeName == 'INPUT')
                            el.setAttribute("placeholder",value)
                        else
                            el.innerText = value;
                    })
                });
            };
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    //setTimeout(getServerInfo, 2000);
                }
            };
            xhr.open("GET", route_to_status, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.send();
        }
        // make the gear icons open the right modals
        root.getElementById("wifi-connection-settings-button").addEventListener('click', (event) => {
            root.querySelector("rfx-wifi-modal>.rfx-modal").style.display = "block"
        })
        root.getElementById("system-settings-button").addEventListener('click', (event) => {       
            root.querySelector("rfx-firmware-modal>.rfx-modal").style.display = "block"
        })

        // Wifi Modal Functionality
        let submit = root.getElementById("submit-button")
        let header = root.getElementById('modal-header')
        let label = root.getElementById('header-label');
        function submit_server(){
            if(block_request==true)
            {
                setTimeout(() => {
                    submit_server()
                }, 100);
                return
            }
            block_request = true;
            let data = {};
            // Build json data packet 
            root.querySelectorAll('.rfx-input').forEach(item => {
                data[item.id] = item.value;
            })
            var xhr = new XMLHttpRequest();
            xhr.onload = () => {
                if (xhr.response === 'ok') {
                    mark(submit, good_style)
                    label.innerText = "Wifi Settings: Saved"
                    mark(header, good_style)
                    root.querySelectorAll('.rfx-input').forEach(item => {
                        item.style.backgroundColor = ""
                        item.value = "";
                    })
                    block_request=false
                    setTimeout(getServerInfo(),1000);
                }
                else {
                    label.innerText = "Wifi Settings: Error- Bad Response";
                    mark(header, warning_style)
                    mark(submit, warning_style)
                }
            };
            xhr.onerror = () => {
                label.innerText = "Wifi Settings: Error- Server";
                mark(header, warning_style)
                mark(submit, warning_style)
            }
            xhr.ontimeout = () => {
                label.innerText = "Wifi Settings: Error- Server Unreachable";
                mark(header, warning_style)
                mark(submit, warning_style)
            }
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    block_request=false;
                }
            };
            xhr.timeout = 2000
            xhr.open("POST", route_to_settings, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.send(JSON.stringify(data));
        }
        submit.addEventListener('click', () => {
            submit_server()
        })

        root.querySelectorAll('.rfx-input').forEach(item => {
            console.log(item)
            item.addEventListener('change', event => {
                mark(item, caution_style)
                mark(submit, caution_style)
                mark(header, caution_style)
                label.innerText = "Wifi Settings: Need to submit"
            })
        })

        // Firmware Modal Functionality
        root.getElementById("OTA_file_select").addEventListener('click', () => {
            root.getElementById('fileinput').click()
        })
        root.getElementById("fileinput").addEventListener('change', (e) => {
            var fileName = e.target.files[0].name;
            root.getElementById('OTA_Execute').classList.remove('disabled');
            root.getElementById('progress_label').innerText = fileName
            root.getElementById('progress_container').style.backgroundColor = 'lightgray'
        })
        function OTA(){
            if(block_request==true){
                setTimeout(OTA(), timeout);
                return;
            }
            block_request = true;
            var xhr = new XMLHttpRequest();
            function done(msg) {
                root.getElementById('OTA_file_select').classList.remove('disabled');
                root.getElementById("progress_label").innerText = msg;
                root.getElementById("progress").style.width = 0;
                //getServerInfo();
            }
            xhr.onload = () => {
                if (xhr.response === 'ok') {
                    console.log(xhr.response);
                    console.log("restart recieved")
                    done("Firmware Update Successful");
                    root.getElementById("progress_container").style.backgroundColor = "lightGreen";

                }
                else {
                    console.log(xhr.response);
                    done("OTA Bad Response")
                    root.getElementById("progress_container").style.backgroundColor = "lightCoral";
                }
            };
            xhr.onerror = () => {
                console.log("restart error")
                done("Update Error");
                root.getElementById("progress_container").style.backgroundColor = "lightCoral";

            }
            xhr.ontimeout = () => {
                console.log("restart timeout")
                done("Server Timeout");
                root.getElementById("progress_container").style.backgroundColor = "lightCoral";
            }
            xhr.upload.addEventListener('progress', function (evt) {
                if (evt.lengthComputable) {
                    var per = evt.loaded / evt.total;
                    console.log(per);
                    root.getElementById("progress").style.width = Math.round(per * 100) + "%";
                }

            })
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    block_request=false;
                }
            };
            xhr.open("POST", route_to_update, true);
            let file = root.getElementById("fileinput").files[0];
            var formData = new FormData();
            formData.append("update", file, file.name);
            xhr.file = file;
            root.getElementById('fileinput').value = '';
            root.getElementById('OTA_Execute').classList.add('disabled');
            root.getElementById('OTA_file_select').classList.add('disabled');
            root.getElementById("progress_label").innerText = "Uploading..."
            xhr.send(formData);
            return false;
        }
        root.getElementById('OTA_Execute').addEventListener('click', event => {
            OTA();            
        })
        // Console element functionality
        var ws = null;
        function start() {
            if (ws)
                ws.close();
            ws = new WebSocket(route_to_websocket);
            ws.onopen = function () {
                console.log('connected!');
                root.getElementById("panel-header").classList.add("rfx-good")
                root.getElementById("panel-header").classList.remove("rfx-warning")
            };
            ws.onmessage = function (evt) {
                var received_msg = evt.data;
                root.getElementById("panel-header").classList.add("rfx-good")
                root.getElementById("panel-header").classList.remove("rfx-warning")
                if (received_msg)
                    root.getElementById("console_view").innerHTML += received_msg;
                let innerText = root.getElementById("console_view").innerHTML;
                if (innerText.length > 10000) {
                    innerText = innerText.substr(innerText.length - 10000);
                    root.getElementById("console_view").innerHTML = innerText;
                }
                var textarea = root.getElementById('console_view');
                if (root.getElementById("scroll_console").classList.contains("rfx-active")) {
                    textarea.scrollTop = textarea.scrollHeight;
                }
            };
            ws.onclose = function () {
                console.log('closed!');
                root.getElementById("panel-header").classList.add("rfx-warning")
                root.getElementById("panel-header").classList.remove("rfx-good")
            }
            ws.onerror = function (evt) {
                console.log("error: " + evt)
                root.getElementById("panel-header").classList.add("rfx-warning")
                root.getElementById("panel-header").classList.remove("rfx-good")
            }
        }

        function check() {
            if (!ws || ws.readyState == 3) {

                root.getElementById("panel-header").classList.add("rfx-warning")
                root.getElementById("panel-header").classList.remove("rfx-good")
                start();
            }
            else {

            }
        }

        start();
        setInterval(check, 5000);
        function sendcode() {
            block_request = true;
            if (!ws)
                return;
            console.log("value: " + root.getElementById("code").value)
            var xhr = new XMLHttpRequest();
            function bad_response() {

            }
            xhr.onload = () => {
                if (xhr.response === 'ok') {
                    console.log("code recieved")
                }
                else {
                    console.log("code bad response")
                }

            };
            xhr.onerror = () => {
                console.log("code error")
            }
            xhr.ontimeout = () => {
                console.log("code timeout")
            }
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    block_request=false;
                }
            };
            xhr.timeout = 2000
            var data = new FormData();
            data.append('code', root.getElementById("code").value);
            let t = root.getElementById("code").value;
            if (t.length > 0)
                ws.send(t);
            root.getElementById("code").value = "";
            //xhr.open("POST", "http://192.168.4.62/engine/api", true);
            //xhr.setRequestHeader("Content-Type", "text/html");
            //xhr.send(data);
        }
        //function update(){
        root.getElementById("code_button").addEventListener('click', event => { sendcode() })
        root.getElementById("clr_console").addEventListener('click', event => { root.getElementById("console_view").innerHTML = "" })
        root.getElementById("scroll_console").addEventListener('click', event => {
            let el = root.getElementById("scroll_console");
            if (el.classList.contains("rfx-active")) {
                el.classList.remove("rfx-active");
                el.classList.add("rfx-inactive");
                console.log("add");
            }
            else {
                el.classList.remove("rfx-inactive");
                el.classList.add("rfx-active");
                console.log("remove");

                var textarea = root.getElementById('console_view');
                textarea.scrollTop = textarea.scrollHeight;
            }

        })
        root.getElementById("code").addEventListener("keyup", function (event) {
            // Number 13 is the "Enter" key on the keyboard
            if (event.keyCode === 13) {
                // Cancel the default action, if needed
                event.preventDefault();
                // Trigger the button element with a click
                root.getElementById("code_button").click();
            }
        });
    }
    disconnectedCallback() {
    }
    static get observedAttributes() {
        return [];
    }
    attributeChangedCallback(name, oldValue, newValue) {

    }
    adoptedCallback() {
    }
}
customElements.define("rfx-dashboard", dashboard);







