(async () => {

  const get = selector => document.querySelector(selector);
  const getAll = selector => document.querySelectorAll(selector);

  let runInterval = undefined;

  const MCU_SP_START = 1024 * 2 - 1;

  let tabs = {code: true};
  let tabsCount = 1;

  if (!localStorage.theme) {
    localStorage.theme = 'monokai';
  }

  if (!localStorage.frequency) {
    localStorage.frequency = 1;
  }

  const editor = CodeMirror(get('.code-wrap'), {
    lineNumbers: true,
    mode: 'text/x-c',
    theme: localStorage.theme,
    value: '// Write code here...',
    indentUnit: 2,
    tabSize: 2,
    indentWithTabs: true,
    lineWrapping: true,
    allowDropFileTypes: ['text/plain', 'text/*'],
    spellcheck: false,
    autocorrect: false,
    autocapitalize: false
  });

  if (localStorage.lastCode) {
    editor.setValue(localStorage.lastCode);
  }

  let saveCodeInterval = setInterval(() => {
    localStorage.lastCode = editor.getValue();
  }, 1000);

  const runMCU = () => {
    runInterval = setInterval(() => {
      socket.emit('execute cycle');
    }, 1000 * (1 / +localStorage.frequency));
  }

  const getStack = state => {
    let stack = [];
    for (let i = MCU_SP_START - 1; i >= state.sp; i--) {
      stack.push(state.data_memory[state.RAM + 1 + i]);
    }
    return stack;
  }

  const terminalAppend = (terminal, text) => {
    let span = create('span', {}, text);
    let shouldScroll = terminal.shouldScroll();
    terminal.appendChild(span);
    if (shouldScroll) {
      terminal.scrollDown(terminal.scrollHeight * 10);
    }
  }

  HTMLElement.prototype.on = function(events, callback, bool) {
    let eventsArray = events.split(' ');
    let element = this;
    eventsArray.forEach(event => {
      element.addEventListener(event, callback, bool);
    });
  }

  HTMLElement.prototype.shouldScroll = function() {
    let scrollPosition = this.scrollTop + this.offsetHeight;
    let difference = Math.abs(this.scrollHeight - scrollPosition);
    return difference < 10;
  }

  HTMLElement.prototype.scrollDown = function(amount) {
    this.scrollTop = amount;
  }

  NodeList.prototype.on = function(events, callback, bool) {
    let eventsArray = events.split(' ');
    Array.from(this).forEach(element => {
      eventsArray.forEach(event => {
        element.addEventListener(event, callback, bool);
      });
    });
  }

  const create = (name, options, text) => {
    let element = document.createElement(name);
    if (typeof options === 'object') {
      for (let [key, value] of Object.entries(options)) {
        element.setAttribute(key, value);
      }
    }
    if (typeof text === 'string') {
      element.innerText = text;
    }
    return element;
  }

  for (let i = 1; i <= 32; i++) {
    let reg = create('div', {class: `reg reg-${i}`});
    reg.innerHTML = `R${i}: <span class='value'>0x0</span>`;
    get('.registers').appendChild(reg);
  }

  const socket = new WebSocket('ws://localhost:3000');
  const log = console.log;
  window.test = () => socket.emit('test', 'test');

  socket.handlers = {};

  socket.emit = function(event, data) {
    let object = {event, data};
    let message = JSON.stringify(object);
    log('Sending ', object)
    this.send(message);
  }

  socket.on = function(event, handler) {
    this.handlers[event] = handler;
  }

  socket.addEventListener('open', (event) => {
    log('Connected to the proxy server');
  });

  socket.addEventListener('close', (event) => {
    clearInterval(runInterval);
    terminalAppend(get('.log-output'), 'Connection closed');
  });

  socket.addEventListener('message', (event) => {
    let message = null;
    try {
      message = JSON.parse(event.data);
    } catch (error) {
      log('Could not parse a message');
      return;
    }
    log('Received ', message);
    if (typeof socket.handlers[message.event] === 'function') {
      socket.handlers[message.event](message.data);
    }
  });

  socket.on('ready', data => {
    log('Server ready, got data ', data);
  });

  socket.on('pong', data => {
    log('Server: ', data);
  });

  socket.on('test', data => {
    log('Python test:', data);
  });

  socket.on('log', data => {
    terminalAppend(get('.log-output'), data);
  });

  socket.on('console', data => {
    terminalAppend(get('.mcu-output'), data);
  });

  socket.on('execute stop', () => {
    log('Stopping');
    clearInterval(runInterval);
  });

  socket.on('mcu resumed', runMCU);

  let updateStack = state => {
    let stack = getStack(state);
    let wrap = get('.stack');
    while (wrap.firstChild) {
      wrap.firstChild.remove();
    }
    log('Stack', stack);
    if (stack.length === 0) {
      return wrap.appendChild(
        create('div', {class: 'value'}, 'Empty')
      );
    }
    let fragment = document.createDocumentFragment();
    stack.forEach(value => {
      let element = create('div', {class: 'value'}, `0x${value.toString(16)}`);
      fragment.appendChild(element);
    });
    wrap.appendChild(fragment);
  }

  let stackPointer = -1;

  socket.on('mcu state', state => {
    state = JSON.parse(state);
    get('.pc').innerText = '0x' + (state.pc * 2).toString(16);
    for (let i = 0; i < 32; i++) {
      get('.reg-' + (i + 1) + ' .value').innerText = '0x' + state.data_memory[state.R + i].toString(16);
    }
    log('MCU state', state);
    if (stackPointer !== state.sp) {
      updateStack(state);
    }
    stackPointer = state.sp;
  });

  get('.compile-asm').on('click', e => {
    let code = editor.getValue();
    socket.emit('compile asm', code);
  });

  get('.compile-c').on('click', e => {
    let code = editor.getValue();
    socket.emit('compile c', code);
  });

  get('.execute-cycle').on('click', e => {
    socket.emit('execute cycle');
  });

  get('.mcu-run').on('click', runMCU);

  get('.mcu-stop').on('click', e => {
    clearInterval(runInterval);
  });

  get('.mcu-resume').on('click', () => {
    socket.emit('mcu resume');
  });

  let setManagerName = () => {
    let getManagerName = num => {
      if (num < 2) return '';
      if (num === 2) return 'two'
      return 'three';
    };
    get('.tile-manager').setAttribute('class', 'tile-manager ' + getManagerName(tabsCount));
  }

  getAll('.tabs .tab').on('click', function() {
    let tab = this.getAttribute('tab');
    if (tabs[tab]) return;
    tabs[tab] = true;
    let tile = get('.tile[tab='+tab+']');
    tabsCount++;
    setManagerName();
    tile.style.display = 'block';
  });

  getAll('.tile .x').on('click', function() {
    let tab = this.parentElement.getAttribute('tab');
    if (!tabs[tab]) return;
    tabs[tab] = false;
    tabsCount--;
    let tile = get('.tile[tab='+tab+']');
    setManagerName();
    tile.style.display = 'none';
  });

  get('.theme-select').on('change', function() {
    if (this.value === 'none') return;
    editor.setOption('theme', this.value);
    localStorage.theme = this.value;
  });

  get('.freq').value = localStorage.frequency;

  get('.freq').on('change', function() {
    localStorage.frequency = this.value;
  });

  const sendInterrupt = vect => {
    socket.emit('interrupt', vect);
  }

  let interruptTable = [
    "RESET", /* Reset Interrupt Request */
    "INT0", /* External Interrupt Request 0 */
    "INT1", /* External Interrupt Request 1 */
    "PCINT0", /* Pin Change Interrupt Request 0 */
    "PCINT1", /* Pin Change Interrupt Request 0 */
    "PCINT2", /* Pin Change Interrupt Request 1 */
    "WDT", /* Watchdog Time-out Interrupt */
    "TIMER2_COMPA", /* Timer/Counter2 Compare Match A */
    "TIMER2_COMPB", /* Timer/Counter2 Compare Match B */
    "TIMER2_OVF", /* Timer/Counter2 Overflow */
    "TIMER1_CAPT", /* Timer/Counter1 Capture Event */
    "TIMER1_COMPA", /* Timer/Counter1 Compare Match A */
    "TIMER1_COMPB", /* Timer/Counter1 Compare Match B */
    "TIMER1_OVF", /* Timer/Counter1 Overflow */
    "TIMER0_COMPA", /* TimerCounter0 Compare Match A */
    "TIMER0_COMPB", /* TimerCounter0 Compare Match B */
    "TIMER0_OVF", /* Timer/Couner0 Overflow */
    "SPI_STC", /* SPI Serial Transfer Complete */
    "USART_RX", /* USART Rx Complete */
    "USART_UDRE", /* USART, Data Register Empty */
    "USART_TX", /* USART Tx Complete */
    "ADC", /* ADC Conversion Complete */
    "EE_READY", /* EEPROM Ready */
    "ANALOG_COMP", /* Analog Comparator */
    "TWI", /* Two-wire Serial Interface */
    "SPM_READY" /* Store Program Memory Read */
  ];

  interruptTable.forEach((interrupt, i) => {
    get('.interrupt-table').appendChild(
      create('option', {value: i}, interrupt)
    );
  });

  get('.send-irq').on('click', function() {
    let vect = get('.interrupt-table option:checked').value;
    if (vect === 'none') return;
    sendInterrupt(+vect);
  });

  get('.reset-app').on('click', () => {
    sendInterrupt(0);
  });

  get('.reset-mcu').on('click', () => {
    socket.emit('reset mcu');
  });

})();