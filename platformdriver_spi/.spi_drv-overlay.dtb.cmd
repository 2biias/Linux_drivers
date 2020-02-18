cmd_/home/stud/hal_work/exercise7/spi_drv-overlay.dtb := mkdir -p /home/stud/hal_work/exercise7/ ; arm-poky-linux-gnueabi-gcc -E -Wp,-MD,/home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.d.pre.tmp -nostdinc -I./scripts/dtc/include-prefixes -undef -D__DTS__ -x assembler-with-cpp -o /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.dts.tmp /home/stud/hal_work/exercise7/spi_drv-overlay.dts ; ./scripts/dtc/dtc -O dtb -o /home/stud/hal_work/exercise7/spi_drv-overlay.dtb -b 0 -i/home/stud/hal_work/exercise7/ -i./scripts/dtc/include-prefixes -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-gpios_property -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-graph_port -Wno-unique_unit_address -Wno-pci_device_reg  -d /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.d.dtc.tmp /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.dts.tmp ; cat /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.d.pre.tmp /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.d.dtc.tmp > /home/stud/hal_work/exercise7/.spi_drv-overlay.dtb.d

source_/home/stud/hal_work/exercise7/spi_drv-overlay.dtb := /home/stud/hal_work/exercise7/spi_drv-overlay.dts

deps_/home/stud/hal_work/exercise7/spi_drv-overlay.dtb := \

/home/stud/hal_work/exercise7/spi_drv-overlay.dtb: $(deps_/home/stud/hal_work/exercise7/spi_drv-overlay.dtb)

$(deps_/home/stud/hal_work/exercise7/spi_drv-overlay.dtb):
