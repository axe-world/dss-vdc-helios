This vDC for DigtalSTROM smarthome system provides a modbus RTU (NOT modbus TCP of older Helios Devices with EasyControls2 !!) interface for Helios KWL devices. Tested with Helios EC200/300 W, but probably also with other models which support Helios EasyControls3.
The interaction between DigitalStrom system and this vDC is done bei calling scenes which are configured through a config file of the VDC. Additionally the vDC sends the value of the two Helios device internal sensors
internalTemperature and internalHumidity to DigitalSTROM system. This is currently hardcoded.  In future versions I will add possibility to add more sensor values through the config file.

Integration example in DigitalSTROM configurator:

 - create a "userdefined state" based on value of a CO2 sensor in your DigitalSTROM smarthome (e.g. Netatmo device) with two states "CO2 high" and "CO2" low
 - create two new "scene responders" one reacting on userdefined state "CO2 high" and calling the scene of this vDC which is configured to execute the "home" or "boost" profile in the Helios device
  and one reacting on userdefined state "CO2 low" and calling the scene of this vDC which is configured to execute the "away" profile in the Helios device

 
See README for more details!
