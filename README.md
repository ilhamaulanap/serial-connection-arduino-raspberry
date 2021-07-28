# serial-connection-arduino-raspberry
How to Connect Arduino with Raspberry via bluetooth through serial connection and send the data to MySQL database

## Tools
Arduino IDE,
Python,
pymysql

## How it Works

Arduino Ide Function to receive data from Raspberry through serial port
```ino
void inputSuhu() 
{
  if ( Serial.available() > 0 ) {
     //Membaca input suhu yang dinginkan dari port serial
    inputsuhu = Serial.parseInt();
    suhuSetPoint = int(inputsuhu);
}
}
```
Arduino Ide Function to send data to Raspberry through serial port
```ino
void kirimdata() {
  Serial.print(String(suhuSetPoint));
  Serial.print(',');
  Serial.print(String(suhuatas));
  Serial.print(',');
  Serial.print(String(deltainputsuhu));
  Serial.print(',');
  Serial.print(String(deltasuhu));
  Serial.print(',');
  Serial.print(String(outputKipas));
  Serial.print(',');
  Serial.print(String(outputPeltier));
  Serial.print(',');
  Serial.print(String(pwmValueK));
  Serial.print(',');
  Serial.print(String(pwmValueP));
  Serial.print(',');
  Serial.print(String(outputFuzzyK));
  Serial.print(',');
  Serial.print(String(outputFuzzyP));
  Serial.println();
  delay(5000);
}
```
Raspberry python function to connect with arduino Bluetooth HC-05
```py
if os.path.exists('/dev/rfcomm1') == False:
    path = 'sudo rfcomm bind 1 00:20:10:08:64:51'
    os.system (path)
    time.sleep(1)

## Membaca data serial yang dikirimkan melalui bluetooth HC-05
bluetoothSerial = serial.Serial( "/dev/rfcomm1", baudrate=9600 )
```
change `/dev/rfcomm1` to your Raspberry Bluetooth Port
change `00:20:10:08:64:51` to your Arduino Bluetooth HC-05 Adress

Raspberry python function to connect with Mysql Database
```py
import serial
import os
import time
import pymysql.cursors

dbConn = pymysql.connect(
host='194.5.156.1',
user='u592694108_bioboxku',
password='********',
database='u592694108_biobox'
)

#koneksi database

cursor = dbConn.cursor()
```
change `the host, user,password, and database` to your own database

Raspberry python function to send data to arduino from Mysql Databse
```py
while True:
    time.sleep(4)
    cursor.execute("SELECT inputsuhu FROM setpoint")
    inputsuhu = (cursor.fetchone()[0])
    bluetoothSerial.write(str(inputsuhu).encode('utf-8'))
    time.sleep(2)
}
```
Raspberry python function to receive data from arduino and send it to mysql database
```py
    data = bluetoothSerial.readline().strip().decode('utf-8')
    pieces = data.split(",")
    datainputsuhu = int(float(pieces[0]))
    datasuhuatas  = (float(pieces[1]))
    datadeltasuhu = (float(pieces[2]))
    datadeltainput = (float(pieces[3]))
    datapwmK = (float(pieces[4]))
    datapwmP = (float(pieces[5]))
    print(datainputsuhu)
    print(datasuhuatas)
    sqlinsert = "INSERT INTO biobox (inputsuhu,suhuatas) VALUES (%s,%s)"
    val = (datainputsuhu, datasuhuatas)
    cursor.execute(sqlinsert, val)
    dbConn.commit() #commit the insert
    print(cursor.rowcount, "sudah terecord")
```
hange `INSERT INTO biobox (inputsuhu,suhuatas) VALUES (%s,%s)` to your own SQL query

## Contributing

Yes, please!

## License

MIT
