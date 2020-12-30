let tmpl = document.createElement('template')
tmpl.innerHTML= /*html*/`
<style>
.checkbox{
  height: 32px;
  width: 160px;
  padding: 0px;
  display: flex;
  align-items: center;
  border-radius: 5px;
  justify-content: flex-end;
}
.checkbox input{
  outline: none;
  height: 32px;
  width: 72px;
  border-radius: 16px;
  -webkit-appearance: none;
  position: relative;
  background: #e6e6e6;
  box-shadow: inset 0 0 5px rgba(0,0,0,0.2);
  transition: 0.4s cubic-bezier(0.68, -0.55, 0.265, 1.55);
}
.checkbox input:checked{
  background: #88cc88;
}
.checkbox input:before{
  position: absolute;
  content: "";
  left: 0;
  height: 100%;
  width: 32px;
  background: linear-gradient(#fff,#f2f2f2,#e6e6e6,#d9d9d9);
  box-shadow: 0 2px 5px rgba(0,0,0,.2);
  border-radius: 50px;
  transform: scale(0.85);
  transition: left 0.4s cubic-bezier(0.68, -0.55, 0.265, 1.55);
}
input:checked:before{
  left: 40px;
}
table{
    width:100%; height:32px;position:relative; max-height: 2rem;
}
#label_container{
    width:15%; 
    text-align:right;
    font-family: Verdana, sans-serif;
    font-weight: bold;
    margin-right:8px;
    margin-left:8px;
    padding-left:8px;
    white-space: nowrap;
}
#input_text, #input_select{
    font-family: 'Courier New', monospace;
    border:1px solid #ccc;
    height:28px;
    color:#000;
    background-color:#f1f1f1;
    font-size:1.6rem;
    width:100%;
    text-align:left;
    font-weight: bold;
}
#checkbox_text{
    font-weight:bold;
}
#textbox_container{
    padding:0px 8px 0px 8px;
    display:flex;
    align-content:bottom;
}
#select_container{
    padding:0 8px 0 8px;
}
#checkbox_container{
    float:right;
    padding-right:8px
}
</style>
<table>
    <tr>
        <td id="label_container"></td>
        <td id="textbox_container">
            <input id="input_text" type="text">
            <span id="units" style="display:none;margin-top:auto; margin-bottom:2px;padding-right:2px;font-size:.8rem">(usec/step)</span>
        </td>
        <td id="checkbox_container">
            <div class="checkbox">
                <label id="checkbox_text" class="text"></label>
                <input id="input_checkbox" type="checkbox">
            </div>
        </td>
        <td id="select_container">
            <select id="input_select" type="text"></select>
        </td>
    </tr>
</table>
    `
export default class data_box extends HTMLElement {

    constructor() {
        super();
        this._internals = this.attachInternals();
        // Attach a shadow root to the element.
        let shadowRoot = this.attachShadow({mode: 'open'});
        let newTmp = tmpl.content.cloneNode(true)

        // Set the label text by what ever is inside the tag.
        newTmp.getElementById("label_container" ).innerText = this.innerText
        if(this.innerHTML.length==0)
            newTmp.getElementById("label_container" ).setAttribute("dislay","none");
        shadowRoot.appendChild(newTmp);

        this._type = "text"
        this._units = ""

        // Create a reference to the two main components
        this.input = this.shadowRoot.getElementById("input_text");
        let checkbox = this.shadowRoot.getElementById("input_checkbox")

        this.original_background = this.style.getPropertyValue("background-color");

        checkbox.addEventListener("change",()=>{       
            if(checkbox.checked)     
                this.shadowRoot.querySelector("#checkbox_text").innerHTML="True"
                else
            this.shadowRoot.querySelector("#checkbox_text").innerHTML="False"
        })
    } 
    static get observedAttributes() { return ["type", "units", "value", "placeholder"]; }
    attributeChangedCallback(name, oldValue, newValue) {
        // ##### Handle input type #####
        if(name=="placeholder"){ 
            this.shadowRoot.getElementById("input_text").setAttribute("placeholder",newValue)
        }
        if(name == "type"){
            this.shadowRoot.getElementById("checkbox_container").style.setProperty("display","none");
            this.shadowRoot.getElementById("select_container").style.setProperty("display","none");
            this._type = newValue;
            if(this._type == "float"){
                this.input.setAttribute("type","number")
                this.input.removeAttribute("step")
            }
            else if(this._type == "integer"){
                this.input.setAttribute("type","number")
                this.input.setAttribute("step","1")
            }
            else if(this._type == "checkbox"){
                this.shadowRoot.getElementById("textbox_container").style.setProperty("display","none");
                this.shadowRoot.getElementById("checkbox_container").style.removeProperty("display")//,"unset");
                this.input = this.shadowRoot.getElementById("input_checkbox");
            }
            else if(this._type == "select"){
                this.shadowRoot.getElementById("textbox_container").style.setProperty("display","none");
                this.shadowRoot.getElementById("select_container").style.removeProperty("display")//,"unset");
                this.shadowRoot.getElementById("input_select").innerHTML=this.innerHTML
                this.input = this.shadowRoot.getElementById("input_select");
            }
            else{
                this.input.setAttribute("type",this._type)
            }
            this.input.addEventListener("change",()=>{
                this.shadowRoot.querySelector("table").style.setProperty("background-color","#ffffbb");

                let value = this.input.value;
                if(this._type == "integer"){
                    value = parseInt(value,0);
                    this.input.value = value;
                }
            })
        }
        // ##### handle units #####
        if(name == "units"){
            let units = this.shadowRoot.getElementById("units")
            this._units = newValue;
            if(this._units==""){
                units.style.setProperty("display","none")
            }
            else{
                units.style.removeProperty("display")
                let distance = units.offsetWidth
                this.input.style.setProperty("padding-right",String(distance)+"px")
                units.style.setProperty("margin-left",String(-distance)+"px")
            }
        }
        if(name == "value"){
            if(this._type=="checkbox"){
                if(newValue == "true"){
                    this.input.checked = true;
                    this.shadowRoot.querySelector("#checkbox_text").innerHTML="True"
                }
                else{
                    this.input.checked = false;
                    this.shadowRoot.querySelector("#checkbox_text").innerHTML="False"
                }
            }else{
                this.input.setAttribute("value",newValue);
            }
        }
    }
    connectedCallback() {
        this.shadowRoot.querySelector("table").style.setProperty("background-color",this.original_background);
        this.shadowRoot.getElementById("label_container" ).style.setProperty("display","none")
        if(this.firstChild){
            let html_tag_index = this.innerHTML.indexOf("<");
            if(html_tag_index < 0){
                let innerString = this.innerHTML
                innerString = innerString.replace(/(\r\n|\n|\r|\t)/gm, "");
                innerString = innerString.trim();
                console.log("length"+innerString.length)
                if(innerString.length>0){
                    this.shadowRoot.getElementById("label_container" ).innerText = innerString;
                    this.shadowRoot.getElementById("label_container" ).style.removeProperty("display")
                }
            }
            else{
                let innerString = this.innerHTML.substring(0,this.innerHTML.indexOf("<"));
                innerString = innerString.replace(/(\r\n|\n|\r|\t)/gm, "");
                innerString = innerString.trim();
                console.log("length"+innerString.length)
                if(innerString.length>0){
                    this.shadowRoot.getElementById("label_container" ).innerText = innerString;
                    this.shadowRoot.getElementById("label_container" ).style.removeProperty("display")
                }

            }
        }
    }
    clear_change_flag(){
        this.shadowRoot.querySelector("table").style.setProperty("background-color","#"+String(this.original_background));
    }
    static get formAssociated() { return true; }
}
customElements.define('rfx-input',data_box)
