=======To run Object Detection======

1) locate file path of key.json in this folder
   e.g. C:\Users\ooiko\Desktop\objectdetection\key.json	
2) nagivate to server folder
3) open cmd in that directory 
4) set the key.json path in the cmd 
   e.g. set GOOGLE_APPLICATION_CREDENTIALS=C:\Users\ooiko\Desktop\objectdetection\key.json
4) run the server in cmd by typing (node server.js)
5) wait for listen 8888
6) get the ipv4 address of this computer
7) go to objectdetection folder and launch objectdetection.ino
8) set your ip address in line 297 of the code
   e.g. client.begin("http://192.168.61.243:8888/imageUpdate")
9) set BOTtoken and CHAT_ID for telegram in line 15 and 16 of code respectively
   guide for setting up telegram bot:
   https://randomnerdtutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
10) change wifi credentials in line 43 and 44 respectively
11) compile and upload code to esp32cam

