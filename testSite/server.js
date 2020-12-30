import data_box from './dataBox.js'

let color_good = '#00a300'
let color_caution = '#e3a21a'
let color_warning = '#ee1111'

function run_after_load() {
  document.getElementById("header-right").innerHTML += /*html*/`
    <a id="nav_server" href="#server" class="w3-button"><span class="icon-database"></span></a>
    `


    
  document.getElementById("main-view").innerHTML += /*html*/`
    <!-- The Modal -->
    <div id="Network-settings" class="w3-modal">
    <div class="w3-modal-content" style="width:400px;">
      <header class="w3-container rfx-bar w3-black">
        <div onclick="document.getElementById('Network-settings').style.display='none'" 
        class="w3-button w3-display-topright">&times;</div>
        <div>Server Settings</div>
      </header>
      <div>
        <iframe src="networkSettings.html"  frameBorder="0" width=100%; height=235px;></iframe>
      </div>
      </div>
    </div>

    <div id="server" class="rfx-tab" style="display:none; max-width: 1000px; margin: auto;">

    <div class="w3-row-padding">
      <div class="rfx-card">
        <div class="rfx-bar w3-metro-darken">
          <div class="rfx-bar-item"><b>Network</b></div>
          <div id="server_LED" class="led "></div>
        </div>
        <div class="w3-row-padding">
          <div class="w3-half" style="padding-top: 16px;">
            <div class="w3-card">
              <div class="rfx-bar w3-metro-darken">
                <b class="rfx-bar-item">Status</b>
              </div>
              <table class="w3-table w3-bordered">
                <tr>
                  <td style="text-align: left;"><b>IP:</b></td>
                  <td id="server_ip" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>NAME:</b></td>
                  <td id="server_name" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>SSID:</b></td>
                  <td id="server_ssid" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>MODE:</b></td>
                  <td id="server_mode" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>PING (msec):</b></td>
                  <td id="server_ping" style="text-align: left;">NULL</td>
                </tr>
              </table>
              <button class="rfx-button" style="margin:8px" onclick="document.getElementById('Network-settings').style.display='block'">Edit Network Settings</button>
            </div>
          </div>
        </div>
      </div>
    </div>
  
  
  
    <div class="w3-row-padding">
      <div class="rfx-card">
        <div class="rfx-bar w3-metro-darken">
          <div class="rfx-bar-item"><b>Processor</b></div>
          <div id="server_LED" class="led "></div>
        </div>
        <div class="w3-row-padding">
          <div class="w3-half" style="padding-top: 16px;">
            <div class="w3-card">
              <div class="rfx-bar w3-metro-darken">
                <b class="rfx-bar-item">Status</b>
              </div>
              <table class="w3-table w3-bordered ">
                <tr>
                  <td style="text-align: left;"><b>CPU Freq (Mhz):</b></td>
                  <td id="server_freq" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>FREE RAM (kb):</b></td>
                  <td id="server_heap" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;"><b>UP TIME (sec):</b></td>
                  <td id="up_time" style="text-align: left;">NULL</td>
                </tr>
                <tr>
                  <td style="text-align: left;">
                    <div><b>FILESYSTEM MEMORY:</b></div><b>(used / total kb)</b>
                  </td>
                  <td>
                    <span id="used_bytes" style="text-align: left;"></span>
                    /
                    <span id="total_bytes" style="text-align: left;"></span>
                </tr>
              </table>
            </div>
          </div>
          <div class="w3-half" style="padding-top: 16px;">
            <div class="w3-card">
              <div class="rfx-bar w3-metro-darken">
                <b class="rfx-bar-item">Microcontroller</b>
                <div id="server_setting_msg" class="rfx-bar-item"></div>
              </div>
              <div class="w3-padding">
                <button id="restart_button" class="w3-metro-blue rfx-button">Restart</button>
                <div class="w3-card" style="margin-top:8px">
                    <div style="width:100%;background:black;color:white;padding:4px">Over the Air Update</div>
                    <div style="padding:8px">
                        <label id="OTA_file_select" class="custom-file-upload rfx-button w3-metro-blue">
                            <input id="fileinput" type='file' name='update'>
                            Choose File
                        </label>
                        <button id="OTA_Execute" class="w3-metro-blue rfx-button w3-disabled">Upload</button>
                    </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
    `
  document.getElementById('restart_button').addEventListener('click', event => {
    if (!confirm("Restart the ESP chip, server and all?"))
      return
    var xhr = new XMLHttpRequest();
    function bad_response() {

    }
    xhr.onload = () => {
      if (xhr.response === 'ok') {
        console.log("restart recieved")
      }
      else {
        console.log("restart bad response")
      }

    };
    xhr.onerror = () => {
      console.log("restart error")
    }
    xhr.ontimeout = () => {
      console.log("restart timeout")
    }
    xhr.timeout = 2000
    xhr.open("GET", "http://192.168.4.62/server/restart", true);
    xhr.setRequestHeader("Content-Type", "text/html");
    xhr.send();
  })
  document.getElementById('OTA_Execute').addEventListener('click', event => {
    var xhr = new XMLHttpRequest();
    xhr.onload = () => {
      if (xhr.response === 'ok') {
        console.log("restart recieved")
      }
      else {
        console.log("restart bad response")
      }

    };
    xhr.onerror = () => {
      console.log("restart error")
    }
    xhr.ontimeout = () => {
      console.log("restart timeout")
    }
    xhr.upload.addEventListener('progress', function (evt) {
      if (evt.lengthComputable) {
        var per = evt.loaded / evt.total;
        document.getElementById("prg").innerText = Math.round(per * 100) + "%";
      }
    })
    xhr.open("POST", "http://192.168.4.62/update", true);
    let file = document.getElementById("fileinput").files[0];
    var formData = new FormData();
    formData.append("update", file, file.name);
    xhr.file = file;
    xhr.send(formData);
    return false;
  })
}
//###### Poll server status and update display #######

function update_server_status(status) {
  let color = color_warning;
  if (status == "ok") {
    color = color_good
  }
  if (status == "caution") {
    color = color_caution
  }
  document.getElementById("server_LED").style.backgroundColor = color
  document.getElementById("nav_server").style.color = color
  document.getElementById("server_ping").style.color = color
}
function getServerInfo() {
  var xhr = new XMLHttpRequest();
  let start_date = new Date();
  xhr.onload = () => {
    let end_date = new Date();
    let millis = end_date.getTime() - start_date.getTime();
    let response = JSON.parse(xhr.responseText)
    document.getElementById("server_ip").innerText = response.ip;
    document.getElementById("server_name").innerText = response.dns_name;
    document.getElementById("server_ssid").innerText = response.ssid;
    document.getElementById("server_heap").innerText = Math.round(response.heap / 1000);
    document.getElementById("server_freq").innerText = response.freq;
    document.getElementById("server_mode").innerText = response.mode;
    document.getElementById("server_ping").innerText = (millis);

    document.getElementById("up_time").innerText = Math.round(response.up_time / 1000)
    document.getElementById("total_bytes").innerText = Math.round(response.total_bytes / 1000)
    document.getElementById("used_bytes").innerText = Math.round(response.used_bytes / 1000)

    if (millis < 250)
      update_server_status("ok");
    else if (millis < 750)
      update_server_status("caution");
    else
      update_server_status("warning");
    setTimeout(getServerInfo, 1000);
  };
  xhr.onerror = () => {
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "error";
    setTimeout(getServerInfo, 1000);
  }
  xhr.timeout = 2000;
  xhr.ontimeout = () => {
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "timeout";
    setTimeout(getServerInfo, 1000);
  }
  xhr.open("GET", "http://192.168.4.62/server/status", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
setTimeout(getServerInfo, 1000);
//window.onload.add(run_after_load);

//window.addEventListener('load',run_after_load,false);

if (window.addEventListener) // W3C standard
  window.addEventListener('load', run_after_load, false); // NB **not** 'onload'
else if (window.attachEvent) // Microsoft
  window.attachEvent('onload', run_after_load);
export { run_after_load }