#### CPTS 360 Lab 5

Base code and documentation for WSU CPTS360 Lab 5 (Web Proxy)

`proxy.c`
`csapp.h`
`csapp.c`
    These are starter files.  `csapp.c` and `csapp.h` are described in
    your textbook. You may make any changes you like to these files.  
    You may create and submit any additional files you like. 
    You may use `port-for-user.pl` or `free-port.sh` to generate
    unique ports for your proxy or tiny server. 

`Makefile`
    This is the makefile that builds the proxy program.  Type `make`
    to build your solution, or `make clean` followed by `make` for a
    fresh build. 

`port-for-user.pl`
    Generates a random port for a particular user
    usage: `./port-for-user.pl <userID>`

`free-port.sh`
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: `./free-port.sh`

`tiny`
    Tiny Web server from the CSAPP textbook

