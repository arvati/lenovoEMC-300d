# link = https://gist.github.com/DanielSel/e575e189720022eaffd61c2878a41508
# Copyright (C) 2017 Daniel Sel
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=docker
PKG_VERSION:=17.05.0
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

define Package/docker
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Docker
  URL:=https://www.docker.com/
  DEPENDS:=+libseccomp +libdevmapper
endef

define Package/docker/description
 DEV ONLY -- test package to run precompiled docker from ALPINE
endef


define Package/docker/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/etc/config/docker $(1)/etc/config/docker
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/docker $(1)/etc/init.d/docker
endef

define Package/docker/conffiles
/etc/config/docker
endef


define Package/docker/preinst
#!/bin/sh
echo "Adding docker usergroup"
groupadd -r docker
exit 0
endef

define Package/docker/postinst
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
        echo "Enabling rc.d symlink for docker"
        /etc/init.d/docker enable
fi
exit 0
endef

define Package/docker/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
        echo "Removing rc.d symlink for docker"
        /etc/init.d/docker disable
fi
exit 0
endef


define Build/Prepare
endef

define Build/Compile
endef

define Build/Install
endef

define Build/Clean
endef


$(eval $(call BuildPackage,docker))