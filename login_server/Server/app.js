var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mysql = require('mysql');
var logger = require('morgan');
var session = require('express-session');
var redis = require('redis');
var redisClient = redis.createClient(6379, '192.168.1.201');
//var redisClient = redis.createClient(6379, 'localhost');
var mysqlClient = mysql.createConnection({user : 'root', password : '151526'});

var app = express();
mysqlClient.query("USE game");

app.use(logger('dev'));
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

app.get('/', function(request, response){
    response.send('/public/index.html')
});

app.post('/login', function (request, response) {
    mysqlClient.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [request.body.id, request.body.password] , function(err, rows){
        response.writeHead(200, headers);
        if(rows.length === 1)   //만약 로그인에 성공했다면
        {
            response.end('ok:' + request.session.id);
            request.session.userID = request.body.id;

            console.log(request.session.id);
            console.log(request.session.userID);
            redisClient.set(request.session.id, request.session.userID);
        }
        else if(rows.length === 0)  //로그인 실패
        {
            response.end('login fail');
        }
    });
});

app.post('/forgot/id', function(request, response){
  mysqlClient.query('select id from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
      response.writeHead(200, headers);
      if(rows.length === 1)   //만약 ID를 찾았다면
      {
          response.end(rows[0].id);
          console.log('Find password : ' + rows[0].id);
      }
      else if(rows.length === 0)  //찾기 실패
      {
          response.end('find fail');
      }
  });
});

app.post('/forgot/password', function(request, response){
  mysqlClient.query('select password from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
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
  });
});

app.post('/register/overlap/email', function (request, response) {
    mysqlClient.query('select * from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
        response.writeHead(200, headers);
        if(rows.length === 1)   //email 중복
        {
            response.end('email overlap');
        }
        else if(rows.length === 0)  //중복 아님
        {
            response.end('email ok');
        }
    });
});

app.post('/register/overlap/id', function (request, response) {
    mysqlClient.query('select * from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
        response.writeHead(200, headers);
        if(rows.length === 1)   //id 중복
        {
            response.end('id overlap');
        }
        else if(rows.length === 0)  //중복 아님
        {
            response.end('id ok');
        }
    });
});

app.post('/register', function (request, response) {
    mysqlClient.query('insert into user_info (id, password, email, win, lose) values( BINARY(?),  BINARY(?),  BINARY(?), BINARY(?),  BINARY(?))',
    [request.body.id, request.body.password, request.body.email, 0, 0],
    function(err, rows){
        console.log(err);
    });
    response.writeHead(200, headers);
    response.end('register ok');
    /*mysqlClient.query('select * from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
        if(rows.length === 1)   //id 중복
        {
            response.writeHead(200, {'Content-Type' : 'text/html'});
            response.end('fail');
        }
        else if(rows.length === 0)  //회원가입
        {
            response.writeHead(200, {'Content-Type' : 'text/html'});
            response.end('succ');
            console.log(request.session.id);
            mysqlClient.query('insert into user_info (id, password, email, win, lose) values( BINARY(?),  BINARY(?),  BINARY(?), BINARY(?),  BINARY(?))',[request.body.id, request.body.password, request.body.email, 0, 0]);
        }
    });*/
});

app.listen(3000, function(){
    console.log('Connected 3000 port!')
});
