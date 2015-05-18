<?php

?>
<!DOCTYPE html>
<html lang="en">
<head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">

<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>Home Automation</title>
<link href='https://fonts.googleapis.com/css?family=Josefin+Sans:400' rel='stylesheet' type='text/css'>
<style type="text/css">
	body {
		background-color: #eee;
	}
	.main {
		font-family: 'Josefin Sans', Arial, sans-serif;
		font-size: 82px;
		font-weight: 100;
		color: #777;
		text-align: center;
		padding-top: 50px;
		letter-spacing: 3px;
	}
	.mini {
		font-size: 12px;
		text-align: left;
		display: inline-block;
	}

    .formLayout {
    	font-size: 12px;
		text-align: left;
		display: inline-block;
		
        background-color: #f3f3f3;
        border: solid 1px #a1a1a1;
        padding: 20px;
        width: 500px;
    }
    
    .formLayout label, .formLayout button {
        display: block;
        width: 100px;
        float: left;
        margin-bottom: 5px;
        position: relative;
  		top: 50%;
  		transform: translateY(-50%);
    }
 
    .formLayout label {
        text-align: right;
        padding-right: 20px;
    }
 
    br {
        clear: left;
    }
</style>
<!--[if IE]><script src="http://html5shiv.googlecode.com/svn/trunk/html5.js"></script><![endif]-->
	<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js" type="text/javascript"></script>
    <script type="text/javascript" src="lib/mqttws31.js"></script>
    <script type="text/javascript" src="config.js"></script>
    <script type="text/javascript">
		
		//MQTT Stuff 
		function MQTTConnect() {
			mqtt = new Paho.MQTT.Client(host,port,"web_" + parseInt(Math.random() * 100,10));
			var options = {
				timeout: 3,
				useSSL: useTLS,
				cleanSession: cleansession,
				onSuccess: onConnect,
				onFailure: onFailure,
				userName: username,
				password: password
			};

			mqtt.onConnectionLost = onConnectionLost;
			mqtt.onMessageArrived = onMessageArrived;
			console.log("Host="+ host + ", port=" + port + " TLS = " + useTLS + " username=" + username + " password=" + password);
			mqtt.connect(options);
		}
		function onMessageArrived(message) {
			console.log("onMessageArrived:"+message.payloadString);
			document.getElementById('STATUS').innerHTML = message.payloadString + "<br />" + document.getElementById('STATUS').innerHTML;
			//check to see if payload is JSON
			try {
				var NO_OUTPUT = "false";
				var msgjson = jQuery.parseJSON(message.payloadString); //parse payload
				var output = "<label>Device"
				var NOTFIRST = "false";
				for (var key in msgjson) {
					var device=key;
					output = output + key + ": </label>";
					console.log("DEVICE:" + device);
					var adress=msgjson[key]["Adress"];
					console.log("Adress:" + adress);
					//Check if the message is on or off status message.
					for (var OL in msgjson[key]) {
						if(msgjson[key][OL] === "ON" || msgjson[key][OL] === "OFF") {
							NO_OUTPUT = "true";
							console.log("ON or OFF");
						}
					}
					//check if the device is offline (LWT message from prev connected device)
					if(msgjson[key]["Capability"] === "OFFLINE") {
						NO_OUTPUT = "true";
						console.log("OFFLINE");
					}
					for (var key2 in msgjson[key]["Capability"]) {
						var GPIO=msgjson[key]["Capability"][key2];
						if(NOTFIRST==="true") {
							output = output + "<label></label>";
						}
						output = output + "<label>" + key2 +"</label>"+ "<button onclick='publish(\"{\\\""+key2+"\\\":\\\"ON\\\"}\",\""+adress+"\",2);'>ON</button>";
						output = output + "<button onclick='publish(\"{\\\""+key2+"\\\":\\\"OFF\\\"}\",\""+adress+"\",2);'>OFF</button>";
						output = output + "<br />";
						NOTFIRST="true";
					}
					if(NOTFIRST==="false") {
						output = output + "<br />";
					}
					//IF LWT or status message do not change the buttons.
					if(NO_OUTPUT !== "true") {
						document.getElementById("BUTTONS").innerHTML = output + document.getElementById("BUTTONS").innerHTML;
						console.log(output);
					}		
				}
			} catch (e) {
				console.log(e);
				return false;
			};

		}
		function onConnect() {
			// Once a connection has been made, make a subscription and send a message.
			document.getElementById("BUTTONS").innerHTML = "";
			console.log("onConnect Sub to: iot/switch/+/status");
			mqtt.subscribe("iot/switch/+/status", {qos:1,onSuccess:onSubscribe,onFailure:onSubscribeFailure});
		};
		function onSubscribe(x) {
		  console.log('Subscribed');
		}
		function onSubscribeFailure(x) {
		  console.log('Subscribe failed: ' + message.errorMessage);
		}
		function onFailure(responseObject) {
			console.log("onFailure: " + responseObject.errorMessage + "Retrying");
			setTimeout(MQTTConnect, reconnectTimeout);
		}
		function onConnectionLost(responseObject) {
			document.getElementById("BUTTONS").innerHTML = "<br />Connection Lost<br />";
		  if (responseObject.errorCode !== 0)
		    console.log("onConnectionLost:"+responseObject.errorMessage);
		    setTimeout(MQTTConnect, reconnectTimeout);
		}
		function getURLParameters(paramName) {
			var sURL = window.document.URL.toString();  
		    if (sURL.indexOf("?") > 0)
		    {
		       var arrParams = sURL.split("?");         
		       var arrURLParams = arrParams[1].split("&");      
		       var arrParamNames = new Array(arrURLParams.length);
		       var arrParamValues = new Array(arrURLParams.length);     
		       var i = 0;
		       for (i=0;i<arrURLParams.length;i++)
		       {
		        var sParam =  arrURLParams[i].split("=");
		        arrParamNames[i] = sParam[0];
		        if (sParam[1] != "")
		            arrParamValues[i] = unescape(sParam[1]);
		        else
		            arrParamValues[i] = "No Value";
		       }
		
		       for (i=0;i<arrURLParams.length;i++)
		       {
		                if(arrParamNames[i] == paramName){
		            //alert("Param:"+arrParamValues[i]);
		                return arrParamValues[i];
		             }
		       }
		       return null;
		    }
		
		}
		var publish = function (payload, topic, qos) {
			console.log("Topic: " + topic + "Publish: " + payload);
			var message = new Paho.MQTT.Message(payload);
			message.destinationName = topic;
			message.qos = qos;
			mqtt.send(message);
 		}
		//Start The Goddamn Program
		var username = getURLParameters("username");
		var password = getURLParameters("password");
		MQTTConnect();
    </script>
    
    
  </head>
<body id="index" class="home">
	<div class="main">
		Home Automation<br />
		<div class="formLayout" id="BUTTONS">

		</div>
		<br />
		<div class="mini">
			<label id="STATUS"></label>
		<br />
		mikael@murf.se 2015
		</div>
	</div>
</body>
</html>