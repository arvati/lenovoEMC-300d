# link https://github.com/Entware/rtndev/blob/master/serf/Makefile
#
# Copyright (C) 2017-2018 Entware
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=serf
PKG_VERSION:=1.3.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://archive.apache.org/dist/serf
PKG_HASH:=549c2d21c577a8a9c0450facb5cca809f26591f048e466552240947bdf7a87cc


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/scons.mk

define Package/libserf
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=High performance C-based HTTP client library
	URL:=https://serf.apache.org/
	MAINTAINER:=Entware team, https://entware.net
	DEPENDS:=+libopenssl +libaprutil +PACKAGE_unixodbc:unixodbc +libapr
	#### +unixodbc
endef

define Package/libserf/description
 The serf library is a high performance C-based HTTP client library built upon the Apache Portable Runtime (APR) library.
 It is permissively licensed under the Apache License, v2.
 Key features:
  multiplexed, asynchronous connections
  SSL/TLS support
  full HTTP pipelining
  multiple authentication modes (Basic, Digest, Kerberos/NTLM)
  zero-copy support for increased throughput
 Serf is the default client library of Apache Subversion and Apache OpenOffice
endef

# Dirty trick to define PATH_MAX. It is needed by apr.h. PATH_MAX is defined in <linux/limits> and is the same for all feeds

SCONS_VARS += \
	LINKFLAGS="$(TARGET_LDFLAGS)"

SCONS_OPTIONS += \
	CC="$(TARGET_CC_NOCACHE)" \
    CPPFLAGS="$(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS) -DPATH_MAX=4096" \
    LINKFLAGS="$(TARGET_LDFLAGS)" \
	PREFIX="/usr" \
	APR="$(STAGING_DIR)/usr" \
	APU="$(STAGING_DIR)/usr"

define Build/Prepare
	$(call Build/Prepare/Default)
endef


define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		$(SCONS_VARS) \
		scons \
			$(SCONS_OPTIONS) \
		install \
	)
endef

# Also take a look at https://github.com/openwrt/packages/blob/master/utils/gpsd/Makefile
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/serf-1
	$(CP) $(PKG_BUILD_DIR)/{serf_bucket_types,serf_bucket_util,serf}.h $(1)/usr/include/serf-1/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libserf-1.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/serf-1.pc $(1)/usr/lib/pkgconfig/
endef



define Package/libserf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libserf-1.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libserf))

