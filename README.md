# DistributedComputing
Client-Server-Workers distributed computing software for a preprogrammed physics problem.

This project is a basic example of distributed computing software. It is comprized of:
1) Server -> It coordinates calculation and hands out the calculation parts
2) Worker -> It handles simulation
3) Client -> It triggers calculation by sending a calculate request to the server

To run this project you need to:
1) Start up the Server.exe
2) Run as many Worker.exe as you wish. Worker's/Client's connect to IP is hardcoded, you need to change it in code: 
  - default ip: "127.0.0.1", ports: 6633 and 6635
3) Run Client.exe to initialize calculations


Physics problem analysed:
Influence of radiation on cell system
