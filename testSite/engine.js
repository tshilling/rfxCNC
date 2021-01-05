function run_after_load(){
    document.getElementById("header-right").innerHTML += /*html*/`
    <a id="nav_engine" href="#engine" class="w3-button"><span class="icon-cogs"></span></a>
    `
    document.getElementById("main-view").innerHTML += /*html*/`
    <!-- The Modal -->

    <div id="engine" class="rfx-tab" style="display:none; max-width: 1200px; margin: auto;">
        <div class="w3-row-padding" style=" margin:auto;">
            <div id="engine-status"class="w3-third" style="min-width:350px !important;">
                <div class="w3-card" style="margin-bottom:8px">
                    <div class="rfx-bar w3-metro-darken">
                        <b class="rfx-bar-item">Engine Status</b>
                        <div id="engine_LED" class="led "></div>
                    </div>
                </div>
                <div id="status_leds">
                    <label class="led" style="display:inline-block; width:16px; height:16px"></label><label>test</label>
                    <div class="led" style="display:inline-block">test</div>
                </div>
            </div>
        </div>
    </div>`
}

function get_status() {
    var xhr = new XMLHttpRequest();
    xhr.onload = () => {
        let response = JSON.parse(xhr.responseText)
        let binary = response.status;
        let status_leds = document.querySelectorAll("#status_leds .led" );
        console.log(status_leds)
        for(let i = 0; i < binary.length;i++){
            if(i>=status_leds.length)
                break;
            if(binary[i] == '1')
                status_leds[i].style.backgroundColor = "#0F0"
            else
                status_leds[i].style.backgroundColor = "black"
            console.log(binary[i])
        }
        setTimeout(get_status, 500);
    };
    xhr.onerror = () => {
      setTimeout(get_status, 2000);
    }
    xhr.timeout = 2000;
    xhr.ontimeout = () => {
      setTimeout(get_status, 2000);
    }
    xhr.open("GET", "http://192.168.4.62/engine/status", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send();
  }
  setTimeout(get_status, 1000);







if (window.addEventListener) // W3C standard
  window.addEventListener('load', run_after_load, false); // NB **not** 'onload'
else if (window.attachEvent) // Microsoft
  window.attachEvent('onload', run_after_load);
export { }