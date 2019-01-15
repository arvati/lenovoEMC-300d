#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio-f7188x
PKG_RELEASE:=1

PKG_MAINTAINER:=Ademar Arvati Filho <arvati@hotmail.com>
PKG_LICENSE:=GPL-3.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=LenovoEMC-300d Drivers
  TITLE:=F71808E and F71869 F71869A F71882FG F71889F F81866 GPIO support
  DEPENDS:=@PCI_SUPPORT @GPIO_SUPPORT @TARGET_x86
  FILES:=$(PKG_BUILD_DIR)/$(PKG_NAME).ko
  AUTOLOAD:=$(call AutoLoad,30,$(PKG_NAME),1)
  KCONFIG:=
endef

define KernelPackage/$(PKG_NAME)/description
 This option enables support for GPIOs found on Fintek Super-I/O chips F71869, F71869A, F71882FG, F71889F and F81866.
	And added support for GPIO driver for Fintek Super-I/O F71808E
endef

EXTRA_KCONFIG:= \
	CONFIG_GPIO_F7188X=m

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