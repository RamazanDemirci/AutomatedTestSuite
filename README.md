# AutomatedTestSuite

I developed this project for testing other project which string messages communication project. if you stare at source code, requirements quite project specific. of course this is not commercial project and don't maintain anymore. but maybe someone find it useful.

Project purpose is create all possible messages test case and send to software over tcp and receive the response analyze and compare sent and received messages display result on ui. it was helpful for me at that time. when i change the software i run all text messages that i could trust my software i did not need manually test again. this software was saved time to me. in the source files "messages.json" is used for configuration  format and parameter. of course if i develop such that software today, i would handle with ui this configuration. at that time i was only person to use this. i didn't want spend effort for ui configuration. 

* recv_params: dynamic fields definitions.
- name : title of field
- format : c style format of field for extract the data. how many char or int, float etc.
- type : field type int, string, char or double
- min : field min value for useful for random generate
- max : field max value for useful for random generate
- range : if the field represent as enum this range very useful. if you dont use leave blank
- generate : there are three options for generating dynamic fields ; 
    - random: random generate between min and max
    - min2max: generate message for every possible values from min to max 
    - range: only generate for defined values
- lsb: i dont remember this field i think some fields ased on bit and  big endian and little endian issue handle with this field.
- we should define receive param for each send_params
* recv_format : we should define receive format for each recv_params
* send_params : recv_param field description is valid for this field
* send_format: similiar recv_format
* get_format: send after every send message this message i said above this is very speific req actually this would be define as scenerio.


