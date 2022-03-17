# Pixie16_GUI_DAQ
This is a GUI DAQ for XIA Pixie16 digitizer.

# requires 
PlxSDK8.0+, installed at /usr/opt/PlxSdk
PixieSDK 3.3, installed at /usr/opt/xia/PixieSDK
CERN ROOT 6.0+ (using 6.24/06)

The program is developing on Debian 10. using g++ 8.3.0, using Pixie16 16-bit 250MHz revision F.

# make
just $make, it will create object files and link all together to pixieDAQ
it will also make test and example under testing folder.

# Pixie16Class
this is the main class for controlling the pixie16 digitizer. 
It unfolds some control for the CHANNEL_CSRA. It likes a C++ wrapper for the C code for the PixieSDK. Before PixieSDK3.3, it also decode the data.

# DataBlock
this is a C++ class for a data Block, which is a Block for pixie output.
