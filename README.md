This vDC for DigtalSTROM smarthome system provides a modbus interface for Helios KWL devices. Tested with Helios EC200/300 W, but probably also with other models which support Helios EasyControls3.
The interaction between DigitalStrom system and this vDC is done bei calling scenes which are configured through a config file of the VDC. Additionally the vDC sends the value of the two Helios device internal sensors
internalTemperature and internalHumidity to DigitalSTROM system. This is currently hardcoded.  In future versions I will add possibility to add more sensor values through the config file.

Integration example in DigitalSTROM configurator:

 - create a "userdefined state" based on value of a CO2 sensor in your DigitalSTROM smarthome (e.g. Netatmo device) with two states "CO2 high" and "CO2" low
 - create two new "scene responders" one reacting on userdefined state "CO2 high" and calling the scene of this vDC which is configured to execute the "home" or "boost" profile in the Helios device
  and one reacting on userdefined state "CO2 low" and calling the scene of this vDC which is configured to execute the "away" profile in the Helios device
  

Configuration
--------------

This vDC requires a configuration file called helios.cfg in the same folder as the vDC.
If you start the vDC without an existing configuration file, a new one will be created containing the required parameters. vDC will terminate after this.
Change the parameters according to your requirements and start the vDC again.

Following is a description of each parameter:

vdcdsuid  -> this is a unique DS id and will be automatically created; just leave empty in config file
reload_values -> time in seconds after which new values are pulled from helios device
zone_id   -> DigitalStrom zone id
debug     -> Logging level for the vDC  - 7 debug / all messages  ; 0 nearly no messages;

Section "kwl" contains the KWL device configuration:

kwl : id -> some alphanumeric ID identifying the device (e.g. model). 
             
kwl : name -> Any name for your KWL device

kwl : scenes : s0 to s19 (current maximum is 20 scenes)
        dsId -> Id of the DigitalStrom scene ; Scene 1 is dsId = 5, Scene 2 is dsID = 17, Scene 3 is dsId = 18, Scene 4 is dsId = 19 (see table 1 below)
        modbus : m0 to m09 (current maximum is 10 modbus values)
          register -> modbus register to be changed / written; for documentation of the Helios KWL modbus registers and allowed values see Helios documentation in folder helios-modbus-docu
          value -> value to be written into the modbus register


Section "sensor_values" contains the KWL values which should be reported as value sensor ("Sensorwert") to DSS

sensor_values : s0 to s4 (current maximum is 5 value sensors)
        value_name -> name of the KWL data parameter to be evaluated (see table 3 below for all parameters currently supported)
        sensor_type -> DS specific value (see table 1 below) 
        sensor_usage -> DS specific value (see table 2 below)
        
        

Tables:
--------

Table 1 - DS specific sensor type to be used in config parameters sensor_values : s(x) -> sensor_type:

sensor_type   Description
----------------------------------------
1               Temperature (C)
2               Relative Humidity (%)
22              Room Carbon Dioxide (CO2)


Table 2 - DS specific sensor usage to be used in config parameters sensor_values : s(x) -> sensor_usage:

sensor_usage   Description
----------------------------------------
0                outdoor sensor
1                indoor sensor
        
Table 3 - data values currently supported

name of data value            description                                                                          use as                          
--------------------------------------------------------------------------------------------------------------------------------------
internalTemperature           temperature of Helios KWL internal sensor                                            sensor_values   
internalHumidity              humidity of Helios KWL internal sensor                                               sensor_values   


Sample of a valid helios.cfg file with useful settings, see file helios.cfg.sample
This sample config configures DigitalStrom scenes 1-4 following:
  scene 1: activate "home" profile in KWL  (modbus register 4609 value 0)
  scene 2: activate "away" profile in KWL   (modbus register 4609 value 1)
  scene 3: activate "boost" profile for 10 minutes in KWL   (modbus register 4612 value 10)
  scene 4: activate "boost" profile in KWL   (modbus register 4612 value 0)