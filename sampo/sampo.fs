.( LOADING SAMPO )

 1 VIEWS  KERNHPU.BLK     2 VIEWS    EXTEND.BLK
 3 VIEWS        CPU.BLK   4 VIEWS     UTIHP.BLK
 5 VIEWS     EDITOR.BLK   6 VIEWS    TURTLE.BLK
 7 VIEWS     LISTAT.BLK   8 VIEWS   CONTROL.BLK
 9 VIEWS       MUUT.BLK  10 VIEWS      EVAL.BLK
11 VIEWS   sampogen.BLK  12 VIEWS      help.BLK
13 VIEWS       BASE.BLK  14 VIEWS   GRKURSO.BLK
15 VIEWS         es.BLK  16 VIEWS     sampo.BLK
17 views     selita.blk  18 VIEWS      auta.blk

define sampo.blk
define auta.blk

from editor.blk 1 load

: HPINI 6 VIDIO10 START ; ' HPINI IS BOOT
' CO(BSPEISI) IS BSPEISI
' CO(SCROLL) IS SCROLL
PATCH S(CONSOLE) (CONSOLE)          ' S(KEY) IS KEY
PATCH SQUIT QUIT DECIMAL 6 VIDIO10

from sampogen.blk 1 load

' (EMIT) IS EMIT
' (TYPE) IS TYPE

clear 0 0 0 0

DECIMAL

luo :x nel 4 kertaa :x eteen 90 oikea valmis
luo :x kukka 8 kertaa :x nel 45 oikea valmis

luo :f kuvaaja 500 kertaa i i :f tee piste valmis
luo sin90 drop sin<90 0 drop valmis
luo sin45 drop sin<45 0 drop valmis
luo uusi 0 0 (vidio) 2drop 2drop valmis

