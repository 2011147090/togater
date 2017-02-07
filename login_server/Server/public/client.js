var socket = io();

var execInUnity = function(method) {
 		var args = Array.prototype.slice.call(arguments, 1);
 		SendMessage("Chatting Manager", method, args.join(','));
};

socket.on('chat', function(message){
    execInUnity('RecvChatMessage', message);
});
