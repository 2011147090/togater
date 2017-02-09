var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mysql = require('mysql');
var logger = require('morgan');
var session = require('express-session');
var redis = require('redis');
//var redisClient = redis.createClient(6379, '192.168.1.201');
var redisClient = redis.createClient(6379, 'localhost');
var cluster = require('cluster');
var os = require('os');
var md5 = require('md5');
var sha256 = require('sha256');
var uuid = require('uuid/v4');
//var aes256 = require('aes256');


var mysqlPool = mysql.createPool({
    connectionLimit : 20,
    //host : '192.168.1.203',
    host : 'localhost',
    user : 'root',
    password : '151526',
    database : 'game'
});

var app = require('express')();
var server = require('http').Server(app);

if(cluster.isMaster){
  for(var i = 0 ; i < os.cpus().length ; i++){
    cluster.fork();
  }
}
else{
  server.listen(3000, function(){
      console.log('listen......');
  });

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

        //TODO
        /*
        로그인 시도
        디비 정보와 로그인 정보 비교 (디비에는 id 솔트, password 솔트 2개 존재, 솔트는 회원가입할때 UUID를 솔트로 이용)
        로그인 성공 시 아이디와 솔트를 합한 해시값 생성(md5 : 32자, sha256 : 64자)
        생성된 해시값을 클라이언트에 전달, 서버 레디스에 저장
        */

  app.get('/', function(request, response){
      response.send('/public/index.html')
  });
  app.get('/test', function(request, response){
      console.log('test');
      response.send('hello');
  });

//TODO 레디스에서 아이디를 가져오는게 아니라 솔트를 가져와야 한다.
app.post('/login', function (request, response) {
    redisClient.get(request.body.id, function(err, reply) {
        if(reply === null){
          console.log('this id is not login');

          var userID = request.body.id;
          var password = request.body.password;

          mysqlPool.getConnection(function(err, connection){
            connection.query('select password_salt from user_info where id =  BINARY(?)', [userID] , function(err, firstRows){
              if(firstRows.length === 1){
                var password_salt = firstRows[0].password_salt;
                var encryptedPassword = md5(password + password_salt);

                connection.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [userID, encryptedPassword] , function(err, secondRows){
                  response.writeHead(200, headers);

                  if(secondRows.length === 1)   //만약 로그인에 성공했다면
                  {
                      console.log(secondRows[0].id_salt);

                      var accessToken = md5(userID + secondRows[0].id_salt)
                      response.end('ok:' + accessToken);

                      //redisClient.set(request.session.id, request.session.userID);
                      //redisClient.set(request.session.userID, 'login');
                  }
                  else if(secondRows.length === 0)  //로그인 실패
                  {
                      response.end('login fail');
                  }
                  connection.release();

                  if(err)
                    console.log(err);
                });
              }
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



  // app.post('/login', function (request, response) {
  //     redisClient.get(request.body.id, function(err, reply) {
  //         if(reply === null){
  //           console.log('this id is not login');
  //
  //           var userID = request.body.id;
  //           var password = request.body.password;
  //
  //           console.log(encryptedPassword);
  //           mysqlPool.getConnection(function(err, connection){
  //               connection.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [userID, encryptedPassword] , function(err, rows){
  //                 response.writeHead(200, headers);
  //
  //                 if(rows.length === 1)   //만약 로그인에 성공했다면
  //                 {
  //                     console.log(rows[0].id_salt);
  //
  //                     var accessToken = md5(userID + rows[0].id_salt)
  //                     response.end('ok:' + accessToken);
  //                     //request.session.userID = request.body.id;
  //
  //                     //console.log('login : ' + request.session.id);
  //                     //console.log('sessionID : ' + request.session.userID);
  //                     redisClient.set(request.session.id, request.session.userID);
  //                     redisClient.set(request.session.userID, 'login');
  //                 }
  //                 else if(rows.length === 0)  //로그인 실패
  //                 {
  //                     response.end('login fail');
  //                 }
  //                 connection.release();
  //
  //                 if(err)
  //                   console.log(err);
  //               });
  //           });
  //         }
  //         else{
  //           console.log('this id is login');
  //           response.writeHead(200, headers);
  //           response.end('already login');
  //         }
  //     });
  // });

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

  // app.post('/forgot/password', function(request, response){
  //     mysqlPool.getConnection(function(err, connection){
  //         connection.query('select password from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
  //           response.writeHead(200, headers);
  //           if(rows.length === 1)   //만약 비밀번호를 찾았다면
  //           {
  //               var encryptedPassword = rows[0].password;
  //               var decryptedPassword = aes256.decrypt(key, encryptedPassword);
  //               response.end(decryptedPassword);
  //               console.log('Find password : ' + decryptedPassword);
  //           }
  //           else if(rows.length === 0)  //찾기 실패
  //           {
  //               response.end('find fail');
  //           }
  //           connection.release();
  //
  //           if(err)
  //             console.log(err);
  //         });
  //     });
  // });

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
          var id_salt = uuid();
          var password_salt = uuid();
          var encryptedPassword = md5(password + password_salt)

          connection.query('insert into user_info (id, password, email, win, lose, id_salt, password_salt, rating) values ( BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?))',
          [userID, encryptedPassword, email, 0, 0, id_salt, password_salt, 500], function(err, rows){
              response.writeHead(200, headers);
              response.end('register ok');

              connection.release();

              if(err)
                console.log(err);
          });
      });
  });
}
