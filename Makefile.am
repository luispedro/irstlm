AUTOMAKE_OPTIONS = foreign
SUBDIRS = src scripts doc example

EXTRA_DIST = README

program_transform_name = 's/_lm/-lm/;' 

bin_released_files = @basebindir@
bininstallname = $(PACKAGE_NAME)-$(PACKAGE_VERSION)-bin.tar.gz


if SAME_MACHTYPE
install_cmd = 
else
install_cmd = \
	pushd $(basebindir) ; \
	if [[ -e $(MY_MACHTYPE) ]] ; then rm $(MY_MACHTYPE); fi ; \
	ln -s $(basebindir)/$(MY_ARCH) $(MY_MACHTYPE) ; \
	popd ; \
	pushd $(baselibdir) ; \
	if [[ -e $(MY_MACHTYPE) ]] ; then rm $(MY_MACHTYPE); fi ; \
	ln -s $(baselibdir)/$(MY_ARCH) $(MY_MACHTYPE) ; \
	popd ;
endif

binrelease_cmd = \
	mkdir -p ${top_srcdir}/${bininstalldir} ; \
	for p in $(bin_released_files); do \
		cp -r $$p ${top_srcdir}/${bininstalldir}/ ; \
	done ; \
	tar czf ${bininstallname} ${bininstalldir} ;\
	rm -rf ${bininstalldir}

install: install-recursive
	$(install_cmd) \
	$(binrelease_cmd)

