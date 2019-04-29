# nvram
Userspace port of the NVRAM manipulation tool

## Reference website
https://github.com/mirko/openwrt-x-burst/blob/master/package/nvram/Makefile

## Makefile For Openwrt
```
include $(TOPDIR)/rules.mk

PKG_NAME:=nvram
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dnetlab/nvram.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=e01994dc728f611d13971a32317b7a8aa887a4cb
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Dnetlab
  TITLE:=nvram for app config
endef

# define Build/Prepare
	# $(CP) ./src/* $(PKG_BUILD_DIR)/
# endef

define Package/$(PKG_NAME)/install
    $(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nvram.init $(1)/etc/init.d/nvram
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nvram $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
```