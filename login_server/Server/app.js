var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mysql = require('mysql');
var logger = require('morgan');
var session = require('express-session');
var redis = require('redis');
var redisClient = redis.createClient(6379, '192.168.1.201');
var cluster = require('cluster');
var os = require('os');

var mysqlPool = mysql.createPool({
    connectionLimit : 20,
    host : '192.168.1.203',
    user : 'root',
    password : '123123',
    database : 'game'
});

var app = require('express')();
var server = require('http').Server(app);
//var io = require('socket.io')(server);
// this is test

if(cluster.isMaster){
  for(var i = 0 ; i < 4 ; i++){
    cluster.fork();
  }
}
else{
  server.listen(3000, function(){
      console.log('listen......');
  });

  /*
  var app = express();
  var http = require('http').Server(app);
  var io = require('socket.io')(http);

  app.listen(3000, function(){
      console.log('Connected 3000 port!')
  });
  */

  //app.use(logger('dev'));
  app.use(express.static('public'));
  app.use(cookieParser());
  app.use(bodyParser.urlencoded({extended : false}));
  app.use(session({
      secret : 'secret key',
      resave : false,
      saveUninitialized : true
  }));

  var headers = {
      "Content-Type": "text/html",
      'ACCESS-CONTROL-ALLOW-CREDENTIALS' : 'true',
      'ACCESS-CONTROL-ALLOW-ORIGIN' : '*',
      'ACCESS-CONTROL-ALLOW-METHODS' : 'GET, POST, OPTIONS',
      'ACCESS-CONTROL-ALLOW-HEADERS' : 'Accept, X-Access-Token, X-Application-Name, X-Request-Sent-Time'
  };
  /*io.on('connection', function(socket){
      console.log('connected client....');

      socket.on('chat', function(message){
          socket.broadcast.emit('chat', message);
      });
  });*/

//TODO : 로그아웃 기능이 필요하다. 로그인 시에 세션을 생성하는데 로그아웃을 따로 만들어서 로그인에서 만든 세션을 삭제 해주어야 한다.

  app.get('/', function(request, response){
      response.send('/public/index.html')
  });
  app.get('/test', function(request, response){
      console.log('test');
      response.send('hello');
  });

  app.post('/login', function (request, response) {
      //console.log('id : ' + response.body.id);
      //console.log('password' + response.body.password);
      redisClient.get(request.body.id, function(err, reply) {
          if(reply === null){
            console.log('this id is not login');

            mysqlPool.getConnection(function(err, connection){
                connection.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [request.body.id, request.body.password] , function(err, rows){
                  response.writeHead(200, headers);
                  if(rows.length === 1)   //만약 로그인에 성공했다면
                  {
                      response.end('ok:' + request.session.id);
                      request.session.userID = request.body.id;

                      console.log('login : ' + request.session.id);
                      console.log('sessionID : ' + request.session.userID);
                      redisClient.set(request.session.id, request.session.userID);
                      redisClient.set(request.session.userID, 'login');
                  }
                  else if(rows.length === 0)  //로그인 실패
                  {
                      response.end('login fail');
                  }
                  connection.release();

                  if(err)
                    console.log(err);
                });
            });
          }
          else{
            console.log('this id is login');
            response.writeHead(200, headers);
            response.end('already login');
          }
      });
  });

  app.post('/login/webpage', function(request, response){

  });

  app.post('/forgot/id', function(request, response){
      mysqlPool.getConnection(function(err, connection){
          connection.query('select id from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
              response.writeHead(200, headers);

              if(rows.length === 1)   //만약 ID를 찾았다면
              {
                  response.end(rows[0].id);
                  console.log('Find ID : ' + rows[0].id);
              }
              else if(rows.length === 0)  //찾기 실패
              {
                  response.end('find fail');
              }
              connection.release();
              if(err)
                console.log(err);
          });
      });
  });

  app.post('/forgot/password', function(request, response){
      mysqlPool.getConnection(function(err, connection){
          connection.query('select password from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
            response.writeHead(200, headers);
            if(rows.length === 1)   //만약 비밀번호를 찾았다면
            {
                response.end(rows[0].password);
                console.log('Find password : ' + rows[0].password);
            }
            else if(rows.length === 0)  //찾기 실패
            {
                response.end('find fail');
            }
            connection.release();

            if(err)
              console.log(err);
          });
      });
  });

  app.post('/register/overlap/email', function (request, response) {
      mysqlPool.getConnection(function(err, connection){
          connection.query('select * from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
            response.writeHead(200, headers);
            if(rows.length === 1)   //email 중복
            {

                response.end('email overlap');
            }
            else if(rows.length === 0)  //중복 아님
            {
                response.end('email ok');
            }
            connection.release();

            if(err)
              console.log(err);
          });
      });
  });

  app.post('/register/overlap/id', function (request, response) {
      mysqlPool.getConnection(function(err, connection){
          connection.query('select * from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
            response.writeHead(200, headers);
            if(rows.length === 1)   //id 중복
            {
                response.end('id overlap');
            }
            else if(rows.length === 0)  //중복 아님
            {
                response.end('id ok');
            }

            connection.release();

            if(err)
              console.log(err);
          });
      });
  });

  app.post('/register', function (request, response) {
      mysqlPool.getConnection(function(err, connection){
          connection.query('insert into user_info (id, password, email, win, lose, rating) values( BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?))',
          [request.body.id, request.body.password, request.body.email, 0, 0, 500], function(err, rows){
              console.log(err);

              response.writeHead(200, headers);
              response.end('register ok');

              connection.release();

              if(err)
                console.log(err);
          });
      });
  });
}
