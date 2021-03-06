/*
 * Red Pitaya Template Application
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */


(
function(APP, $, undefined) {
    
    // App configuration
    APP.config = {};
    APP.config.app_id = 'redpitaya-serial-monitor';
    APP.config.app_url = '/bazaar?start=' + APP.config.app_id + '?' + location.search.substr(1);
    APP.config.socket_url = 'ws://' + window.location.hostname + ':9002';

    // WebSocket
    APP.ws = null;




    // Starts template application on server
    APP.startApp = function() {

        $.get(APP.config.app_url)
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    APP.connectWebSocket();
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    APP.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    APP.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                APP.startApp();
            });
    };




    APP.connectWebSocket = function() {

        //Create WebSocket
        if (window.WebSocket) {
            APP.ws = new WebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            APP.ws = new MozWebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }


        // Define WebSocket event listeners
        if (APP.ws) {

            APP.ws.onopen = function() {
                console.log('Socket opened');               
            };

            APP.ws.onclose = function() {
                console.log('Socket closed');
            };

            APP.ws.onerror = function(ev) {
                //$('#hello_message').text("Connection error");
                console.log('Websocket error: ', ev);         
            };

            APP.ws.onmessage = function(ev) {
				var data = new Uint8Array(ev.data);
				var inflate = pako.inflate(data);
				var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
				var receive = JSON.parse(text);
				
				if (receive.parameters && receive.parameters.datain) {
					$('#console').val($('#console').val() + receive.parameters.datain.value);
					console.log('Message recieved', receive);
				}
            };
        }
    };

}(window.APP = window.APP || {}, jQuery)
);




// Page onload event handler
$(function() {
	$('#send').click(function() {
		let lineEnding = '';
		switch($('#newline').val()) {
			case 'lf': lineEnding = '\n'; break;
			case 'cr': lineEnding = '\r'; break;
			case 'crlf': lineEnding = '\r\n'; break;
		}
		let payload = {};
		payload.parameters = {};
		payload.parameters['dataout'] = {value: $('#message').val() + lineEnding};
		APP.ws.send(JSON.stringify(payload));
	});
    // Start application
    APP.startApp();
});
