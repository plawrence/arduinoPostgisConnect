arduinoPostgisConnect
=====================

This project allows an arduino unit to connect to a RESTful web interface and finally to a Postgis database

##The three parts of this project are as follows:
1. An Arduino script for sending logged data to our server.  The sent data will be accompanied by a hashed password to check for tampering.
2. The php script that resides on the server and receives the sent data.  Once the hash has been verified, the script will send the data to the PostGIS db, which is currently residing on the same server.
3. The PostGIS db, which currently has two tables - a datalogger table with Lat/Long locations, and a table for the sensor data that is linked via a foreign key to the datalogger table.