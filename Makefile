short_ver = 0.1
long_ver = $(shell git describe --long 2>/dev/null || echo $(short_ver)-0-unknown)

MODULE_big = pgbloomfun
OBJS = pgbloomfun.o bloom.o murmurhash3.o

EXTENSION = pgbloomfun
HAVE_EXT = $(shell test "$(shell pg_config --version | sed -e 's,PostgreSQL ,,')" "<" "9.1" && echo "false" || echo "true")
HAVE_JSON = $(shell test "$(shell pg_config --version | sed -e 's,PostgreSQL ,,')" "<" "9.2" && echo "false" || echo "true")

ifeq ($(HAVE_EXT),true)
DATA_built = pgbloomfun--$(short_ver).sql pgbloomfun.control
DATA = ext/pgbloomfun--unpackaged--0.1.sql
else
DATA_built = pgbloomfun.sql
endif

SHLIB_LINK = -lm

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

pgbloomfun.control: ext/pgbloomfun.control
	sed -e 's,__short_ver__,$(short_ver),g' < $^ > $@

pgbloomfun--$(short_ver).sql: ext/pgbloomfun.sql
ifeq ($(HAVE_JSON),true)
	cp -fp $^ $@
else
	sed -e 's, json , text ,g' $^ > $@
endif

pgbloomfun.sql: ext/pgbloomfun.sql
	sed -e 's,MODULE_PATHNAME,$$libdir/pgbloomfun,' \
	    -e 's, json , text ,g' \
	    -e '/CREATE EXTENSION/ d' \
	    $^ > $@

dist:
	git archive --output=../pgbloomfun_$(long_ver).tar.gz --prefix=pgbloomfun/ HEAD .

deb%:
	cp debian/changelog.in debian/changelog
	dch -v $(long_ver) "Automatically built package"
	sed -e s/PGVER/$(subst deb,,$@)/g < debian/packages.in > debian/packages
	yada rebuild
	debuild -uc -us -b

rpm:
	git archive --output=pgbloomfun-rpm-src.tar.gz --prefix=pgbloomfun/ HEAD
	rpmbuild -ta pgbloomfun-rpm-src.tar.gz \
		--define 'major_version $(short_ver)' \
		--define 'minor_version $(subst -,.,$(subst $(short_ver)-,,$(long_ver)))'
	$(RM) pgbloomfun-rpm-src.tar.gz
