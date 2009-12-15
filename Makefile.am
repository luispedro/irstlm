AUTOMAKE_OPTIONS = foreign
SUBDIRS = src scripts example

EXTRA_DIST = README

program_transform_name = 's/_lm/-lm/;' 

bin_released_files = @basebindir@
lib_released_files = @baselibdir@ @includedir@
bininstallname = $(PACKAGE_NAME)-$(PACKAGE_VERSION)-bin
libinstallname = $(PACKAGE_NAME)-$(PACKAGE_VERSION)-lib


if SAME_MACHTYPE
install_cmd = 
else
install_cmd = \
	pushd $(basebindir) ; \
	if [[ -e $(MY_MACHTYPE) ]] ; then rm $(MY_MACHTYPE); fi ; \
	ln -s $(MY_ARCH) $(MY_MACHTYPE) ; \
	popd ; \
	pushd $(baselibdir) ; \
	if [[ -e $(MY_MACHTYPE) ]] ; then rm $(MY_MACHTYPE); fi ; \
	ln -s $(MY_ARCH) $(MY_MACHTYPE) ; \
	popd ;
endif

binrelease_cmd = \
	mkdir -p ${top_srcdir}/${bininstalldir} ; \
	for p in $(bin_released_files); do \
		cp -r $$p ${top_srcdir}/${bininstalldir}/ ; \
	done ; \
	tar czf ${bininstallname}.tar.gz ${bininstalldir} ;\
	zip -r ${bininstallname}.zip ${bininstalldir} ;\
	rm -rf ${bininstalldir};\
	mkdir -p ${top_srcdir}/${libinstalldir} ; \
	for p in $(lib_released_files); do \
		cp -r $$p ${top_srcdir}/${libinstalldir}/ ; \
	done ; \
	tar czf ${libinstallname}.tar.gz ${libinstalldir} ;\
	zip -r ${libinstallname}.zip ${libinstalldir} ;\
	rm -rf ${bininstalldir}

install: install-recursive
	$(install_cmd) \
	$(binrelease_cmd)

