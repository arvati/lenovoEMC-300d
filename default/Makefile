include $(TOPDIR)/rules.mk

PKG_NAME:=lenovo
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/lenovo
    SECTION:=firmware
    CATEGORY:=Firmware
    SUBMENU:=LenovoEMC-300d Drivers
    TITLE:=LenovoEMC-300d default packages
    DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-scsi-core +kmod-ata-core +kmod-i2c-core +kmod-usb-core \
    +kmod-ata-ahci +kmod-ata-ahci-platform +kmod-ata-piix +kmod-scsi-generic +kmod-scsi-cdrom \
    +kmod-hwmon-core +kmod-hwmon-gpiofan +kmod-hwmon-pwmfan +kmod-i2c-i801 +kmod-i2c-smbus \
    +kmod-hid-generic +kmod-leds-gpio +kmod-r8169 +kmod-bluetooth +kmod-gpio-pca953x \
    +kmod-gpio-button-hotplug +kmod-i2c-gpio-custom +kmod-parport-pc +kmod-serial-8250 \
    +kmod-regmap +kmod-ikconfig +kmod-zram +kmod-random-core +kmod-thermal +kmod-itco-wdt \
    +kmod-pcspkr +kmod-sound-dummy +kmod-sound-hda-intel +kmod-usb-uhci +kmod-usb2-pci \
    +kmod-usb3 +kmod-usb-audio +kmod-usb-printer +kmod-usb-serial-ch341 +kmod-usb-serial-ftdi \
    +kmod-usb-serial-pl2303 +kmod-usb-serial-cp210x +kmod-usb-storage +kmod-usb-storage-uas \
    +kmod-usb-hid +kmod-backlight-pwm +kmod-fb +kmod-drm +kmod-video-core +kmod-kvm-intel
endef

# Package description; a more verbose description on what our package does
define Package/lenovo/description
  Meta Package for LenovoEMC-300d default packages
endef


define Build/Prepare
endef

define Build/Compile
endef

define Package/helloworld/install
endef

$(eval $(call BuildPackage,lenovo))