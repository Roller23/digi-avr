(async () => {
  const socket = new WebSocket('ws://localhost:3000');
  const log = console.log;

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
    // socket.emit('ping', 'Hello')
  });

  socket.on('pong', data => {
    log('Server: ', data);
  });

  socket.on('log', data => {
    log('Python log:', data);
  });

  socket.on('console', data => {
    let span = document.createElement('span');
    span.innerText = data;
    document.querySelector('.mcu-console').appendChild(span);
  });

})();