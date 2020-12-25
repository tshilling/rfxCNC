

let color_good = '#00a300'
let color_caution = '#e3a21a'
let color_warning = '#ee1111'
var tmpl = document.getElementById('comment-template');

function openTab(tabName) {
  var i;
  var x = document.getElementsByClassName("tab");
  for (i = 0; i < x.length; i++) {
    x[i].style.display = "none";
  }
  document.getElementById(tabName).style.display = "block";
}
function getServerInfo(){
  var xhr = new XMLHttpRequest();
  let start_date = new Date();
  xhr.onload = () => {
    let end_date = new Date();
    let millis = end_date.getTime() - start_date.getTime();

    let response = JSON.parse(xhr.responseText)
    document.getElementById("server_ip").innerText = response.IP;
    document.getElementById("server_ssid").innerText = response.SSID;
    document.getElementById("server_heap").innerText = Math.round(response.HEAP/1000);
    document.getElementById("server_freq").innerText = response.FREQ;
    document.getElementById("server_mode").innerText = response.MODE
    document.getElementById("server_ping").innerText = (millis);
    if(millis < 250)
      update_server_status("ok");
    else if(millis < 500)
      update_server_status("caution");
    else
      update_server_status("warning");
  };
  xhr.onerror =() =>{
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "error";
  }
  xhr.timeout=1000;
  xhr.ontimeout=()=>{
    update_server_status("warning");
    document.getElementById("server_ping").innerText = "timeout";
  }
  xhr.open("GET", "http://192.168.4.62/server/status", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
setInterval(() => {
  getServerInfo()
}, 2000);
function submitNetworkInfo()
{
  console.log("button was clicked!");
  var ssid = document.getElementById("ssid").value;
  var password = document.getElementById("password").value;
  var data = {ssid:ssid, password:password};

  var xhr = new XMLHttpRequest();
  xhr.onload = () => {
    if(xhr.response === 'ok'){
      document.getElementById('server_setting_save_icon').style.backgroundColor = color_good;
      document.getElementById('server_setting_msg').innerText = "Settings saved";
      document.getElementById('server_setting_msg').style.color = color_good;
    }
    else{
      document.getElementById('server_setting_save_icon').style.backgroundColor = color_warning;
      document.getElementById('server_setting_msg').innerText = "Error saving settings";
      document.getElementById('server_setting_msg').style.color = color_warning;
    }

  };
  xhr.onerror =() =>{
      document.getElementById('server_setting_save_icon').style.backgroundColor = color_warning;
      document.getElementById('server_setting_msg').innerText = "Server unreachable";
      document.getElementById('server_setting_msg').style.color = color_warning;
  }
  xhr.open("POST", "http://192.168.4.62/server/settings", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(JSON.stringify(data));
};
function submitEngineInfo()
{
  console.log("button was clicked!");
  //var ssid = document.getElementById("ssid").value;
  //var password = document.getElementById("password").value;
  var data = {machineName:document.getElementById("EC_Name").value};

  var xhr = new XMLHttpRequest();
  xhr.onload = () => {
    if(xhr.response === 'ok'){
      document.getElementById('engine_setting_save_icon').style.backgroundColor = color_good;
      document.getElementById('engine_setting_msg').innerText = "Config saved";
      document.getElementById('engine_setting_msg').style.color = color_good;
  
      resetInputColor(document.getElementById("engineConfigContainer"));
    console.log("ok")
    }
    else{
      document.getElementById('engine_setting_save_icon').style.backgroundColor = color_warning;
      document.getElementById('engine_setting_msg').innerText = "Error saving config";
      document.getElementById('engine_setting_msg').style.color = color_warning;
      console.log("not ok")
    }

  };
  xhr.onerror =() =>{
      document.getElementById('engine_setting_save_icon').style.backgroundColor = color_warning;
      document.getElementById('engine_setting_msg').innerText = "Server unreachable";
      document.getElementById('engine_setting_msg').style.color = color_warning;
    console.log("error")
  }
  xhr.ontimeout =() =>{
    document.getElementById('engine_setting_save_icon').style.backgroundColor = color_warning;
    document.getElementById('engine_setting_msg').innerText = "Server timeout";
    document.getElementById('engine_setting_msg').style.color = color_warning;
    console.log("timeout")
  }
  xhr.timeout = 1000;

  xhr.open("POST", "http://192.168.4.62/engine", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(JSON.stringify(data));
};
function update_server_status(status){
  let color = color_warning;
  if(status == "ok"){
    color = color_good
  }
  if(status == "caution"){
    color = color_caution
  }
  document.getElementById("server_LED").style.backgroundColor = color
  document.getElementById("nav_server").style.color = color
  document.getElementById("server_ping").style.color = color
}
function devalidateServerSettings(){
  document.getElementById('server_setting_save_icon').style.backgroundColor = color_caution;
  document.getElementById('server_setting_msg').innerText = "Settings changed, save required";
      document.getElementById('server_setting_msg').style.color = color_caution;
}
let engine_config
function resetInputColor(node){
  if(node.nodeName.toLowerCase()=="input" || node.nodeName.toLowerCase()=="select")
    node.style.backgroundColor = "#f1f1f1"
    if(node.classList)
      if(node.classList.contains("w3-check"))
        node.parentElement.style.backgroundColor = "#ffffff"
  let children = node.childNodes;
  for(var i=0; i<children.length; i++) {
    resetInputColor(children[i])
  }
}
function devalidateEngineConfig(input){
  document.getElementById('engine_setting_save_icon').style.backgroundColor = color_caution;
  document.getElementById('engine_setting_msg').innerText = "Settings changed, save required";
  document.getElementById('engine_setting_msg').style.color = color_caution;
  let id = input;
  console.log(id)

  input.style.backgroundColor = "#ffffcc"
}
function get_engine_settings(){
  var xhr = new XMLHttpRequest();
  xhr.onload = () => {
    engine_config = JSON.parse(xhr.responseText)
    if(!engine_config.hasOwnProperty('machineName'))
      engine_config.machineName = "none"
    if(!engine_config.hasOwnProperty('defaultUnits'))
      engine_config.defaultUnits = "mm"
    if(!engine_config.hasOwnProperty('axisCount'))
      engine_config.axisCount = 3
    if(!engine_config.hasOwnProperty('discrete')){ 
      engine_config.discrete = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.discrete[i] = false
      }
    }
    if(!engine_config.hasOwnProperty('axisId')){ 
      engine_config.axisId = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.axisId[i] = ""
      }
    }
    if(!engine_config.hasOwnProperty('follow')){ 
      engine_config.follow = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.follow[i] = ""
      }
    }
    if(!engine_config.hasOwnProperty('rotary')){ 
      engine_config.rotary = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.rotary[i] = false
      }
    }
    if(!engine_config.hasOwnProperty('min')){ 
      engine_config.min = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.min[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('max')){ 
      engine_config.max = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.max[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('home')){ 
      engine_config.home = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.home[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('homeNegative')){ 
      engine_config.homeNegative = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.homeNegative[i] = false
      }
    }
    if(!engine_config.hasOwnProperty('stepPin')){ 
      engine_config.stepPin = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.stepPin[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('dirPin')){ 
      engine_config.dirPin = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.dirPin[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('dirInvert')){ 
      engine_config.dirInvert = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.dirInvert[i] = false
      }
    }
    if(!engine_config.hasOwnProperty('stepsPerUnit')){ 
      engine_config.stepsPerUnit = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.stepsPerUnit[i] = 1
      }
    }
    if(!engine_config.hasOwnProperty('maxFeed')){ 
      engine_config.maxFeed = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.maxFeed[i] = 0
      }
    }
    if(!engine_config.hasOwnProperty('acceleration')){ 
      engine_config.acceleration = [engine_config.axisCount]
      for(i = 0; i < engine_config.axisCount;i++){
        engine_config.acceleration[i] = 0
      }
    }


    console.log(engine_config);
    document.getElementById("EC_Name").value = engine_config.machineName;
    document.getElementById("EC_Units").value = engine_config.defaultUnits;
    document.getElementById("EC_Axis").value = engine_config.axisCount;

    var temp = document.getElementById("axisTemplate");
    for(i = 0; i < engine_config.axisCount;i++){
      var clon = temp.content.cloneNode(true);
      clon.getElementById("axisLabel").innerText = "Axis - "+i
      clon.getElementById("axisId").value = engine_config.axisId[i]
      clon.getElementById("axisFollow").value = engine_config.follow[i]
      clon.getElementById("axisRotary").value = engine_config.rotary[i]
      clon.getElementById("axisDiscrete").value = engine_config.discrete[i]
      clon.getElementById("axisMin").value = engine_config.min[i]
      clon.getElementById("axisMax").value = engine_config.max[i]
      clon.getElementById("axisHome").value = engine_config.home[i]
      clon.getElementById("axisHomeNegative").value = engine_config.homeNegative[i]
      clon.getElementById("axisStepPin").value = engine_config.stepPin[i]
      clon.getElementById("axisDirPin").value = engine_config.dirPin[i]
      clon.getElementById("axisDirInvert").value = engine_config.dirInvert[i]
      clon.getElementById("axisStepsPerUnit").value = engine_config.stepsPerUnit[i]
      clon.getElementById("axisMaxFeed").value = engine_config.maxFeed[i]
      clon.getElementById("axisAcceleration").value = engine_config.acceleration[i]

      document.getElementById("engineConfigContainer").append(clon)
    }
    console.log(engine_config);
  };
  xhr.onerror =() =>{
    setTimeout(get_engine_settings, 1000);
  }
  xhr.timeout=1000;
  xhr.ontimeout=()=>{
    setTimeout(get_engine_settings, 1000);
  }
  xhr.open("GET", "http://192.168.4.62/engineConfig.json", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
window.onload = get_engine_settings;