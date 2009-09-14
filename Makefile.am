AUTOMAKE_OPTIONS = foreign
SUBDIRS = src scripts doc example

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


install: install-recursive
	$(install_cmd)
