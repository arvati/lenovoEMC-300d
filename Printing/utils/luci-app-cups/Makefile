#
# Copyright (C) 2016 CZ.NIC z.s.p.o. (http://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-cups
PKG_VERSION:=0.1
PKG_RELEASE:=20161215

PKG_LICENSE:=GPL-3.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-cups
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=CUPS forwarder for LuCI
  DEPENDS:=+luci-mod-admin-full +cups +cups-client
  MAINTAINER:=Michal Hrusecky <michal.hrusecky@nic.cz>
endef

define Package/luci-app-cups/description
 This package will install plugin to LuCI that will forward people to CUPS management page.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-cups/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_BIN) \
		./files/controller/cups.lua \
		$(1)/usr/lib/lua/luci/controller/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/
	$(INSTALL_DATA) \
		./files/view/cups.htm \
		$(1)/usr/lib/lua/luci/view/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_BIN) \
		./files/model/cbi/cups.lua \
		$(1)/usr/lib/lua/luci/model/cbi/
endef

$(eval $(call BuildPackage,luci-app-cups))
