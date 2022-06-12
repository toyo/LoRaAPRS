# LoRaAPRS

LoRaAPRS is an software for APRS IGate and APRS tracker.
This relays position message not only from LoRa to APRS server but also from APRS server to LoRa.
This can work on ESP32 LoRa device described below.

## Getting Started

### Supported devices.

* [TTGO LoRa32 V2.1 (433MHz SX1278)](https://github.com/LilyGO/TTGO-LoRa32-V2.1)
* [TTGO T-Beam V1 (433MHz SX1278)](https://github.com/LilyGO/TTGO-T-Beam)

### Installing

* Compile this using PlatformIO and vscode. click Build and Upload in PlatformIO PROJECT TASKS.
* Set channel information in your region to data/channel.json, and set device information to data/ttgo*.json.
* Click Upload Filesystem Image in PlatformIO PROJECT TASKS.
* Reset device.

## Authors

FUJIURA Toyonori (JG2RZF)
[@toyokun](https://twitter.com/toyokun)

## Version History

* 0.1
    * Initial Release

## Acknowledgments

Inspiration,
* [LoRa APRS iGate](https://github.com/lora-aprs/LoRa_APRS_iGate)
