include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lenovo
PKG_FLAGS:=hold

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

export SHELL:=/bin/sh
.ONESHELL:
.SHELLFLAGS = -ec


include $(INCLUDE_DIR)/package.mk

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

$(eval $(if $(DUMP),,$(call BuildPackage,$(PKG_NAME))))


include $(sort $(wildcard ./modules/*.mk))