package=libgmp
$(package)_download_path=https://github.com/ZencashOfficial/$(package)/releases/download/v20170131/
$(package)_file_name=$(package)-$($(package)_git_commit).tar.gz
$(package)_sha256_hash=59b2c2b5d58fdf5943bfde1fa709e9eb53e7e072c9699d28dc1c2cbb3c8cc32c
$(package)_git_commit=aece03c7b6967f91f3efdac8c673f55adff53ab1
$(package)_dependencies=
$(package)_config_opts=--enable-cxx --disable-shared

define $(package)_config_cmds
  $($(package)_autoconf) --host=$(host) --build=$(build)
endef

define $(package)_build_cmds
  $(MAKE) CPPFLAGS='-fPIC'
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install ; echo '=== staging find for $(package):' ; find $($(package)_staging_dir)
endef
