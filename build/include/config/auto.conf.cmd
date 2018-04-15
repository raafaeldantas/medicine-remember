deps_config := \
	/home/Rafael/esp/esp-idf/components/app_trace/Kconfig \
	/home/Rafael/esp/esp-idf/components/aws_iot/Kconfig \
	/home/Rafael/esp/esp-idf/components/bt/Kconfig \
	/home/Rafael/esp/esp-idf/components/esp32/Kconfig \
	/home/Rafael/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/Rafael/esp/esp-idf/components/espmqtt/Kconfig \
	/home/Rafael/esp/esp-idf/components/ethernet/Kconfig \
	/home/Rafael/esp/esp-idf/components/fatfs/Kconfig \
	/home/Rafael/esp/esp-idf/components/freertos/Kconfig \
	/home/Rafael/esp/esp-idf/components/heap/Kconfig \
	/home/Rafael/esp/esp-idf/components/libsodium/Kconfig \
	/home/Rafael/esp/esp-idf/components/log/Kconfig \
	/home/Rafael/esp/esp-idf/components/lwip/Kconfig \
	/home/Rafael/esp/esp-idf/components/mbedtls/Kconfig \
	/home/Rafael/esp/esp-idf/components/openssl/Kconfig \
	/home/Rafael/esp/esp-idf/components/pthread/Kconfig \
	/home/Rafael/esp/esp-idf/components/spi_flash/Kconfig \
	/home/Rafael/esp/esp-idf/components/spiffs/Kconfig \
	/home/Rafael/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/Rafael/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/Rafael/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/Rafael/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/Rafael/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/Rafael/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
