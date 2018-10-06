include $(TOPDIR)/rules.mk

PKG_NAME:=lenovo

PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

CONFIG_PACKAGE_lenovo=y
define Package/$(PKG_NAME)
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  TITLE:=Virtual lenovo kernel package
endef

$(eval $(if $(DUMP),,$(call BuildPackage,$(PKG_NAME))))


include $(sort $(wildcard ./modules/*.mk))