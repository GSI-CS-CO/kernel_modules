all:
	@echo Make documentation for VD80 driver
	mkdir -p doc/vmodttl doc/vmoddor doc/vmod12a2 doc/vmod16a2 doc/vmod12e16 &>/dev/null
	../../doxy/doxy.sh -n"VMOD-TTL Device Driver" -o "doc/vmodttl" ../include/libvmodttl.h &>/dev/null
	../../doxy/doxy.sh -n"VMOD-DOR Device Driver" -o "doc/vmoddor" ../include/libvmoddor.h &>/dev/null
	../../doxy/doxy.sh -n"VMOD-12A2 Device Driver" -o "doc/vmod12a2" ../include/libvmod12a2.h ../include/libvmod.h &>/dev/null
	../../doxy/doxy.sh -n"VMOD-12A2 Device Driver" -o "doc/vmod12a4" ../include/libvmod12a4.h ../include/libvmod.h &>/dev/null
	../../doxy/doxy.sh -n"VMOD-16A2 Device Driver" -o "doc/vmod16a2" ../include/libvmod16a2.h ../include/libvmod.h &>/dev/null
	../../doxy/doxy.sh -n"VMOD-12E16 Device Driver" -o "doc/vmod12e16" ../include/libvmod12e16.h ../include/libvmod.h &>/dev/null

clean cleanall:
	rm -rf doc/

# empty install rule
install:
