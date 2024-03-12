Configuration
--------------

This vDC requires a configuration file called helios.cfg in the same folder as the vDC.
If you start the vDC without an existing configuration file, a new one will be created containing the required parameters. vDC will terminate after this.
Change the parameters according to your requirements and start the vDC again.

Following is a description of each parameter:

vdcdsuid  -> this is a unique DS id and will be automatically created; just leave empty in config file
reload_values -> time in seconds after which new values are pulled from klafs server
zone_id   -> DigitalStrom zone id
debug     -> Logging level for the vDC  - 7 debug / all messages  ; 0 nearly no messages;

Section "kwl" contains the KWL device configuration:

kwl : id -> some alphanumeric ID identifying the device (e.g. model). 
             
kwl : name -> Any name for your KWL device

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


Sample of a valid helios.cfg file with useful settings:
------------------------------------------------------

reload_values = 60;
zone_id = 65534;
debug = 7;
kwl : 
{
  id = "EC300";
  name = "Helios KWL";
};
sensor_values : 
{
  s0 : 
  {
    value_name = "internalTemperature";
    sensor_type = 1;
    sensor_usage = 1;
  };
  s1 : 
  {
    value_name = "internalHumidity";
    sensor_type = 2;
    sensor_usage = 1;
  };
};

