#### Tiny Web server

This is the home directory for the Tiny server, a 200-line Web
Tiny uses the GET method to serve static content
(text, HTML, GIF, and JPG files) out of `./` and to serve dynamic
content by running CGI programs out of `./cgi-bin`. The default 
page is `home.html` (rather than index.html) so that we can view
the contents of the directory from a browser.

To run Tiny:
   Run `tiny <port>` on the server machine, 
	e.g., `tiny 8000`.
   Point your browser at Tiny: 
	static content: `http://<host>:8000`
	dynamic content: `http://<host>:8000/cgi-bin/adder?1&2`

Files:
  `tiny.c`		The Tiny server
  `Makefile`		Makefile for `tiny.c`
  `home.html`		Test HTML page
  `text.html`		Test HTML page (text only)
  `wsu.png`		Image embedded in `home.html`
  `cougs.jpg`		Static content (image)
  `README`		This file	
  `cgi-bin/adder.c`	CGI program that adds two numbers
  `cgi-bin/Makefile`	Makefile for `adder.c`

