ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = vdc-helios
vdc_helios_SOURCES = main.c network.c configuration.c vdsd.c util.c icons.c helios.h incbin.h

vdc_helios_CFLAGS = \
    $(PTHREAD_CFLAGS) \
    $(LIBCONFIG_CFLAGS) \
    $(MODBUS_CFLAGS) \
    $(LIBDSVDC_CFLAGS) \
    $(LIBDSUID_CFLAGS)

vdc_helios_LDADD = \
    $(PTHREAD_LIBS) \
    $(LIBCONFIG_LIBS) \
    $(MODBUS_LIBS) \
    $(LIBDSVDC_LIBS) \
    $(LIBDSUID_LIBS)
