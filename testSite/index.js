function update_tab() {
  //if (e.preventDefault) e.preventDefault();
  let tab = window.location.hash.substring(1);
  var i;
  var x = document.getElementsByClassName("rfx-tab");
  console.log(tab)

  for (i = 0; i < x.length; i++) {
    x[i].style.display = "none"
    if (x[i].id === tab)
      document.getElementById(tab).style.display = "block"
  }
  if (tab.length == 0) {
    document.getElementById("dashboard").style.display = "block"
  }
}
window.addEventListener('hashchange', update_tab);
//window.onhashchange = update_tab
function update_status() {
  var xhr = new XMLHttpRequest();
  xhr.onload = () => {
    let response = JSON.parse(xhr.responseText)
    let status = response.status;
    let nodes = document.querySelectorAll("#status-bits > .led");
    for (var i = 0; i < nodes.length; i++) {
      if (status[i] == '0')
        nodes[i].style.background = "rgb(100,100,100)";
      else
        nodes[i].style.background = "rgb(100,255,100)";
    }
    setTimeout(update_status, 500);
  };
  xhr.onerror = () => {
    setTimeout(update_status, 500);
    //update_server_status("warning");
    //document.getElementById("server_ping").innerText = "error";
  }
  xhr.timeout = 2000;
  xhr.ontimeout = () => {
    setTimeout(update_status, 500);
    //update_server_status("warning");
    //document.getElementById("server_ping").innerText = "timeout";
  }
  xhr.open("GET", "http://192.168.4.62/engine/status", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send();
}
setTimeout(update_status, 500)
setTimeout(update_tab, 100)

function index_on_load() {
  console.log('hi');
}

if (window.addEventListener) { // W3C standard{
  window.addEventListener('load', index_on_load, false); // NB **not** 'onload'
}
else if (window.attachEvent) { // Microsoft
  window.attachEvent('onload', index_on_load);
}