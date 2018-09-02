#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ums8485md
PKG_RELEASE:=1

PKG_MAINTAINER:=Ademar Arvati Filho <arvati@hotmail.com>
PKG_LICENSE:=GPL-3.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Other modules
  TITLE:=UMS-8485MD-B LCD Driver
  DEPENDS:=@GPIO_SUPPORT @TARGET_x86 +kmod-i2c-core +kmod-i2c-gpio +kmod-i2c-gpio-custom +kmod-ich9-gpio +kmod-f71808e
  FILES:=$(PKG_BUILD_DIR)/$(PKG_NAME).ko
  AUTOLOAD:=$(call AutoLoad,30,$(PKG_NAME),1)
  KCONFIG:=
endef

define KernelPackage/$(PKG_NAME)/description
 Kernel module for UMS-8485MD-B LCD driver
endef

EXTRA_KCONFIG:= \
	CONFIG_LCD_UMS8485MD=m

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

$(eval $(call KernelPackage,$(PKG_NAME))