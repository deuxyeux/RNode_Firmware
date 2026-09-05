PATH_RETICULUM_WEBSITE=../../sites/reticulum.network
PATH_PACKAGES=../../dist_archive

clean:
	@echo Cleaning...
	@-rm -rf ./build

dirs:
	@mkdir -p ./build
	@mkdir -p ./build/3d
	@mkdir -p ./build/pkg
	@mkdir -p ./build/css
	@mkdir -p ./build/gfx
	@mkdir -p ./build/images

pages:
	python ./build.py

pages-debug:
	python ./build.py --no-gz --no-remap

sourcepack:
	@echo Packing firmware sources...
	zip --junk-paths -r build/pkg/rnode_firmware.zip ../arduino-cli.yaml ../RNode_Firmware/BLESerial.cpp ../RNode_Firmware/BLESerial.h ../RNode_Firmware/Bluetooth.h ../RNode_Firmware/Boards.h ../RNode_Firmware/Config.h ../RNode_Firmware/Console.h ../RNode_Firmware/Device.h ../RNode_Firmware/Display.h ../RNode_Firmware/Framing.h ../RNode_Firmware/Graphics.h ../LICENSE ../Makefile ../RNode_Firmware/MD5.cpp ../RNode_Firmware/MD5.h ../partition_hashes ../RNode_Firmware/Power.h ../README.md ../release_hashes.py ../RNode_Firmware/RNode_Firmware.ino ../RNode_Firmware/ROM.h ../RNode_Firmware/sx126x.cpp ../RNode_Firmware/sx126x.h ../RNode_Firmware/sx127x.cpp ../RNode_Firmware/sx127x.h ../RNode_Firmware/sx128x.cpp ../RNode_Firmware/sx128x.h ../RNode_Firmware/Utilities.h ../esp32_btbufs.py
	
data:
	@echo Including assets...
	@cp assets/css/* build/css/
	@cp assets/gfx/* build/gfx/
	@cp assets/images/* build/images/
	@cp assets/stl/* build/3d/
	#@cp assets/pkg/* build/pkg/
	# @cp assets/scripts/* build/scripts/
	# @cp -r ../../Reticulum/docs/manual/* build/reticulum_manual/
	# @cp -r ../../Reticulum/docs/Reticulum\ Manual.pdf build/reticulum_manual/

external:
	make -C $(PATH_RETICULUM_WEBSITE) clean website
	-rm -r $(PATH_PACKAGES)/reticulum.network
	cp -r $(PATH_RETICULUM_WEBSITE)/build $(PATH_PACKAGES)/reticulum.network

site: clean external dirs data sourcepack pages

local: clean external dirs data sourcepack pages-debug

serve:
	python -m http.server 7777 --bind 127.0.0.1 --directory ./build
