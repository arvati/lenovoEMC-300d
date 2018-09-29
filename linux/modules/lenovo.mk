
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
  CONFIG_DRM_I915:drivers/gpu/drm/i915

define KernelPackage/i915
  $(call lenovo_defaults,$(I915_MODULES))
  TITLE:=Intel 8xx/9xx/G3x/G4x/HD Graphics
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-drm
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