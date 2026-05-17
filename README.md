# ZT-XT1 alternative firmware
Alternative firmware for the main MCU in the Zotek/Zoyi ZT-XT1. Not affiliated with Zotek Instruments Co., Ltd.
Use at your own risk. At the moment, this firmware offers no advantage over the official one.

## Required toolchain
The code base requires C23 support. The active toolchain is selected by the `toolchainFile` setting in [CMakePresets.json](CMakePresets.json). You may have to fiddle with the files in cmake/toolchains. Good luck!

## License
See [LICENSE](LICENSE)
