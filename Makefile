short_ver = 0.1
long_ver = $(shell git describe --long 2>/dev/null || echo $(short_ver)-0-unknown)

MODULE_big = pgbloomfun
OBJS = pgbloomfun.o bloom.o murmurhash3.o

EXTENSION = pgbloomfun
DATA_built = pgbloomfun--$(short_ver).sql pgbloomfun.control

SHLIB_LINK = -lm

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

pgbloomfun.control: ext/pgbloomfun.control
	sed -e 's,__short_ver__,$(short_ver),g' < $^ > $@

pgbloomfun--$(short_ver).sql: ext/pgbloomfun.sql
	cp -fp $^ $@

dist:
	git archive --output=../pgbloomfun_$(long_ver).tar.gz --prefix=pgbloomfun/ HEAD .

deb%:
	cp debian/changelog.in debian/changelog
	dch -v $(long_ver) "Automatically built package"
	sed -e s/PGVER/$(subst deb,,$@)/g < debian/packages.in > debian/packages
	yada rebuild
	debuild -uc -us -b
