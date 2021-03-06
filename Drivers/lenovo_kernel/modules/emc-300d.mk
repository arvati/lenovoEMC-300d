LENOVO_MENU:=LenovoEMC-300d Drivers

ModuleConfVar=$(word 1,$(subst :,$(space),$(1)))
ModuleFullPath=$(LINUX_DIR)/$(word 2,$(subst :,$(space),$(1))).ko
ModuleKconfig=$(foreach mod,$(1),$(call ModuleConfVar,$(mod)))
ModuleFiles=$(foreach mod,$(1),$(call ModuleFullPath,$(mod)))
ModuleAuto=$(call AutoLoad,$(1),$(foreach mod,$(2),$(basename $(notdir $(call ModuleFullPath,$(mod))))),$(3))

define lenovo_defaults
  SUBMENU:=$(LENOVO_MENU)
  KCONFIG:=$(call ModuleKconfig,$(1))
  FILES:=$(call ModuleFiles,$(1))
  AUTOLOAD:=$(call ModuleAuto,$(2),$(1),$(3))
endef


GPIO_ICH_MODULES:= \
  CONFIG_GPIO_ICH:drivers/gpio/gpio-ich

define KernelPackage/gpio-ich
  $(call lenovo_defaults,$(GPIO_ICH_MODULES))
  TITLE:=GPIO interface for Intel ICH series
  DEPENDS:=@PCI_SUPPORT @GPIO_SUPPORT @TARGET_x86
endef

define KernelPackage/gpio-ich/description
 GPIO interface for Intel ICH series
endef

$(eval $(call KernelPackage,gpio-ich))


F71882FG_MODULES:= \
  CONFIG_SENSORS_F71882FG:drivers/hwmon/f71882fg

define KernelPackage/f71882fg
  $(call lenovo_defaults,$(F71882FG_MODULES))
  TITLE:=F71882FG Hardware Monitoring Driver
  DEPENDS:=@PCI_SUPPORT @GPIO_SUPPORT @TARGET_x86 +kmod-hwmon-core
endef

define KernelPackage/f71882fg/description
 F71882FG Hardware Monitoring Driver
endef

$(eval $(call KernelPackage,f71882fg))


SERIO_I8042_MODULES:= \
  CONFIG_SERIO_I8042:drivers/input/serio/i8042

define KernelPackage/serio-i8042
  $(call lenovo_defaults,$(SERIO_I8042_MODULES))
  TITLE:=i8042 PC Keyboard controller
  DEPENDS:=@TARGET_x86
endef

define KernelPackage/serio-i8042/description
 i8042 is the chip over which the standard AT keyboard and PS/2 mouse are connected to the computer.
endef

$(eval $(call KernelPackage,serio-i8042))


I915_MODULES:= \
  CONFIG_DRM_I915:drivers/gpu/drm/i915/i915 \
  CONFIG_DRM_KMS_HELPER:drivers/gpu/drm/drm_kms_helper \
  CONFIG_AGP_INTEL:drivers/char/agp/intel-agp \
  CONFIG_FB_INTEL:drivers/video/fbdev/intelfb/intelfb \
  CONFIG_AGP_INTEL:drivers/char/agp/intel-gtt \
  CONFIG_AGP:drivers/char/agp/agpgart \
  CONFIG_DRM:drivers/gpu/drm/drm \
  CONFIG_ACPI_VIDEO:drivers/acpi/video \
  CONFIG_BACKLIGHT_CLASS_DEVICE:drivers/video/backlight/backlight

define KernelPackage/i915
  $(call lenovo_defaults,$(I915_MODULES))
  TITLE:=Intel 8xx/9xx/G3x/G4x/HD Graphics
  DEPENDS:=@PCI_SUPPORT @DISPLAY_SUPPORT @TARGET_x86 +kmod-i2c-algo-bit +kmod-video-core +kmod-input-core
  KCONFIG+= \
        CONFIG_DRM_I915_ALPHA_SUPPORT=n \
        CONFIG_DRM_I915_CAPTURE_ERROR=y \
        CONFIG_DRM_I915_COMPRESS_ERROR=y \
        CONFIG_DRM_I915_USERPTR=y \
        CONFIG_DRM_I915_GVT=n \
        CONFIG_DRM_I915_WERROR=n \
        CONFIG_DRM_I915_DEBUG=n \
        CONFIG_DRM_I915_SW_FENCE_DEBUG_OBJECTS=n \
        CONFIG_DRM_I915_SW_FENCE_CHECK_DAG=n \
        CONFIG_DRM_I915_SELFTEST=n \
        CONFIG_DRM_I915_LOW_LEVEL_TRACEPOINTS=n \
        CONFIG_DRM_I915_DEBUG_VBLANK_EVADE=n \
        CONFIG_DRM_VMWGFX=n \
        CONFIG_DRM_GMA500=n \
        CONFIG_DRM_VIRTIO_GPU=n \
	    CONFIG_AGP_AMD64=n \
	    CONFIG_AGP_SIS=n \
	    CONFIG_AGP_VIA=n \
	    CONFIG_FB_INTEL_DEBUG=n \
	    CONFIG_FB_INTEL_I2C=y
endef

define KernelPackage/i915/description
 Intel 8xx/9xx/G3x/G4x/HD Graphics
endef

$(eval $(call KernelPackage,i915))

RTC_DRV_CMOS_MODULES:= \
  CONFIG_RTC_DRV_CMOS:drivers/rtc/rtc-cmos

define KernelPackage/rtc-cmos
  $(call lenovo_defaults,$(RTC_DRV_CMOS_MODULES))
  TITLE:=PC-style CMOS real time clock
  DEPENDS:=@TARGET_x86 +kmod-i2c-core
  DEFAULT:=m if ALL_KMODS && RTC_SUPPORT
  KCONFIG+= \
        CONFIG_RTC_CLASS=y
endef

define KernelPackage/rtc-cmos/description
 PC-style CMOS real time clock
endef

$(eval $(call KernelPackage,rtc-cmos))

TIGON3_MODULES:= \
  CONFIG_TIGON3:drivers/net/ethernet/broadcom/tg3

define KernelPackage/tg3
  $(call lenovo_defaults,$(TIGON3_MODULES))
  TITLE:=Broadcom Tigon3 support
  KCONFIG+= \
    CONFIG_NETWORK_PHY_TIMESTAMPING=y \
    CONFIG_DP83640_PHY=n \
    CONFIG_PTP_1588_CLOCK_KVM=y
  DEPENDS:=@TARGET_x86 @PCI_SUPPORT +kmod-libphy +kmod-pps +kmod-ptp
endef

define KernelPackage/tg3/description
 This driver supports Broadcom Tigon3 based gigabit Ethernet cards and
 The IEEE 1588 standard defines a method to precisely synchronize distributed clocks over Ethernet networks. The standard defines a Precision Time Protocol (PTP), which can be used to achieve synchronization within a few dozen microseconds. In addition, with the help of special hardware time stamping units, it can be possible to achieve synchronization to within a few hundred nanoseconds.
endef

$(eval $(call KernelPackage,tg3))