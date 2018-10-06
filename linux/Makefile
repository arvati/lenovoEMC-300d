include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lenovo
PKG_FLAGS:=hold
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

CONFIG_PACKAGE_lenovo=y

define Package/lenovo
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  SUBMENU:=LenovoEMC-300d Drivers
  TITLE:=Virtual lenovo kernel package
  VERSION:=$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)
  URL:=http://www.kernel.org/
  PKG_FLAGS:=nonshared
endef

define Package/lenovo/install
  # nothing to do
endef

$(eval $(if $(DUMP),,$(call BuildPackage,lenovo)))

include $(sort $(wildcard ./modules/*.mk))