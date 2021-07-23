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

![Screenshot from 2021-07-23 11-24-40](https://user-images.githubusercontent.com/8038366/126761369-b2896387-66d9-43d7-be30-1014f12e8b2a.png)
![Screenshot from 2021-07-23 11-24-45](https://user-images.githubusercontent.com/8038366/126761376-cd1088cc-b644-4e32-b44a-88024697322a.png)
![Screenshot from 2021-07-23 11-24-54](https://user-images.githubusercontent.com/8038366/126761383-e90ac77d-4014-4ac5-9424-8395a73e95df.png)
![Screenshot from 2021-07-23 11-25-13](https://user-images.githubusercontent.com/8038366/126761388-9637fc78-51e7-4633-958d-62d3101c9e67.png)
![Screenshot from 2021-07-23 11-25-20](https://user-images.githubusercontent.com/8038366/126761393-ef67926f-ea36-4574-ac84-f8eb32916543.png)
![Screenshot from 2021-07-23 11-25-24](https://user-images.githubusercontent.com/8038366/126761397-c04892b7-41f7-4acb-a4eb-1429410981d3.png)
![Screenshot from 2021-07-23 11-25-41](https://user-images.githubusercontent.com/8038366/126761401-9a679f5e-0460-4edc-92bb-bd67a5866f50.png)
![Screenshot from 2021-07-23 11-25-46](https://user-images.githubusercontent.com/8038366/126761406-204e7606-0fc2-4e42-9a7a-dd74f355e19d.png)



