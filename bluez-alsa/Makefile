#
# Copyright (C) 2017-2018 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bluez-alsa
PKG_VERSION:=1.3.1
PKG_RELEASE:=2
PKG_LICENSE:=MIT

PKG_SOURCE_URL:=https://codeload.github.com/Arkq/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=29dad23877d0cf46a16e2f8d3746219e89068c33d052059caf1caaacd8b40cac

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk


define Package/bluez-alsa
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+alsa-lib +bluez-daemon +glib2 +sbc +fdk-aac
# +libreadline +libbsd +libncurses
  TITLE:=Bluetooth Audio ALSA backend
  URL:=https://github.com/Arkq/bluez-alsa
endef

define Package/bluez-alsa/description
  ALSA plugin supporting Bluetooth audio
endef

CONFIGURE_ARGS += --enable-aac
# --enable-rfcomm --enable-hcitop

define Package/bluez-alsa/install
	$(INSTALL_DIR) $(1)/usr/share/alsa/alsa.conf.d/
	$(CP) $(PKG_BUILD_DIR)/src/asound/20-bluealsa.conf $(1)/usr/share/alsa/alsa.conf.d/
	$(INSTALL_DIR) $(1)/usr/lib/alsa-lib
	$(CP) $(PKG_BUILD_DIR)/src/asound/.libs/libasound_module_*_bluealsa.so $(1)/usr/lib/alsa-lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bluealsa $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/bluealsa-aplay $(1)/usr/bin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/bluealsa-rfcomm $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bluealsa.init $(1)/etc/init.d/bluealsa
endef

$(eval $(call BuildPackage,bluez-alsa))
