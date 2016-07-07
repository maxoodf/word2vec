EXTENSION = pg_doc2vec        # the extensions name
DATA = pg_doc2vec--1.0.1.sql  # script files to install
#REGRESS = doc2vec_test     # our test script file (without extension)
#MODULE_big = doc2vec
MODULES = pg_doc2vec

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
SHARE_FOLDER := $(shell $(PG_CONFIG) --sharedir)
DOC_LEN_MAX := 65536
NEAREST_RESULT_MAX := 9999
DOC2VEC_TASK_THREADS := 8

CXXFLAGS = -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wformat-security -fno-strict-aliasing -fwrapv -fstack-protector-strong -Wformat -Werror=format-security -fPIC -fno-omit-frame-pointer -O3 -std=c++11 -I../../word2vec -I../../doc2vec -I../../wordReader -DSHARE_FOLDER="$(SHARE_FOLDER)" -DDOC_LEN_MAX=$(DOC_LEN_MAX) -DNEAREST_RESULT_MAX=$(NEAREST_RESULT_MAX) -DDOC2VEC_TASK_THREADS=$(DOC2VEC_TASK_THREADS)
#SHLIB_LINK = -lc++

include $(PGXS)
