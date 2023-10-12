( Constants )

0 NOT CONSTANT TRUE
0 CONSTANT FALSE
32 CONSTANT BL
1024 CONSTANT B/BUF

( Variables )

VARIABLE  #OUT        ( NUMBER OF CHARACTERS EMITTED )          
VARIABLE  #LINE       ( THE NUMBER OF LINES SENT SO FAR )       
VARIABLE  OFFSET      ( RELATIVE TO ABSOLUTE DISK BLOCK 0 )     
VARIABLE  BASE        ( FOR NUMERIC INPUT AND OUTPUT )          
VARIABLE  HLD         ( POINTS TO LAST CHARACTER HELD IN PAD )  
VARIABLE  FILE        ( POINTS TO FCB OF CURRENTLY OPEN FILE )  
VARIABLE  IN-FILE     ( POINTS TO FCB OF CURRENTLY OPEN FILE )  
VARIABLE  PRINTING

( Double extension set )

: 2CONSTANT ( -- ) CREATE , , DOES> 2@ ;
: 2VARIABLE ( -- ) 0 0 2CONSTANT DOES> ;

( Address manipulation )

: N>LINK ( nfa -- lfa ) 3 + ;
: L>NAME ( lda -- nda ) 3 - ;
: BODY> ( pfa -- cfa ) 3 - ;
: NAME> ( nfa -- cfa ) 5 + ;
: LINK> ( lfa -- cfa ) 2+ ;
: >BODY ( cfa -- pfa ) 3 + ;
: >NAME ( cfa -- nfa ) 5 - ;
: >LINK ( cfa -- lfa ) 2- ;

( Basic control structures )

: COMPILE ( -- ) R> DUP 2+ >R @ , ;

: IF ( -- sys ) COMPILE ?BRANCH >MARK ; IMMEDIATE
: ELSE ( sys -- sys ) COMPILE BRANCH >MARK SWAP >RESOLVE ; IMMEDIATE
: THEN ( sys -- ) >RESOLVE ; IMMEDIATE

: BEGIN ( -- sys ) <MARK ; IMMEDIATE
: UNTIL ( sys -- ) COMPILE ?BRANCH <RESOLVE ; IMMEDIATE
: AGAIN ( sys -- ) COMPILE BRANCH <RESOLVE ; IMMEDIATE
: WHILE ( sys -- sys ) [COMPILE] IF ; IMMEDIATE
: REPEAT ( sys -- ) SWAP [COMPILE] AGAIN [COMPILE] THEN ; IMMEDIATE

( Misc commands )

: 3DUP ( n1 n2 n3 -- n1 n2 n3 n1 n2 n3 ) DUP 2OVER ROT ;
: MOVE -ROT 2DUP U< IF ROT CMOVE> THEN ROT CMOVE ;
: PLACE ( addr len to -- ) 3DUP 1+ SWAP MOVE C! DROP ;
: ON ( addr -- ) TRUE SWAP ! ;
: OFF ( addr -- ) FALSE SWAP ! ;

( Parsing )

: SOURCE BLK @ ?DUP IF BLOCK B/BUF ELSE TIB #TIB @ ;
: /STRING ( addr len n -- addr' len' ) OVER MIN ROT OVER + -ROT - ;
: PARSE      ( char -- addr len ) >R SOURCE      >IN @ /STRING         OVER SWAP R> SCAN >R OVER - DUP R>     0<>   - >IN +! ;
: PARSE-WORD ( char -- addr len ) >R SOURCE TUCK >IN @ /STRING R@ SKIP OVER SWAP R> SCAN >R OVER - ROT R> DUP 0<> + - >IN ! ;
: 'WORD HERE ;
: WORD PARSE-WORD 'WORD PLACE 'WORD DUP COUNT + BL SWAP C! ;

( Number output )

: HOLD ( char -- ) -1 HLD +!   HLD @ C!   ;                
: <# ( -- ) PAD HLD ! ;                             
: #> ( d# -- addr len ) 2DROP HLD @ PAD OVER - ;  
: SIGN ( n1 -- ) 0< IF ASCII - HOLD THEN ;              
: # ( -- ) BASE @ MU/MOD ROT 9 OVER < IF 7 + THEN ASCII 0 + HOLD ;   
: #S ( -- ) BEGIN # 2DUP OR 0= UNTIL ;

: HEX ( -- )     16 BASE ! ;                             
: DECIMAL ( -- ) 10 BASE ! ;                             
: OCTAL ( -- )    8 BASE ! ;

( Control structures )

: ?ERROR ;
: (ABORT") R@ COUNT ROT ?ERROR R> COUNT + >R ;
: ," 34 PARSE TUCK 'WORD PLACE 1+ ALLOT ;
: ABORT" COMPILE (ABORT") ," ; IMMEDIATE
: ?CONDITION NOT ABORT" Conditionals Wrong ;
: ?>MARK ( -- flag addr ) TRUE >MARK ;
: ?>RESOLVE ( flag addr -- ) SWAP ?CONDITION >RESOLVE ;
: IF ( -- sys ) COMPILE ?BRANCH ?>MARK ; IMMEDIATE
: ELSE ( sys -- sys ) COMPILE BRANCH ?>MARK 2SWAP ?>RESOLVE ; IMMEDIATE
: THEN ( sys -- ) ?>RESOLVE ; IMMEDIATE

( ONLY vocabulary )

: VOCABULARY ( -- ) CREATE 0 , DOES> CONTEXT ! ;
: DEFINITIONS ( -- ) CONTEXT @ CURRENT ! ;

CONTEXT DUP @ SWAP 2+ !
VOCABULARY ONLY ONLY DEFINITIONS

: ALSO ( -- ) CONTEXT DUP 2+ #VOCS 2- 2* CMOVE> ;
: DEFINITIONS DEFINITIONS ;
: FORTH FORTH ;

ONLY FORTH ALSO DEFINITIONS

( Misc commands )

: ASCII ( -- n ) BL WORD 1+ C@ STATE @ IF [COMPILE] LITERAL THEN ; IMMEDIATE