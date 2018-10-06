include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lenovo
PKG_FLAGS:=hold

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages
SCAN_DEPS=modules/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter.mk

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

export SHELL:=/bin/sh
.ONESHELL:
.SHELLFLAGS = -ec


include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

CONFIG_PACKAGE_lenovo=y
define Package/$(PKG_NAME)
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  TITLE:=Virtual lenovo kernel package
  VERSION:=$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)
  URL:=http://www.kernel.org/
  PKG_FLAGS:=nonshared
endef

define Package/$(PKG_NAME)/install
  # nothing to do
endef

define Package/$(PKG_NAME)/extra_provides
	sed -e 's,.*/,,' $(LINUX_DIR)/modules.builtin;
endef

$(eval $(if $(DUMP),,$(call BuildPackage,$(PKG_NAME))))


include $(sort $(wildcard ./modules/*.mk))