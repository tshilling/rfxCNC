function run_after_load(){
    document.getElementById("header-right").innerHTML += /*html*/`
    <a id="nav_engine" href="#engine" class="w3-button"><span class="icon-cogs"></span></a>
    `
    document.getElementById("main-view").innerHTML += /*html*/`
    <template id="status-row-template">
        <tr class="rfx-info-line">
            <td style="margin:auto;"><label style="height:24px; display:flex; align-items: center;justify-content: center"><b id="label">X</b></label></td>
            <td><label style='height:24px; display:flex; align-items: center;justify-content: center'><span class='led' style='width:16px; height:16px'></span></label></td>
            <td><label style='height:24px; display:flex; align-items: center;justify-content: center'><span class='led' style='width:16px; height:16px'></span></label></td>
            <td><label style='height:24px; display:flex; align-items: center;justify-content: center'><span class='led' style='width:16px; height:16px'></span></label></td>
        </tr>
    </template>
    <div id="engine" class="rfx-tab" style="display:none; max-width: 1200px; margin: auto;">
        <div class="w3-row-padding" style=" margin:auto;">
            <div id="server-network"class="w3-third" style="min-width:150px !important; width:150px">
                <div id="status_leds" class="rfx-panel" style="margin-bottom:8px ">
                    <label>Inputs
                    </label>
                    <table id="status-table" style=" border-collapse: collapse;">
                        <colgroup>
                            <col span="1" style="">
                            <col span="1" style="width: 24px;">
                            <col span="1" style="width: 24px;">
                            <col span="1" style="width: 24px;">
                        </colgroup>
                        <tr class="rfx-info-line">
                            <td style="margin:auto;"><label style="height:24px; display:flex; align-items: center;justify-content: center"><b id="label">Axis</b></label></td>
                            <td style="margin:auto;"><label style="height:24px; display:flex; align-items: center;justify-content: center"><b id="label">-</b></label></td>
                            <td style="margin:auto;"><label style="height:24px; display:flex; align-items: center;justify-content: center"><b id="label">+</b></label></td>
                            <td style="margin:auto;"><label style="height:24px; display:flex; align-items: center;justify-content: center"><b id="label">H</b></label></td>
                        </tr>
                    </table>
                    
                    <div style="padding:0 4px 0 4px; height:24px">
                        <label><b>A-Max:</b></label>
                        <label style="height:24px; display:flex; align-items: center; float:right"><span class="led" style="width:16px; height:16px"></span></label>
                    </div>
                </div>   
            </div>  
            <div id="server-network"class="w3-third" style="min-width:150px !important;">
                <div class="rfx-panel" style="margin-bottom:8px min-width:150px !important; width:150px">
                    <label>Coordinates
                    </label>
                    <div>
                        <label id="absolute-0">0</label>
                    </div>
                    <div>
                        <label id="absolute-1">0</label>
                    </div>
                    <div>
                        <label id="absolute-2">0</label>
                    </div>
                    <div>
                        <label id="absolute-3">0</label>
                    </div>
                </div>
                </div>     
        </div>  
    </div>`

    // insert LEDs, saves from typing above...
    let els = document.getElementsByClassName("insert_LED");
    Array.prototype.forEach.call(els, function(el) {
        // Do stuff here
        el.style="margin:auto";
        el.innerHTML = "<label style='height:24px; display:flex; align-items: center;justify-content: center'><span class='led' style='width:16px; height:16px'></span></label>"
    
    });
    get_config();
}
let config
function get_config(){
    var xhr = new XMLHttpRequest();
    xhr.onload = () => {
        config = JSON.parse(xhr.responseText)

        let temp = document.getElementById('status-row-template');
        for(let i = 0; i < config.axis.length;i++){
            let clon = temp.content.cloneNode(true);
            clon.getElementById("label").innerHTML = config.axis[i].axis_id;
            document.getElementById('status-table').appendChild(clon);
        }
        get_status();
    };
    xhr.onerror = () => {
      setTimeout(get_config, 2000);
    }
    xhr.timeout = 2000;
    xhr.ontimeout = () => {
        setTimeout(get_config, 2000);
    }
    xhr.open("GET", "http://192.168.4.62/engineConfig.json", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send();
}

function get_status() {
    var xhr = new XMLHttpRequest();
    xhr.onload = () => {
        let response = JSON.parse(xhr.responseText)
        let binary = response.status;
        let status_leds = document.querySelectorAll("#status_leds .led" );

        for(let i = 0; i < binary.length;i++){
            if(i>=status_leds.length)
                break;
            if(binary[i] == '1')
                status_leds[i].style.backgroundColor = "#2F2"
            else
                status_leds[i].style.backgroundColor = "#222"
        }

        for(let i = 0; i < config.axis.length;i++){
            if(response.axis[i].position == 0)
                document.getElementById("absolute-"+i).innerText = "0"
            else
                document.getElementById("absolute-"+i).innerText = response.axis[i].position / config.axis[i].steps_per_unit;
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







if (window.addEventListener) // W3C standard
  window.addEventListener('load', run_after_load, false); // NB **not** 'onload'
else if (window.attachEvent) // Microsoft
  window.attachEvent('onload', run_after_load);
export { }