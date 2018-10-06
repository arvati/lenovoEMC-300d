#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lenovo
PKG_RELEASE:=1

PKG_MAINTAINER:=Ademar Arvati Filho <arvati@hotmail.com>
PKG_LICENSE:=GPL-3.0

include $(INCLUDE_DIR)/package.mk


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

LPC_ICH_MODULES:= \
  CONFIG_LPC_ICH:drivers/mfd/lpc_ich

define KernelPackage/lpc_ich
  $(call lenovo_defaults,$(LPC_ICH_MODULES))
  TITLE:=LPC interface for Intel ICH
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-itco-wdt
endef

define KernelPackage/lpc_ich/description
 LPC interface for Intel ICH
endef

$(eval $(call KernelPackage,lpc_ich))


GPIO_ICH_MODULES:= \
  CONFIG_GPIO_ICH:drivers/gpio/gpio-ich

define KernelPackage/gpio-ich
  $(call lenovo_defaults,$(GPIO_ICH_MODULES))
  TITLE:=GPIO interface for Intel ICH series
  DEPENDS:=@PCI_SUPPORT @GPIO_SUPPORT @TARGET_x86 +kmod-lpc_ich
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
  :drivers/char/agp/intel-gtt \
  CONFIG_AGP:drivers/char/agp/agpgart \
  :drivers/char/agp/drm_agpsupport \
  CONFIG_DRM:drivers/gpu/drm

define KernelPackage/i915
  $(call lenovo_defaults,$(I915_MODULES))
  TITLE:=Intel 8xx/9xx/G3x/G4x/HD Graphics
  DEPENDS:=@PCI_SUPPORT @TARGET_x86
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
  CONFIG_RTC_DRV_CMOS:drivers/rtc/rtc-cmos \
  CONFIG_RTC_LIB:drivers/rtc/rtc-lib \
  CONFIG_RTC_CLASS:drivers/rtc/rtc-core


define KernelPackage/rtc-cmos
  $(call lenovo_defaults,$(RTC_DRV_CMOS_MODULES))
  TITLE:=PC-style CMOS real time clock
  DEPENDS:=@TARGET_x86
endef

define KernelPackage/rtc-cmos/description
 PC-style CMOS real time clock
endef

$(eval $(call KernelPackage,rtc-cmos))

GPIO_F7188X_MODULES:= \
  CONFIG_GPIO_F7188X:drivers/gpio/f7188x-gpio

define KernelPackage/f7188x-gpio
  $(call lenovo_defaults,$(GPIO_F7188X_MODULES))
  TITLE:=F71869 F71869A F71882FG F71889F and F81866 GPIO support
  DEPENDS:=@TARGET_x86
endef

define KernelPackage/f7188x-gpio/description
 This option enables support for GPIOs found on Fintek Super-I/O chips F71869, F71869A, F71882FG, F71889F and F81866.
endef

$(eval $(call KernelPackage,f7188x-gpio))

I2C_ALGOBIT_MODULES:= \
  CONFIG_I2C_ALGOBIT:drivers/i2c/algos/i2c_algo_bit

define KernelPackage/i2c_algo_bit
  $(call lenovo_defaults,$(I2C_ALGOBIT_MODULES))
  TITLE:=I2C bit-banging interfaces
  DEPENDS:=@TARGET_x86 +kmod-i2c-core
endef

define KernelPackage/i2c_algo_bit/description
 I2C bit-banging interfaces
endef

$(eval $(call KernelPackage,i2c_algo_bit))

MICROCODE_MODULES:= \
  CONFIG_MICROCODE:arch/x86/kernel/cpu/microcode/microcode \
  CONFIG_MICROCODE_INTEL:arch/x86/kernel/cpu/microcode/intel \
  :arch/x86/kernel/cpu/microcode/core

define KernelPackage/microcode
  $(call lenovo_defaults,$(MICROCODE_MODULES))
  TITLE:=CPU microcode loading support
  DEPENDS:=@TARGET_x86 intel-microcode
#CONFIG_CPU_SUP_INTEL=y CONFIG_PROCESSOR_SELECT CONFIG_EXPERT
endef

define KernelPackage/microcode/description
 If you say Y here, you will be able to update the microcode on Intel and AMD processors. The Intel support is for the IA32 family, e.g. Pentium Pro, Pentium II, Pentium III, Pentium 4, Xeon etc. The AMD support is for families 0x10 and later. You will obviously need the actual microcode binary data itself which is not shipped with the Linux kernel.
endef

$(eval $(call KernelPackage,microcode))

TIGON3_MODULES:= \
  CONFIG_TIGON3:drivers/net/tg3 \
  CONFIG_PTP_1588_CLOCK:drivers/ptp/ptp \
  CONFIG_PPS:drivers/pps/pps_core

define KernelPackage/tg3
  $(call lenovo_defaults,$(TIGON3_MODULES))
  TITLE:=Broadcom Tigon3 suppor
  KCONFIG+= \
    CONFIG_NETWORK_PHY_TIMESTAMPING=y \
    CONFIG_DP83640_PHY=n \
    CONFIG_PTP_1588_CLOCK_KVM=y

  DEPENDS:=@TARGET_x86 @PCI_SUPPORT
endef

define KernelPackage/tg3/description
 This driver supports Broadcom Tigon3 based gigabit Ethernet cards and
 The IEEE 1588 standard defines a method to precisely synchronize distributed clocks over Ethernet networks. The standard defines a Precision Time Protocol (PTP), which can be used to achieve synchronization within a few dozen microseconds. In addition, with the help of special hardware time stamping units, it can be possible to achieve synchronization to within a few hundred nanoseconds.
endef

$(eval $(call KernelPackage,tg3))



define Build/Compile
endef
