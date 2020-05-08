var net = require('net');

const http_reply = "HTTP/1.0 200 OK\r\n" +
    "Date: Thu, 07 May 2020 12:49:30 GMT\r\n" +
    "Connection: close\r\n" +
    "Accept-Ranges: bytes\r\n" +
    "Last-Modified: Thu, 07 May 2020 12:49:25 GMT\r\n" +
    "Content-Length: 0\r\n" +
    "\r\n";

var server = net.createServer(function(socket) {
    // process.stdout.write(`New ${socket.remoteFamily} client: ${socket.remoteAddress}:${socket.remotePort}\n`);
    socket.on('data', function (data) {
        // process.stdout.write(`Read ${socket.bytesRead} bytes from client ${socket.remoteAddress}:${socket.remotePort}\n`);
    });
    socket.on('error', function () {
    });
    socket.write(http_reply);
});

server.listen(8080, '127.0.0.1');