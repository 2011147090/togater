var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mysql = require('mysql');
var logger = require('morgan');
//var session = require('express-session');
var redis = require('redis');
var redisClient = redis.createClient(6379, '192.168.1.201');
//var redisClient = redis.createClient(6379, 'localhost');
var md5 = require('md5');
var sha256 = require('sha256');
var uuid = require('uuid/v4');
var nodemailer = require('nodemailer');

var mysqlPool = mysql.createPool({
    connectionLimit : 20,
    host : '192.168.1.251',
    //host : 'localhost',
    user : 'togater',
    password : 'togater',
    database : 'togater'
});

var smtpTransport = nodemailer.createTransport("SMTP", {
    service: 'Gmail',
    auth: {
        user: 'servercamp.togater',
        pass: 'togater123'
    }
});

var app = require('express')();
var server = require('http').Server(app);

var resetPasswordUserList = [];


server.listen(3000, function(){
  console.log('listen......');
});

app.use(express.static('public'));
app.use(cookieParser());
app.use(bodyParser.urlencoded({extended : false}));

/*app.use(session({
  secret : 'secret key',
  resave : false,
  saveUninitialized : true
}));*/

var headers = {
  "Content-Type": "text/html",
  'ACCESS-CONTROL-ALLOW-CREDENTIALS' : 'true',
  'ACCESS-CONTROL-ALLOW-ORIGIN' : '*',
  'ACCESS-CONTROL-ALLOW-METHODS' : 'GET, POST, OPTIONS',
  'ACCESS-CONTROL-ALLOW-HEADERS' : 'Accept, X-Access-Token, X-Application-Name, X-Request-Sent-Time'
};

    //TODO
    /*
    로그인 시도
    디비 정보와 로그인 정보 비교 (디비에는 id 솔트, password 솔트 2개 존재, 솔트는 회원가입할때 UUID를 솔트로 이용)
    로그인 성공 시 아이디와 솔트를 합한 해시값 생성(md5 : 32자, sha256 : 64자)
    생성된 해시값을 클라이언트에 전달, 서버 레디스에 저장
    */

app.get('/', function(request, response){
  response.send('/public/index.html');
});

//TODO 레디스에서 아이디를 가져오는게 아니라 솔트를 가져와야 한다.
app.post('/login', function (request, response) {
redisClient.get(md5(sha256(request.body.id)), function(err, reply) {
    if(reply === null){

      var userID = request.body.id;
      var encryptedPassword = sha256(request.body.password);

      mysqlPool.getConnection(function(err, connection){
          connection.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [userID, encryptedPassword] , function(err, secondRows){
            response.writeHead(200, headers);

            if(secondRows.length === 1)   //만약 로그인에 성공했다면
            {
                var accessToken = md5(sha256(userID));
                response.end('ok:' + accessToken);
                redisClient.set(accessToken, userID);
                console.log('Success Login : ' + userID);
            }
            else if(secondRows.length === 0)  //로그인 실패
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
      console.log('Already Login : ' + request.body.id);
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


app.post('/forgot/password/request', function(request, response){
  let id = request.body.id;
  let resetUUID = uuid();

  mysqlPool.getConnection(function(err, connection){
      connection.query('select email from user_info where id =  BINARY(?)', [id] , function(err, rows){
        console.log(rows[0].email);
          let mailOptions = {
              from: '로그인 서버 <servercamp.togater@gmail.com>',
              to: rows[0].email,
              subject: 'TOGATER 로그인 서버 입니다.',
              html: '비밀번호를 재설정 하려면 아래 토큰을 해당 페이지에서 입력 후 재설정 하시기 바랍니다. <p>토큰 : ' + resetUUID + '</p>'
          };

          smtpTransport.sendMail(mailOptions, function(err, res){
              if (err){
                  console.log(err);
              } else {
                  console.log("Message sent : " + res.message);

                  response.send('sent email');
                  redisClient.set(resetUUID, id);
              }
          });
          connection.release();
      });
  });
});

app.post('/forgot/password/auth', function(request, response){

  let uuid = request.body.uuid;

  redisClient.get(uuid, function(err, reply) {
      if(reply){
           response.send('success auth');
           redisClient.del(uuid);
      }else{
           response.send('fail reset password');
      }
  });
});

app.post('/forgot/password/reset', function(request, response){
  let id = request.body.id;
  let newPassword = sha256(request.body.password);

  mysqlPool.getConnection(function(err, connection){
      connection.query('update user_info set password = BINARY(?) where id =  BINARY(?)', [newPassword, id] , function(err, rows){
          response.send('success');
          connection.release();
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
      var userID = request.body.id;
      var password = request.body.password;
      var email = request.body.email;
      var encryptedPassword = sha256(password);

      connection.query('insert into user_info (id, password, email, win, lose, rating) values ( BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?))',
      [userID, encryptedPassword, email, 0, 0, 400], function(err, rows){
          response.writeHead(200, headers);
          response.end('register ok');

          connection.release();

          if(err)
            console.log(err);
      });
  });
});

app.post('/dashboard/logout', function(request, response){
  redisClient.del(request.body.userToken);
  response.send('redis delete');
});

app.post('/dashboard/withdrawal', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('delete from user_info where id =  BINARY(?)', [reply] , function(err, rows){
                if(err)
                    console.log(err);
                else{
                    redisClient.del(request.body.userToken);
                    response.send('success');
                }
            connection.release();
              });
          });
      }
  });
});

app.post('/dashboard/profile', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('select win, lose, rating from user_info where id =  BINARY(?)', [reply] , function(err, rows){

                  response.send({
                      "win" : rows[0].win,
                      "lose" : rows[0].lose,
                      "rating" : rows[0].rating
                  });
            connection.release();
              });
          });
      }
  });
});


app.post('/dashboard/account', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('select id, email from user_info where id =  BINARY(?)', [reply] , function(err, rows){

                  response.send({
                      "ID" : rows[0].id,
                      "email" : rows[0].email
                  });
            connection.release();
              });
          });
      }
  });
});
