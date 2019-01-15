#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Original hack from = https://github.com/d235j/lenovoEMC-300d

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lpc_ich
PKG_RELEASE:=1

PKG_MAINTAINER:=Ademar Arvati Filho <arvati@hotmail.com>
PKG_LICENSE:=GPL-3.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=LenovoEMC-300d Drivers
  TITLE:=Intel ICH LPC with hacks for Lenovo PX-300d series
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-itco-wdt
  FILES:=$(PKG_BUILD_DIR)/$(PKG_NAME).ko
  AUTOLOAD:=$(call AutoLoad,30,$(PKG_NAME),1)
  KCONFIG:=
endef

define KernelPackage/$(PKG_NAME)/description
	  The LPC bridge function of the Intel ICH provides support for
	  many functional units. This driver provides needed support for
	  other drivers to control these functions, currently GPIO and
	  watchdog.
	  With hacks for Lenovo PX-300d series:
	  The IOmega/Lenovo EMC StorCenter PX-300d series
	  has a generic BIOS that reserves the GPIO1 range,
	  but uses certain GPIOs in that range for its LCD
	  module. We need to override this check to allow the
	  LCD driver access to those GPIOs.
endef

EXTRA_KCONFIG:= \
	CONFIG_LPC_ICH=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))