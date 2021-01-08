#pragma once

#include <Arduino.h>
const char head_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">

    <head>
        <title>RFX CONTROL</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" type="text/css" href="default.css"> <!-- custom css styling-->
    </head>

)rawliteral";
const char index_html[] PROGMEM = R"rawliteral(
    <body>
        <div class="rfx-panel" style="max-width: 400px; margin:auto">
            <label id="header-label">RFX-ESP</label>
            <div>
                <button class="rfx-button" onclick="window.location.href='connect'">WiFi Connection</a>
                <button class="rfx-button" onclick="window.location.href='stats'">System Stats</button>
                <button class="rfx-button" onclick="window.location.href='update'">Firmware Update</button>
                <button id="restart_button" class="rfx-button">Restart</button>
            </div>
        </div>
    </body>
    <script>
        document.getElementById('restart_button').addEventListener('click', event => {
    if (!confirm("Perform Hardware Restart (Power Cycle)?"))
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
    xhr.open("GET", "/server/restart", true);
    xhr.setRequestHeader("Content-Type", "text/html");
    xhr.send();
    })
    </script>
)rawliteral";

const char connect_html[] PROGMEM = R"rawliteral(
<body>
    <div class="rfx-panel" style="max-width: 400px; margin:auto">
        <label id="header-label">Wifi</label>
        <div>
            <label><b>Name on network</b></label>
            <input id="dns_name" class="rfx-input" type="text" placeholder="Access server via name">
            <label><b>SSID</b></label>
            <input id="ssid" class="rfx-input" type="text" placeholder="Wifi Network Name">
            <label><b>Password</b></label>
            <input id="password" class="rfx-input" type="password" placeholder="Wifi Password">
            <button id="submit-button" class="rfx-button" style="width:100%;"
                onclick="submit()">Submit</button>
        </div>
    </div>
</body>

<script>
    document.querySelectorAll('.rfx-input').forEach(item => {
        item.addEventListener('change', event => {
            console.log("change")
            function mark(el) {
                with (el.classList) {
                    remove("rfx-good")
                    add("rfx-caution")
                    remove("rfx-warning")
                }
            }
            mark(item)
            mark(document.getElementById('submit-button'))
            document.getElementById('header-label').innerText = "Wifi Settings: Need to submit"
        })
    })
    function getServerInfo() {
        var xhr = new XMLHttpRequest();
        xhr.onload = () => {
            let response = JSON.parse(xhr.responseText)
            document.getElementById("dns_name").placeholder = response.dns_name;
            document.getElementById("ssid").placeholder = response.ssid;
        };
        xhr.onerror = () => {
            setTimeout(getServerInfo, 1000);
        }
        xhr.open("GET", "http://192.168.4.62/server/status", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.send();
    }
    function submit() {
        let data = {};
        // Build json data packet 
        document.querySelectorAll('.rfx-input').forEach(item => {
            data[item.id] = item.value;
        })
        var xhr = new XMLHttpRequest();
        function bad_response() {
            function mark(el) {
                with (el.classList) {
                    remove("rfx-good")
                    remove("rfx-caution")
                    add("rfx-warning")
                }
            }
            mark(document.getElementById('submit-button'))
        }
        xhr.onload = () => {
            if (xhr.response === 'ok') {
                function mark(el) {
                    with (el.classList) {
                        add("rfx-good")
                        remove("rfx-caution")
                        remove("rfx-warning")
                    }
                }
                mark(document.getElementById('submit-button'))
                document.getElementById('header-label').innerText = "Wifi Settings: Saved"
                document.querySelectorAll('.rfx-input').forEach(item => {
                    item.classList.remove("rfx-good")
                    item.classList.remove("rfx-caution")
                    item.classList.remove("rfx-warning")
                })
            }
            else {
                document.getElementById('header-label').innerText = "Wifi Settings: Error- Bad Response";
                bad_response()
            }

        };
        xhr.onerror = () => {
            document.getElementById('header-label').innerText = "Wifi Settings: Error- Server";
            bad_response()
        }
        xhr.ontimeout = () => {
            document.getElementById('header-label').innerText = "Wifi Settings: Error- Server Unreachable";
            bad_response()
        }
        xhr.timeout = 2000
        xhr.open("POST", "/server/settings", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        console.log(JSON.stringify(data))
        xhr.send(JSON.stringify(data));
    }
    getServerInfo();
</script>
)rawliteral";
    
const char stats_html[] PROGMEM = R"rawliteral(
<body>
    <div class="rfx-panel" style="max-width: 400px; margin:auto">
        <label id="header-label">RFX-ESP</label>
        <div>
            <div style="margin-bottom: 8px;">
                <label><b>Firmware / Ver: </b></label>
                <label id="version" style="float:right">NULL</label>
                <label style="float:right; padding:0 4px 0 4px"> - </label>
                <label id="firmware" style="float:right">NULL</label>
            </div>
            <div class="rfx-panel" style="max-width: 400px; margin:auto; margin-bottom: 8px;">
                <label id="network-label" style="color:black;background-color: lightgray;">Network</label>
                <div>
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
                        <label style="float:right"><b id="server_ping" style="filter: brightness(80%);">NULL</b></label>
                    </div>
                </div>
            </div>
            <div class="rfx-panel" style="max-width: 400px; margin:auto">
                <label id="header-label">CPU</label>
                <div>
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

        </div>
    </div>
</body>
<script>
    function update_server_status(status) {
        let color = "lightCoral";
        with (document) {
            if (status == "ok") {
                with (getElementById("server_ping").classList) {
                    add("rfx-good-invert")
                    remove("rfx-caution-invert")
                    remove("rfx-warning-invert")
                }
                with (getElementById("network-label").classList) {
                    add("rfx-good")
                    remove("rfx-caution")
                    remove("rfx-warning")
                }
            }
            else if (status == "caution") {
                with (getElementById("server_ping").classList) {
                    remove("rfx-good-invert")
                    add("rfx-caution-invert")
                    remove("rfx-warning-invert")
                }
                with (getElementById("network-label").classList) {
                    remove("rfx-good")
                    add("rfx-caution")
                    remove("rfx-warning")
                }
            }
            else {
                with (getElementById("server_ping").classList) {
                    remove("rfx-good-invert")
                    remove("rfx-caution-invert")
                    add("rfx-warning-invert")
                }
                with (getElementById("network-label").classList) {
                    remove("rfx-good")
                    remove("rfx-caution")
                    add("rfx-warning")
                }
            }
        }
    }
    function getServerInfo() {
        var xhr = new XMLHttpRequest();
        let start_date = new Date();
        xhr.onload = () => {
            let end_date = new Date();
            let millis = end_date.getTime() - start_date.getTime();
            let r = JSON.parse(xhr.responseText)
            with (document) {
                getElementById("firmware").innerText = r.firmware;
                getElementById("version").innerText = r.version;
                getElementById("server_ip").innerText = r.ip;
                getElementById("server_name").innerText = r.dns_name;
                getElementById("server_mode").innerText = r.mode;
                getElementById("server_ssid").innerText = r.ssid;
                getElementById("server_ping").innerText = millis;
                getElementById("up_time").innerText = Math.round(r.up_time / 1000)
                getElementById("total_bytes").innerText = Math.round(r.total_bytes / 1000)
                getElementById("used_bytes").innerText = Math.round(r.used_bytes / 1000)

                let total_heap = r.total_heap;
                let free_heap = r.free_heap;
                getElementById("server_total_heap").innerText = Math.round(total_heap / 1000);
                getElementById("server_used_heap").innerText = Math.round((total_heap - free_heap) / 1000);
                getElementById("server_freq").innerText = r.freq;
                if (millis < 250) {
                    update_server_status("ok");
                    getElementById("network-label").innerText = "Network";
                }
                else if (millis < 750) {
                    update_server_status("caution");
                    getElementById("network-label").innerText = "Network";
                }
                else {
                    update_server_status("warning");
                    getElementById("network-label").innerText = "Network - Slow r (ping)";
                }
            }
            setTimeout(getServerInfo, 1000);
        };
        xhr.onerror = () => {
            update_server_status("warning");
            document.getElementById("server_ping").innerText = "error";
            document.getElementById("network-label").innerText = "Network - Error";
            setTimeout(getServerInfo, 1000);
        }
        xhr.timeout = 2000;
        xhr.ontimeout = () => {
            update_server_status("warning");
            document.getElementById("server_ping").innerText = "timeout";
            document.getElementById("network-label").innerText = "Network - Timeout";
            setTimeout(getServerInfo, 1000);
            document.getElementById("server").classList.add("w3-disabled");
        }
        xhr.open("GET", "/server/status", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.send();
    }
    setTimeout(getServerInfo, 1000);
</script>
)rawliteral";
    
const char update_html[] PROGMEM = R"rawliteral(
<body>
    <div class="rfx-panel" style="max-width: 400px; margin:auto">
        <label id="header-label">Firmware</label>
        <div>
            <label><b>Firmware: </b></label>
            <label id="firmware" style="float:right">NULL</label>
        </div>
        <div>
            <label><b>Version: </b></label>
            <label id="version"style="float:right">NULL</label>
        </div>
        <div>
            <button id="OTA_file_select" class="rfx-button" onclick="document.getElementById('fileinput').click()">
                <input id="fileinput" type='file' name='update' onchange="document.getElementById('OTA_Execute').classList.remove('disabled'); document.getElementById('progress_label').innerText=document.getElementById('fileinput').value; document.getElementById('progress_container').style.backgroundColor = 'lightgray'">
                Select Firmware for Update
            </button>
            <div id= "progress_container" class="progress" style="margin-top:8px; text-align: center; position: relative; height:26px; background-color: lightgray;">
                <div id="progress" style="position: absolute; background-color: rgb(74, 128, 172); width:0%; height:100% "></div>
                <div id="progress_label" style="position: absolute; top:0px; width:100%;text-align: center;"></div>
            </div>
            <button id="OTA_Execute" class="rfx-button disabled" style="margin-top:8px">Upload</button>
        </div>
    </div>
</body>

<script>
    document.querySelectorAll('.rfx-input').forEach(item => {
        item.addEventListener('change', event => {
            console.log("change")
            function mark(el) {
                with (el.classList) {
                    remove("rfx-good")
                    add("rfx-caution")
                    remove("rfx-warning")
                }
            }
            mark(item)
            mark(document.getElementById('submit-button'))
            document.getElementById('header-label').innerText = "Wifi Settings: Need to submit"
        })
    })
    function getServerInfo() {
        var xhr = new XMLHttpRequest();
        xhr.onload = () => {
            let response = JSON.parse(xhr.responseText)
            document.getElementById("version").innerText  = response.version;
            document.getElementById("firmware").innerText  = response.firmware;
        };
        xhr.onerror = () => {
            setTimeout(getServerInfo, 1000);
            console.log("error")
        }
        xhr.open("GET", "http://192.168.4.62/server/status", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.send();
    }
    getServerInfo();
    document.getElementById('OTA_Execute').addEventListener('click', event => {
    var xhr = new XMLHttpRequest();
    function done(msg){
        document.getElementById('OTA_file_select').classList.remove('disabled');
        document.getElementById("progress_label").innerText = msg;
        document.getElementById("progress").style.width = 0;
        getServerInfo();
    }
    xhr.onload = () => {
      if (xhr.response === 'ok') {
        console.log(xhr.response);
        console.log("restart recieved")
        done("Firmware Update Successful");
        document.getElementById("progress_container").style.backgroundColor = "lightGreen";
        
      }
      else {
        console.log(xhr.response);
        done("OTA Bad Response")
        document.getElementById("progress_container").style.backgroundColor = "lightCoral";
      }
    };
    xhr.onerror = () => {
        console.log("restart error")
        done("Update Error");
        document.getElementById("progress_container").style.backgroundColor = "lightCoral";
      
    }
    xhr.ontimeout = () => {
        console.log("restart timeout")
        done("Server Timeout");
        document.getElementById("progress_container").style.backgroundColor = "lightCoral";
    }
    xhr.upload.addEventListener('progress', function (evt) {
      if (evt.lengthComputable) {
        var per = evt.loaded / evt.total;
        console.log(per);
        document.getElementById("progress").style.width = Math.round(per * 100) + "%";
      }
      
     })
    xhr.open("POST", "http://192.168.4.62/update", true);
    let file = document.getElementById("fileinput").files[0];
    var formData = new FormData();
    formData.append("update", file, file.name);
    xhr.file = file;
    document.getElementById('fileinput').value='';
    document.getElementById('OTA_Execute').classList.add('disabled');
    document.getElementById('OTA_file_select').classList.add('disabled');
    document.getElementById("progress_label").innerText = "Uploading..."
    xhr.send(formData);
        return false;
  })
</script>
)rawliteral";

const char default_css[] PROGMEM = R"rawliteral(
 :root {
    --caution: #ffd26f;
}

body {
    font-family: Verdana, sans-serif;
    box-sizing: border-box;
}

.rfx-caution {
    color: #000;
    background-color: var(--caution) !important
}

.rfx-caution-invert {
    color: var(--caution) !important;
}

.rfx-good {
    color: #000;
    background-color: #75eb5d !important
}

.rfx-good-invert {
    color: #75eb5d !important;
}

.rfx-warning {
    color: #000;
    background-color: #d65656 !important
}
.rfx-panel {
    border: 1px solid rgb(150, 149, 149);
    border-radius: 4px;
    margin: 8px 0 8px 0;
    box-sizing: border-box;
    align-items: center;
  }
  
  .rfx-panel+.rfx-panel {
    margin-top: -4px;
  }
  
  .rfx-panel>*:first-child {
    background: rgb(0,0,0);
    color:white;
    border-bottom: 1px solid #aaa;
    padding: 2px 4px 2px 4px;
    font-weight: bold;
    display: flex;
    box-sizing: inherit;
    align-items: center;
    width: 100%;
  }
  
  .rfx-panel>*:not(first-child) {
    padding: 8px;
    align-items: center;
    width:100%;
    box-sizing: inherit;
  }

input{
    font-family: 'Courier New', monospace;
    border: 1px solid #CCC;
    width: calc(100% - 5px);
    padding: 2px;
    font-size: 1.2rem;
    margin-bottom:8px;
}

.rfx-button {
    display: block;
    border-radius: 4px;
    border: 2px solid #000;
    transition-duration: 0.2s;
    box-shadow: 0 2px 4px 0 rgba(0, 0, 0, 0.2), 0 1px 5px 0 rgba(0, 0, 0, 0.19);
    cursor: pointer;
    font-size: 1.2rem;
    width: 100%;
    margin-bottom:4px;
}

.rfx-button:hover {
    box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 2px 5px 0 rgba(0, 0, 0, 0.19);
    filter: brightness(110%)
}

.rfx-button:active {
    box-shadow: 0 1px 2px 0 rgba(0, 0, 0, 0.2), 0 1px 2px 0 rgba(0, 0, 0, 0.19);
    filter: brightness(90%)
}
input[type="file"] {
  font-family: 'Courier New', monospace;
  display: none;
}
.custom-file-upload {
  display: inline-block;
}
.disabled{
  cursor:not-allowed;
  opacity:0.3;
  pointer-events:none
}
.progress{
    border-radius: 4px;
    border: 2px solid #000;
    cursor: pointer;
    font-size: 1.2rem;
    width: calc(100%-4px);
    margin-bottom:4px;
}

)rawliteral";

const char favicon[] PROGMEM = R"rawliteral(‰PNG

   
IHDR           szzô   sRGB ®Îé   gAMA  ±üa   	pHYs  Ã  ÃÇo¨d  	bIDATXG•W	pUÕþî}÷¾{ßþ’E³!	­ƒÚvÐŠV#h‚Ø:`µQ7`:¶.£3íèà2
ÖÑÚ8Ó
+-µu*:Zlq©.PTH	ÉY^ÞËÛï=ýÎ=!F	SýfÎÜsï9ÿzþå\M¯‹e?nÁ 	ÝÑø& t [rÑzñ˜»´U~üê
|•Ñ¹¢Iô-„p[-13î—Z7.›$†At.-ýï$ÙÄ¼Æÿë®%¶ëyV ÂgÀ
	«óÿ–DeD€Æc0ëÃg±¼!…d°‹yä‹9zÈ/^‡†ÖÛOè•*0wþ"[ðÒÒ2y.ºZ>5Øp`Â´™tÒ;ŠŽ*)”4&×l:fk—†%ï‡‘<2¡Ò€ãpÆg‰—_Ø‚wr~šCAÜæøŠR[X|3Màæ”ã=˜|˜>—oò£Aqºeâ¹„…üP4M›ÐÒã<pÆŒ³Ä®ÝíÞüÔ‰]W„Lö(Œ43dã`)Œ§?Ìâ­~©ø
ß¬0°xr çÕ’_z9¥aùðØž0Ö¾Ùíñ“ ¼/zB*pl\´`¡ÔÆ-UQ!Ö6‹ÜU–×ÆÅž+§Šs«£cë'qÍm-5Â]Ó(òW†„¸!.~?¯qÜióç2¿ ÀlÆYŒh¹'¼p•-Äª“Ä]3ëÇ1‡®I3Âê©æÇÖÕ8=l‹Á›E©5"Äu1±aö)Þ÷óÂºØ|ûÕ§dŽA×b¢6FVvE~sRœÆpÍ+ø¤—;%©sw¼™”¬ ùŒŸ+t]ß„ªlhÁIðå†ì@ìïê(¼ <¼ùAB
#ãt2Bª¶eãŽ÷L%\—ÛJøùùÓñ°É¹Ãaz¦.ØcŠÁÖËš±oÅtïýÌ¶ƒpc(2ƒ}”‘¥.:¿XìiéQéÛÚà÷ûÉPfy	†ÏÄG¹8î}ûÉÜuå6ìÞŸ®ŽûfÌ·¢wÇ érÀúó«1·ªAä½ï}äwÓ¿²ÚdCÁÂµPä‹Óþ¬·î)àKtÀ5|p5—°hýêíý\‘ÂVÏl–Ûðâá4ßÖ[gY^:º´W‚”d´LŠ`ÍÙalë­Áw7ÉÈW^ùÍ‡}èÖj`‹"ð¹:¢jIí¨®°aÉ‚NHt•¢ø÷‘a¾Éoæ\Ä¦yõ¸îÔ
V<Ö†¡!|¿>ŽŠ€‰æhÀ;Ù•!l_YU/&pÑ3Ÿ¡3“—ê{C¢­}–i1è*¡#õ£ó‘ÕÂ··ã3qÿ³»°pF
ÂÏÇÒð»KØz(CjåãC½Ö_\†…õ)”ÒIäó&.F¢_Çš°k_
¯®=
«Ÿ>€_ïIòÔT9DZ`%Ëu±X@ Áïû±êù½Ð;“%ÌýË !Ë¢öAåZ%_ÃëƒiL~ü îz+„}˜Œ€‘‡žJãž?ž|­ë/kÂÂG÷°è$=:žÚ¨å2GÔìãd†Õ4„0Y·í¸÷ƒ>„Ë¢ŒoW:¨‰ÓÝ%R2â±Ì*Å“ƒ#Üó^7NÛ¸»œJè,»V*‰fkxæ­Ã¸ûSqfe„û•Hf¹zzV¨ãíM«@ÙÊÀŽ½ŸŒ®†œÉHÖ$I¤ØŒAWD×L¯ÆŸ>64£^TgÒ#X×EÛŽ.Œå~’*áä%$Ê¢\¡p,¬ÆÀ;…,0R;IÀ\q\Ä#~‘­Ví”Ö€žZT[Ž»ç„W?éÆ£í~dÍ8töÒP/šg³_(A‚OI­³Ph^ˆ*Ô•Éªc°$P[_O›jE\=#Q`ä“Ç´ "’­W×”åó)üË¢X÷l'þ¹²¯wôàgÿ)a¾’ÝÑ„1”Ä»W”á±ïÕáòS*<j—…B+±Á=H35\RëÇvÊ\‰&>õ«‡Ä¬wîG£Cº Ã0\|”)Çw6P”DKMÛ//cOO°;Û¸e»†~»
;ûrX³í ö,oÂ·W:Á Ž…ñàë‚
ÊZ +gW4•ã©9ÃÙ",f‰eû±Ñ<_õ‚áKùˆÆè2Ç;¢@<††Ç¬Þ
×4£ÑéA–Ž•…Ë£xä7íØï­K”VÖQNì
ãùÝ	¼tÃtëØÖ½` ¶/™‚"	dYézÕZ7¨ âËf~–t?SÈç¥Øsªä’‡CÌaÃäe„'ÅqßNM	—=ÂãÀ4K0\ôãæxe ‡ù›Ž ÌÏE
Ÿ·éÅ22”˜è&ãi„‰\¸–¾ ‚óWñ:•…ß-0²‹ÈÐð+¦ê8'ô˜·<ý_¼›«†^U‡›ß(âö7F/ìº«bdùX]ŽçMóÞ·èÅ 
/üuÑÉ(ŒP¸É$+A89ˆæ³½µ±vÜ½HUñ-5¹Q0XòÈ…ÐÜ¶G]yRWåì<$”»äœJò2FÿaÙé5xåà¥RÞÖ?Ï›‚ËúQÊ»lx“ÉAîè0‚[Æµc‰Ì‚›ñZo ?ÝibýG!Xá0ìLö^ß€SlHR¸¤á @¥¶t±>ª–Ë›!ðÇ{Æ„o¾d
–6öÃáõ¬Ã-Ç’­OìóawÝo]bÌvYÈ3§%ž˜Óˆk§£À¦bD*që9<Ø®Ö>Çh„=fVD°yA
êü=ÐYœz}U˜üÛýTPí¡LÏz‰/( 1þözß¬Ü2=Å˜pñ—Ðç«Ã»ÒxæÓ$ÞÝ¥0Ù¶qac×}«ç”åË…ß1ÑîFqîS=¼¨Ú2^¸Äq
HŒW"¿¦ÅÁ#L?Ï¸ ›?'Lb8fYþ”ƒÿ‚ÞvŠÅ"òìF2Õb–Ÿ¼ìâÉL9âËÂ%Æb`<¼®i\õç“^C‘5¢ k]:’Î@Ïô"œíF<uÌ JlLÃ¼vå™ç®ÆªÇ u¨ÌâSU©ŸH¸Ä„
H×Õî¸e’}}0˜´.ÓÇÇü•åÕŽñ0ïûUàÉ®J<¼×B;K²Í_8—–û¨¬&³¨?…æ¦'.1á|½¿\,J¼ÃEÈ&ÊKéý“pÛ›GFW*(8±¼Ù¡r,2¥©3Q½áí
ƒTàëŒ.^£ê'ÕŠæiÍRsoÔÖOå5µbÓm+ÄÈ–
Ü61íñCàüvÅŠ)¨är    IEND®B`‚)rawliteral";