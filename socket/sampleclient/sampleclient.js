var net = require('net');
//创建TCP客户端

var client = new net.Socket();
client.setEncoding('utf8');
client.connect(8000,'localhost',function () {
    console.log('已连接到服务器');
    client.write('你好');
});

client.on('data',function (data) {
    console.log('已接受服务器端发送的数据：'+data);
});

client.on('end', function(over){

    console.log("connect finished");
    while(1);
})