# Studio Display Brightness Control

A very hacked together "program" that sends just the right 
bytes to let you change a Studio Display's brightness.

Will iterate through available `/dev/hidrawX` until it finds 
a matching vendor id + product id with the correct report description.
Once found it'll send the appropriate byte array to set the brightness.

## Usage
`gcc ./main.c -ludev -o studio `
`./studio 50000`


