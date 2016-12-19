/* Geekbot ESP8266 Server
 *
 * Designed to work with customized geekbotV2LineNavigator.ino
 * ie. https://github.com/robotgeek/geekbot/tree/esp8266_dev/geekbotV2LineNavigator
 *
 * Usage notes: Robot must be on and you must answer the first question on the LCD (Where am I starting?)
 * Destination commands will not be accepted while robot is reporting busy (traveling to or from destination)
 * Robot will automatically return home after the interval specified in LineNavigator sketch (for fetching objects)
 *
 */

/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/* Configure this sketch by editing these constants */

const int led = 5; //Pin 5 on ESP8266 Thing Dev, Pin 13 on others
const char *ssid = "YourWiFiSSID"; //Your network SSID
const char *password = "YourWiFiPassword"; //Your network password
const char *dns_name = "myGeekbot"; //Access the robot on your network with http://myGeekbot.local/
const int MAP_LOCATION_COUNT = 4; //Number of locations in destinationList below
const String destinationList[MAP_LOCATION_COUNT] = //List locations from LineNavigator/myMapp.cpp
{
	"Robot Lab",
	"Garage",
	"Kitchen",
	"Dogs Bed"
};

/* There is no need to edit below this line */

ESP8266WebServer server ( 80 ); //WebServer object running on port 80 (HTTP)

int home_index = 0; //Robot Status
int location_index = 0; //Robot Status
int is_robot_busy = 0; //Robot Status

void setup ( void )
{
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );

	Serial.begin ( 115200 );

	WiFi.begin ( ssid, password );
	//Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED )
	{
		delay ( 500 );
		//Serial.print ( "." );
	}

	/*
		Serial.println ( "" );
		Serial.print ( "Connected to " );
		Serial.println ( ssid );
		Serial.print ( "IP address: " );
		Serial.println ( WiFi.localIP() );
	*/

	if ( MDNS.begin ( dns_name ) )
	{
		//Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
	server.on ( "/test.svg", drawGraph );
	server.on ( "/inline", []()
	{
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();

	//Serial.println ( "HTTP server started" );
}

void loop ( void )
{
	server.handleClient();
	parseSerialData();
}

/* Check for data and parse robot status from serial port */
void parseSerialData()
{
	char buffer[64] = {0};
	int i = 0;

	if ( Serial.available() )
	{
		delay(100); //wait for remaining bytes
		while( Serial.available() && i < sizeof(buffer) )
		{
			buffer[i++] = Serial.read();
		}
		buffer[i++] = '\0';
	}

	if( i > 0 )
	{
		//Compare strings and perform actions
		if (strncmp(buffer, "robot:", 6) == 0)
		{
			home_index = atoi( buffer + 6 );
			location_index = atoi( buffer + 8 );
			is_robot_busy = atoi( buffer + 10 );
		}
	}
}

void handleRoot()
{
	digitalWrite ( led, 1 );
	char temp[1024];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	int bufferPosition = snprintf ( temp, 1024,
	                                "<html>\
  	<head>\
    	<!--<meta http-equiv='refresh' content='10'/>-->\
    	<title>Geekbot Navigator</title>\
   		<style>\
    		body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    	</style>\
 	</head>\
  	<body>\
    	<a href='./'><h1>Geekbot Navigator!</h1></a>\
    	<h3>Robot Busy?: %d</h3>\
    	<p/>Current Location: %s\
    	<p/>Home Location: %s\
    	<p>Uptime: %02d:%02d:%02d</p>\
    	<h4>Select Home Location:</h4>",
	                                is_robot_busy,
	                                destinationList[location_index].c_str(),
	                                destinationList[home_index].c_str(),
	                                hr, min % 60, sec % 60
	                              );

	for ( int i = 0; i < MAP_LOCATION_COUNT; ++i )
	{
		char charBuf[25] = {0};
		destinationList[i].toCharArray(charBuf, 25) ;
		bufferPosition += snprintf( temp + bufferPosition, 1024 - bufferPosition,
		                            "<p/><a href='/home%0d'>%0d. %s</a>"
		                            , i, i, charBuf
		                          );
	}

	bufferPosition += snprintf( temp + bufferPosition, 1024 - bufferPosition,
	                            "<h4>Select Destination:</h4>"
	                          );

	for ( int i = 0; i < MAP_LOCATION_COUNT; ++i )
	{
		char charBuf[25] = {0};
		destinationList[i].toCharArray(charBuf, 25) ;
		bufferPosition += snprintf( temp + bufferPosition, 1024 - bufferPosition,
		                            "<p/><a href='/go%0d'>%0d. %s</a>"
		                            , i, i, charBuf
		                          );
	}

	bufferPosition += snprintf( temp + bufferPosition, 1024 - bufferPosition,
	                            "<p/><img src=\"/test.svg\" />\
    </body>\
    </html>"
	                          );
	server.send ( 200, "text/html", temp );
	digitalWrite ( led, 0 );
}

/* Exploiting the not found handler and the auto generated URI data to command robot */
void handleNotFound()
{
	if ( !is_robot_busy )
	{
		if ( strncmp( server.uri().c_str(), "/go", 3 ) == 0 )
		{
			Serial.print( "dest:" );
			Serial.println( server.uri()[3] );
		}

		if ( strncmp( server.uri().c_str(), "/home", 5 ) == 0 )
		{
			Serial.print( "home:" );
			Serial.println( server.uri()[5] );
		}

		delay(2000);
	}

	parseSerialData();

	handleRoot();

	/*
	digitalWrite ( led, 1 );
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ )
	{
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
	digitalWrite ( led, 0 );
	*/
}

void drawGraph()
{
	String out = "";
	char temp[100];
	out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
	out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
	out += "<g stroke=\"black\">\n";
	int y = rand() % 130;
	for (int x = 10; x < 390; x += 10)
	{
		int y2 = rand() % 130;
		sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
		out += temp;
		y = y2;
	}
	out += "</g>\n</svg>\n";

	server.send ( 200, "image/svg+xml", out);
}
