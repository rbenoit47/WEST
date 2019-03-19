EXEC=WEStats.Abs
SRC_FILES=WEStats_main.c WEStats_namelist.ftn WEStats_utils.c fst_utils.c norm_it.ftn stats_we.ftn

LIBRMN=rmn_015.2

default: $(EXEC)

compile: $(EXEC)

$(EXEC): $(SRC_FILES)
	r.compile -src $(SRC_FILES) -bidon c -main mymain -librmn $(LIBRMN) -o $(EXEC)
	strip $(EXEC)

clean:
	@echo "Cleaning directory..."
	rm -vrf .fo *.o *.f
	ls $(EXEC)_*.* 2> /dev/null | grep -E '_[[:digit:]]+.' | xargs rm -vf


