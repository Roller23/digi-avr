(async () => {
  const socket = new WebSocket('ws://localhost:3000');
  const log = console.log;
  const get = selector => document.querySelector(selector);
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
    log('Connection closed');
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
    let span = document.createElement('span');
    span.innerText = data;
    get('.log-output').appendChild(span);
  });

  socket.on('console', data => {
    let span = document.createElement('span');
    span.innerText = data;
    get('.mcu-output').appendChild(span);
  });

  get('.compile-code').addEventListener('click', e => {
    let code = get('.code-area').value;
    socket.emit('compile asm', code);
  });

  get('.execute-cycle').addEventListener('click', e => {
    socket.emit('execute cycle');
  });

})();