#Fatbeacon#

This is a nRF52 impementation of the [Physical Web Fatbeacon](https://github.com/google/physical-web/issues/784) spec.  It's a work in progress, so don't expect too much.
The code is based off the experimental_ble_app_eddystone example that comes with the nRF5 v11 SDK.

I've only maintained the GNU Make configuration and its built and tested on the nRF52 DK, as well as my [BeaconBuddy](https://github.com/angst7/BeaconBuddy) hardware as that's what I use.
To build it, just update the NRF_SDK_PATH in the Makefile, and go for it.    

Currently supported hardware is:
* nRF52 DK (PCA10040)
* BeaconBuddy
* RuuviTag rev.B3
* ...and probably any hardware for which an nRF BSP header exists

This version is a little rough, far from production, and will probably melt your eyes in addition to any silicon it touches.  You've been warned.

## License ##
Licensed under BSD unless otherwise specified in the source code (some code is copyright Nordic Semiconductor, licenses in source files)
See LICENSE for other info.