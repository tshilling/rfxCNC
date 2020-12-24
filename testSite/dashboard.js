class dashboard{
    loadDashboard(){
        console.log("dashboard loaded");
    }
}
class my_class extends HTMLDivElement{
    constructor() {
        super();
    }
    set_values(){
        console.log("my Values")
    }
}

export default class Container extends HTMLDivElement {
    constructor() {
        super();
        this.innerHTML = 'Content'; // Does Nothing
    } 
    connectedCallback() { // Fires when attached
      console.log('Callback');
      this.innerHTML = /*html*/`
        <div>content</div>
        <div>second</div>
        `;
    }  
}

customElements.define('my-contain', Container, { extends: "div" });
customElements.define('my-c', my_class, { extends: "div" });
export {dashboard, Container, my_class}