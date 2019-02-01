#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hplip
PKG_VERSION:=3.16.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/hplip
PKG_MD5SUM:=7d187c3142ba323a3021767d333ad9f4

PKG_BUILD_DEPENDS := python
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/hplip
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=HP Linux Imaging and Printing
  URL:=http://sourceforge.net/projects/hplip/
  DEPENDS+=+libjpeg +libtiff +libusb-compat +cups +libcupsimage
endef

define Package/hplip/description
	HPLIP is an HP developed solution for printing, scanning, and faxing with HP inkjet and laser based printers in Linux.
endef

CONFIGURE_ARGS += \
	--disable-gui-build \
	--disable-network-build \
	--disable-fax-build \
	--disable-scan-build \
	--disable-pp-build \
	--disable-doc-build \
	--disable-foomatic-xml-install \
	--disable-dbus-build \
	--enable-libusb01_build \
	--enable-cups-ppd-install \
	--enable-cups-drv-install

define Build/Configure
	sed -i 's|PYTHONINCLUDEDIR = .*|PYTHONINCLUDEDIR = $(STAGING_DIR)/usr/include/python2.7|' $(PKG_BUILD_DIR)/Makefile.in
	$(call Build/Configure/Default,\
		$(CONFIGURE_ARGS),\
		ac_cv_lib_cups_cupsDoFileRequest=yes \
	)
endef

define Package/hplip/install
	make -C $(PKG_BUILD_DIR) install DESTDIR=$(1)
	rm -rf $(1)/etc/udev $(1)/usr/bin $(1)/usr/lib/systemd $(1)/usr/lib/python2.7
	rm -rf $(1)/usr/share/doc $(1)/usr/share/hal $(1)/var
	mv $(1)/usr/share/hplip/data $(1)/usr/share/hplip-data
	rm -rf $(1)/usr/share/hplip
	mkdir -p $(1)/usr/share/hplip
	mv $(1)/usr/share/hplip-data $(1)/usr/share/hplip/data
endef

define Package/hplip/conffiles
/etc/hp/hplip.conf
endef

define Package/postinst
#!/bin/sh
[ -n "$$IPKG_INSTROOT" ] || {
/etc/init.d/cupsd enabled && /etc/init.d/cupsd restart
}
endef

$(eval $(call BuildPackage,hplip))
