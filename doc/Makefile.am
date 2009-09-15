bin_released_dir = doc
bin_released_files = irstlm-manual.pdf

binrelease_cmd = \
	mkdir -p ${top_srcdir}/${bininstalldir}/${bin_released_dir} ; \
	for p in $(bin_released_files); do \
		cp -r $$p ${top_srcdir}/${bininstalldir}/${bin_released_dir}/$$p ; \
	done ;

install:
	$(binrelease_cmd)

EXTRA_DIST = \
	irstlm-manual.pdf

