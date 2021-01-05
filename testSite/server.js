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
      <div class="w3-modal-content w3-card-4 w3-border" style="width:400px;">
        <header class="w3-container rfx-bar w3-black">
          <div onclick="document.getElementById('Network-settings').style.display='none'" 
          class="w3-button w3-display-topright">&times;</div>
          <div>Server Settings</div>
        </header>
        <div>
          <iframe src="http://192.168.4.62/connect"  frameBorder="0" width=100%; height=300px;></iframe>

        </div>
      </div>
    </div>
    <div id="firmware-settings" class="w3-modal">
    <div class="w3-modal-content w3-card-4 w3-border" style="width:400px;">
      <header class="w3-container rfx-bar w3-black">
        <div onclick="document.getElementById('firmware-settings').style.display='none'" 
        class="w3-button w3-display-topright">&times;</div>
        <div></div>
      </header>
      <div>
        <iframe id="update-iframe" src="firmware.html"  frameBorder="0" width=100%; height=300px;></iframe>
      </div>
    </div>
  </div>
    <div id="server" class="rfx-tab" style="display:none; max-width: 1200px; margin: auto;">
    <div class="w3-row-padding" style=" margin:auto;">
    <div id="server-network"class="w3-third" style="min-width:350px !important;">
        <div class="rfx-panel" style="margin-bottom:8px">
            <label class="w3-metro-darken" style="height:32px;">                
                <label><b>Network Status</b></label>
                <div id="server_LED" class="led " style="margin-left: auto;"></div>
                <span class="icon-cog w3-button"
                onclick="document.getElementById('Network-settings').style.display='block'"></span>
                </label>
            <div style="border-bottom: 2px solid lightgray;">
                <label><b>IP / Name: </b></label>
                <label id="server_name" style="float:right">NULL</label>
                <label style="float:right; padding:0 4px 0 4px"> - </label>
                <label id="server_ip" style="float:right">NULL</label>
            </div>
            <div style="border-bottom: 2px solid lightgray;">
                <label><b>Mode: </b></label>
                <label id="server_mode" style="float:right">NULL</label>
            </div>
            <div style="border-bottom: 2px solid lightgray;">
                <label><b>WiFi SSID: </b></label>
                <label id="server_ssid" style="float:right">NULL</label>
            </div>
            <div style="border-bottom: 2px solid lightgray;">
                <label><b>Ping (msec): </b></label>
                <label style="float:right"><b id="server_ping"  style="filter: brightness(80%);">NULL</b></label>
            </div>
        </div>
    </div>
    <div id="server-status" class="w3-third" style="min-width:350px !important;">
      
    <div class="rfx-panel" style="margin-bottom:8px">
        <label class="w3-metro-darken" style="height:32px; vertical-align: middle;">                
          <b>Processor Status</b>
        </label>
        <div style="border-bottom: 2px solid lightgray;">
          <label><b>CPU Freq (MHz): </b></label>
          <label id="server_freq" style="float:right">NULL</label>
        </div>
        <div style="border-bottom: 2px solid lightgray;">
            <label><b>Up Time (sec): </b></label>
            <label id="up_time" style="float:right">NULL</label>
        </div>
        <div style="border-bottom: 2px solid lightgray;">
            <label><b>Filesystem Used/Total (kb): </b></label>
            <label id="total_bytes" style="float:right">NULL</label>
            <label style="float:right; padding:0 4px 0 4px"> / </label>
            <label id="used_bytes" style="float:right">NULL</label>
        </div>
        <div style="border-bottom: 2px solid lightgray;">
            <label><b>RAM Used/Total(kb): </b></label>
            <label id="server_total_heap" style="float:right">NULL</label>
            <label style="float:right; padding:0 4px 0 4px"> / </label>
            <label id="server_used_heap" style="float:right">NULL</label>
        </div>

      </div>
    </div>
    <div id="server-status" class="w3-third" style="min-width:350px !important;">
      <div class="rfx-panel" style="margin-bottom:8px">
        <label class="w3-metro-darken" style="height:32px; vertical-align: middle;">                
          <b>Control</b>
        </label>
        <div>
          <button id="restart_button" class="rfx-button">Restart</button>
          <button id="firmware_button" class="rfx-button">Firmware</button>
        </div>        
      </div>
    </div>
    <div id="server-filesystem" class="w3-half" style="min-width:350px !important;">
        <div class="w3-card" style="margin-bottom:8px">
            <div class="rfx-bar w3-metro-darken">
                <b class="rfx-bar-item">File Systems</b>
                <div id="server_setting_msg" class="rfx-bar-item"></div>
            </div>
            <div style="padding:0 8px 8px 8px">
              <div class="rfx-panel">
                <label>Internal</label>
                <div style="padding:8px">
                  <ul id="filelist" class="w3-ul w3-hoverable">
                  </ul>
                </div>
              </div>
              <div class="rfx-panel" style="margin-top:8px;">
                <label>External (SD)</label>
                <div style="padding:8px !important;">
                  
                </div>
              </div>
            </div>
        </div>
    </div>
</div>
    </div>
    `
  document.getElementById('firmware_button').addEventListener('click',event=>{
    document.getElementById('firmware-settings').style.display='block';
    let iframe = document.getElementById('update-iframe')
    var innerDoc = iframe.contentDocument || iframe.contentWindow.document;
    
    innerDoc.getElementById('header-label').style.display = "none"
          
  })
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
      document.getElementById('upload_complete').classList.remove("w3-disabled")
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
        document.getElementById("upload_percent").style.width = Math.round(per * 100) + "%";
      }
    })
    xhr.open("POST", "http://192.168.4.62/update", true);
    let file = document.getElementById("fileinput").files[0];
    var formData = new FormData();
    formData.append("update", file, file.name);
    xhr.file = file;
    xhr.send(formData);
    document.getElementById('upload_complete').classList.add("w3-disabled")
    document.getElementById('upload_in_progress').style.display='block';

    return false;
  })
  document.getElementById('fileinput').addEventListener('change',event=>{
    let file = document.getElementById("fileinput").files[0];
    document.getElementById('server_new_fw_name').innerText = file.name;
    document.getElementById('OTA_Execute').classList.remove('w3-disabled');
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
    document.getElementById("server_mode").innerText = response.mode;
    document.getElementById("server_ssid").innerText = response.ssid;
    document.getElementById("server_ping").innerText = millis;

    document.getElementById("up_time").innerText = Math.round(response.up_time / 1000)
    document.getElementById("total_bytes").innerText = Math.round(response.total_bytes / 1000)
    document.getElementById("used_bytes").innerText = Math.round(response.used_bytes / 1000)

    let total_heap = response.total_heap;
    let free_heap = response.free_heap;
    document.getElementById("server_total_heap").innerText = Math.round(total_heap / 1000);
    document.getElementById("server_used_heap").innerText = Math.round((total_heap-free_heap) / 1000);

    document.getElementById("server_freq").innerText = response.freq;
    if (millis < 250)
      update_server_status("ok");
    else if (millis < 750)
      update_server_status("caution");
    else
      update_server_status("warning");
    setTimeout(getServerInfo, 1000);
    document.getElementById("server").classList.remove("w3-disabled");
  };
  xhr.onerror = () => {
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "error";
    setTimeout(getServerInfo, 1000);
    document.getElementById("server").classList.add("w3-disabled");
  }
  xhr.timeout = 2000;
  xhr.ontimeout = () => {
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "timeout";
    setTimeout(getServerInfo, 1000);
    document.getElementById("server").classList.add("w3-disabled");
  }
  xhr.open("GET", "http://192.168.4.62/server/status", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
setTimeout(getServerInfo, 1000);
function download(file, text) { 
              
  //creating an invisible element 
  var element = document.createElement('a'); 
  element.setAttribute('href',  
  'data:text/plain;charset=utf-8, ' 
  + encodeURIComponent(text)); 
  element.setAttribute('download', file); 

  // Above code is equivalent to 
  // <a href="path of file" download="file name"> 

  document.body.appendChild(element); 

  //onClick property 
  element.click(); 

  document.body.removeChild(element); 
} 
function get_file_system(){
  var xhr = new XMLHttpRequest();
  xhr.onload = () => {
    let response = JSON.parse(xhr.responseText)
    let list = response.files;
    for(let i=0;i<list.length;i++){
      let el = document.createElement("li");
      let link = document.createElement("a");
      link.target = '_blank';
      link.href = "http://192.168.4.62"+list[i].name.substr(7);
      link.innerText = list[i].name.substr(7);
      link.classList.add("fill-link")
      el.appendChild(link)
      document.getElementById("filelist").appendChild(el);
      
    }
    document.getElementById("server-filesystem").classList.remove("w3-disabled");
  };
  xhr.onerror = () => {
    setTimeout(getServerInfo, 2000);
    document.getElementById("server-filesystem").classList.add("w3-disabled");
  }
  xhr.timeout = 2000;
  xhr.ontimeout = () => {
    setTimeout(getServerInfo, 2000);
    document.getElementById("server-filesystem").classList.add("w3-disabled");
  }
  xhr.open("GET", "http://192.168.4.62/fs/internal", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
setTimeout(get_file_system, 2000);

//window.onload.add(run_after_load);

//window.addEventListener('load',run_after_load,false);

if (window.addEventListener) // W3C standard
  window.addEventListener('load', run_after_load, false); // NB **not** 'onload'
else if (window.attachEvent) // Microsoft
  window.attachEvent('onload', run_after_load);
export { }