import serial
import os
import time
import pymysql.cursors

dbConn = pymysql.connect(
host='194.5.156.1',
user='u592694108_bioboxku',
password='moxjesdd2FF',
database='u592694108_biobox'
)

#koneksi database

cursor = dbConn.cursor()

##Koneksi Bluetooth


if os.path.exists('/dev/rfcomm1') == False:
    path = 'sudo rfcomm bind 1 00:20:10:08:64:51'
    os.system (path)
    time.sleep(1)

## Membaca data serial yang dikirimkan melalui bluetooth HC-05
bluetoothSerial = serial.Serial( "/dev/rfcomm1", baudrate=9600 )

## Mengirimkan dan Menerima data dari database
while True:
    time.sleep(4)
    cursor.execute("SELECT inputsuhu FROM setpoint")
    inputsuhu = (cursor.fetchone()[0])
    bluetoothSerial.write(str(inputsuhu).encode('utf-8'))
    time.sleep(2)
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