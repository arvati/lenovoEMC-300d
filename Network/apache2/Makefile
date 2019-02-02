#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=apache2
PKG_VERSION:=2.4.37
PKG_RELEASE:=2
PKG_SOURCE_NAME:=httpd

PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@APACHE/httpd/
PKG_HASH:=3498dc5c6772fac2eb7307dc7963122ffe243b5e806e0be4fb51974ff759d726

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>
PKG_LICENSE:=Apache License
PKG_CPE_ID:=cpe:/a:apache2:http_server

PKG_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_APACHE2_HTTP2 \
	CONFIG_APACHE2_DAV \
	CONFIG_APACHE2_PROXY \
	CONFIG_APACHE2_SESSION \
	CONFIG_APACHE2_AUTH \
	CONFIG_APACHE2_LUA \
	CONFIG_APACHE2_VHOST

# http://httpd.apache.org/docs/2.4/mod/

ADDITIONAL_MODULES:=

ifeq ($(CONFIG_APACHE2_LUA),y)
  ADDITIONAL_MODULES += --enable-lua
endif
ifneq ($(CONFIG_APACHE2_LUA),y)
  ADDITIONAL_MODULES += --disable-lua
endif

ifeq ($(CONFIG_APACHE2_HTTP2),y)
  ADDITIONAL_MODULES += --enable-http2
endif
ifneq ($(CONFIG_APACHE2_HTTP2),y)
  ADDITIONAL_MODULES += --disable-http2
endif

ifeq ($(CONFIG_APACHE2_DAV),y)
  ADDITIONAL_MODULES += --enable-dav --enable-dav_fs --enable-dav_lock
endif
ifneq ($(CONFIG_APACHE2_DAV),y)
  ADDITIONAL_MODULES += --disable-dav --disable-dav_fs --disable-dav_lock
endif

ifeq ($(CONFIG_APACHE2_PROXY),y)
  ADDITIONAL_MODULES += --enable-proxy_ajp --enable-proxy_balancer \
                        --enable-proxy_connect --enable-proxy_express --enable-proxy_fcgi \
                        --enable-proxy_fdpass --enable-proxy_ftp --enable-proxy_hcheck \
                        --enable-proxy_html --enable-proxy_http --enable-proxy_http2 \
                        --enable-proxy_scgi --enable-proxy_uwsgi --enable-proxy_wstunnel
endif

ifneq ($(CONFIG_APACHE2_PROXY),y)
  ADDITIONAL_MODULES += --disable-proxy_ajp --disable-proxy_balancer \
                        --disable-proxy_connect --disable-proxy_express --disable-proxy_fcgi \
                        --disable-proxy_fdpass --disable-proxy_ftp --disable-proxy_hcheck \
                        --disable-proxy_html --disable-proxy_http --disable-proxy_http2 \
                        --disable-proxy_scgi --disable-proxy_uwsgi --disable-proxy_wstunnel
endif

ifeq ($(CONFIG_APACHE2_SESSION),y)
  ADDITIONAL_MODULES += --enable-session --enable-session_cookie --enable-session_crypto
endif
ifneq ($(CONFIG_APACHE2_SESSION),y)
  ADDITIONAL_MODULES += --disable-session --disable-session_cookie --disable-session_crypto
endif

ifeq ($(CONFIG_APACHE2_AUTH),y)
  ADDITIONAL_MODULES += --enable-auth_basic --enable-auth_digest \
                        --enable-auth_form --enable-authn_anon --enable-authn_core \
                         --enable-authn_dbd --enable-authn_dbm --enable-authn_file  \
                         --enable-authn_socache --enable-authnz_fcgi --enable-authnz_ldap  \
                         --enable-authz_core --enable-authz_dbd --enable-authz_dbm \
                         --enable-authz_groupfile --enable-authz_host \
                         --enable-authz_owner --enable-authz_user
endif
ifneq ($(CONFIG_APACHE2_AUTH),y)
  ADDITIONAL_MODULES += --disable-auth_basic --disable-auth_digest \
                        --disable-auth_form --disable-authn_anon --disable-authn_core \
                        --disable-authn_dbd --disable-authn_dbm --disable-authn_file  \
                        --disable-authn_socache --disable-authnz_fcgi --disable-authnz_ldap  \
                        --disable-authz_core --disable-authz_dbd --disable-authz_dbm \
                        --disable-authz_groupfile --disable-authz_host \
                        --disable-authz_owner --disable-authz_user
endif

ifeq ($(CONFIG_APACHE2_VHOST),y)
  ADDITIONAL_MODULES += --enable-vhost_alias
endif
ifneq ($(CONFIG_APACHE2_VHOST),y)
  ADDITIONAL_MODULES += --disable-vhost_alias
endif


include $(INCLUDE_DIR)/package.mk

define Package/apache2/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=The Apache 2 Web Server
  URL:=https://httpd.apache.org/
  # DEPENDS:=!apache
endef

define Package/apache2/Default/description
 The Apache 2 Web Server is a powerful and flexible HTTP/1.1 compliant
 web server.  Originally designed as a replacement for the NCSA HTTP
 Server, it has grown to be the most popular web server on the Internet.
endef

define Package/apache2/config
  source "$(SOURCE)/Config.in"
endef

define Package/apache2
$(call Package/apache2/Default)
  DEPENDS += +libapr +libaprutil +libpcre +libopenssl +unixodbc +zlib +APACHE2_HTTP2:libnghttp2
endef

define Package/apache2/description
$(call Package/apache2/Default/description)
 .
 This package contains the Apache 2 web server and utility programs.
 .
 Take care that you don't include apache at the moment into your image
 please select it only as module because busybox will override
 /usr/sbin/httpd. It'll be solved soon. If you need to include this
 package in the image anyway, remove httpd from busybox
 (Base system --> Configuration --> Networking Utilities --> httpd).
 Also you should take care for the initscripts, apache's httpd isn't
 compatible with the one from busybox, so if you want to use apache
 for running your webif, you'll need to change the parameters in the
 scripts and configure the rest in /etc/httpd.conf.
endef

define Package/apache2/conffiles
/etc/apache/httpd.conf
/etc/apache/extra/httpd-autoindex.conf
/etc/apache/extra/httpd-dav.conf
/etc/apache/extra/httpd-default.conf
/etc/apache/extra/httpd-info.conf
/etc/apache/extra/httpd-languages.conf
/etc/apache/extra/httpd-manual.conf
/etc/apache/extra/httpd-mpm.conf
/etc/apache/extra/httpd-multilang-errordoc.conf
/etc/apache/extra/httpd-ssl.conf
/etc/apache/extra/httpd-userdir.conf
/etc/apache/extra/httpd-vhosts.conf
/etc/apache/magic
/etc/apache/mime.types
endef

define Package/apache2-icons
$(call Package/apache2/Default)
  TITLE:=Icons from Apache
  DEPENDS += apache2
endef

define Package/apache2-icons/description
$(call Package/apache2/Default/description)
 .
 This package contains the icons from Apache 2.
endef

TARGET_CFLAGS += $(FPIC) -Wno-error
TARGET_CPPFLAGS += -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
TARGET_LDFLAGS += -lpthread


# http://httpd.apache.org/docs/2.4/programs/configure.html#optionalfeatures

define Build/Configure
	$(call Build/Configure/Default, \
		--with-apr="$(STAGING_DIR)/usr/bin/apr-1-config" \
		--with-apr-util="$(STAGING_DIR)/usr/bin/apu-1-config" \
		--with-pcre="$(STAGING_DIR)/usr/bin/pcre-config" \
		$(ADDITIONAL_MODULES) \
		–enable-dev \
		--enable-http \
		--with-crypto \
		--with-sqlit3="$(STAGING_DIR)/usr" \
		--with-openssl="$(STAGING_DIR)/usr" \
		--enable-ssl \
		--enable-proxy \
		--enable-cgi \
		--enable-so \
		--enable-info	\
		--enable-alias \
		--enable-rewrite \
		--enable-deflate \
		--disable-md \
		--disable-disk-cache \
		--enable-maintainer-mode \
		--with-mpm=prefork \
		--with-mpm=worker \
		--enable-mime-magic \
		--without-suexec-bin \
		--sysconfdir=/etc/apache \
		ap_cv_void_ptr_lt_long=no \
		logfiledir="/var/log" \
		runtimedir="/var/run" \
		EXTRA_LIBS="-ldl -lpthread -lcrypto -lrt -lssl" \
	)
endef

define Build/InstallDev
	rm -rf	$(PKG_INSTALL_DIR)/usr/man/ \
		$(PKG_INSTALL_DIR)/usr/share/manual/
	# if you need docs take a look into the build-dir :)
	$(INSTALL_DIR) $(1)/etc
	$(CP)   $(PKG_INSTALL_DIR)/etc/* \
		$(1)/etc
	$(INSTALL_DIR) $(1)/usr/include/apache
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/apache
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP)   $(PKG_INSTALL_DIR)/usr/lib/httpd.exp \
		$(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP)   $(PKG_INSTALL_DIR)/usr/sbin/* \
		$(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/share
	$(CP)   $(PKG_INSTALL_DIR)/usr/share/* \
		$(1)/usr/share
endef

define Package/apache2/preinst
	rm /usr/sbin/httpd
	echo -e "You should take a look in the initscripts, busybox's httpd \n\
	uses some parameters which are maybe unsupported by apache."
endef

define Package/apache2/install
	$(INSTALL_DIR) $(1)/usr/sbin
	# we don't need apxs on the router, it's just for building apache modules.
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{ab,dbmmanage,htdbm,htdigest,htpasswd,httxt2dbm,logresolve} $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{apachectl,checkgid,envvars,envvars-std,htcacheclean,httpd,rotatelogs} $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/httpd.exp $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/{error,htdocs,cgi-bin,build} $(1)/usr/share/
	$(INSTALL_DIR) $(1)/etc/apache
	$(CP) $(PKG_INSTALL_DIR)/etc/apache/{httpd.conf,magic,mime.types,extra} $(1)/etc/apache/
endef

define Package/apache2/postrm
	rm -rf /usr/sbin/httpd
	ln -s /bin/busybox /usr/sbin/httpd
	echo -e "You may need to change your initscripts back for the use \n\
		with busybox's httpd."
endef

define Package/apache2-icons/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/icons $(1)/usr/share/
endef

$(eval $(call BuildPackage,apache2))
$(eval $(call BuildPackage,apache2-icons))

# take a look at: https://dan.drydog.com/apache2php.html
# for svn = http://forum.directadmin.com/showthread.php?t=47720