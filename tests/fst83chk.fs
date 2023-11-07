( -*- forth -*- )

: checking ( [text<:>] -- )
    10 parse ." /////// " type cr
;

: checks: ( [text< >].. -- )
	8 spaces
	begin
		bl word 
		dup dup if c@ then 
	while
		dup count type space
		dup find if 2drop
		else cr ." missing " count type cr 8 spaces
		then
	repeat
        cr
	drop
;

include fst83_12.fs
include fst83_13.fs
include fst83_14.fs
include fst83_15.fs
include fst83_16.fs
include fst83_b.fs
include fst83_c.fs





