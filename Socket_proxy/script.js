(async () => {

  const get = selector => document.querySelector(selector);
  const getAll = selector => document.querySelectorAll(selector);

  let runInterval = undefined;

  const editorThemes = ['midnight', 'monokai', 'liquibyte'];

  const editor = CodeMirror(get('.code-wrap'), {
    lineNumbers: true,
    mode: 'text/x-c',
    theme: 'monokai',
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

  setInterval(() => {
    localStorage.lastCode = editor.getValue();
  }, 1000);

  const runMCU = () => {
    runInterval = setInterval(() => {
      socket.emit('execute cycle');
    }, 100);
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

})();