: ? DUP . ;
: 2? 2DUP SWAP . . ;
: 8* 8 * ;
: PAUSE ;
: NOOP ;
: BOUNDS OVER + SWAP ;

.( Constants ) 10 EMIT

0 NOT CONSTANT TRUE
0 CONSTANT FALSE
32 CONSTANT BL
64 CONSTANT C/L
16 CONSTANT L/SCR

.( Block I/O Constants )

    2 CONSTANT #BUFFERS
 1024 CONSTANT B/BUF
  128 CONSTANT B/REC
    8 CONSTANT REC/BLK
   42 CONSTANT B/FCB
      VARIABLE DISK-ERROR
   -2 CONSTANT LIMIT
       #BUFFERS 1+ 8* 2+ CONSTANT >SIZE
LIMIT B/BUF #BUFFERS * -  CONSTANT FIRST
FIRST >SIZE - CONSTANT INIT-R0
: >BUFFERS   ( -- adr )   FIRST  >SIZE - ;
: >END       ( -- adr )   FIRST  2-  ;
: BUFFER#    ( n -- adr )   8* >BUFFERS +   ;
: >UPDATE    ( -- adr )   1 BUFFER# 6 +  ;

.( Comments )

: \ >IN @ NEGATE C/L MOD >IN +! ;

.( Variables ) 10 EMIT

VARIABLE  #OUT        ( NUMBER OF CHARACTERS EMITTED )
VARIABLE  #LINE       ( THE NUMBER OF LINES SENT SO FAR )
VARIABLE  OFFSET      ( RELATIVE TO ABSOLUTE DISK BLOCK 0 )
VARIABLE  HLD         ( POINTS TO LAST CHARACTER HELD IN PAD )
VARIABLE  FILE        ( POINTS TO FCB OF CURRENTLY OPEN FILE )
VARIABLE  IN-FILE     ( POINTS TO FCB OF CURRENTLY OPEN FILE )
VARIABLE  PRINTING

TRUE CAPS !

.( Double extension set ) 10 EMIT

: 2CONSTANT ( -- ) CREATE , , DOES> 2@ ;
: 2VARIABLE ( -- ) 0 0 2CONSTANT DOES> ;

.( Address manipulation ) 10 EMIT

: BODY> ( pfa -- cfa ) 3 - ;
: NAME> ( nfa -- cfa ) 29 + ;
: LINK> ( lfa -- cfa ) 2+ ;
: >BODY ( cfa -- pfa ) 3 + ;
: >NAME ( cfa -- nfa ) 29 - ;
: >LINK ( cfa -- lfa ) 2- ;
: N>LINK ( nfa -- lfa ) NAME> >LINK ;
: L>NAME ( lda -- nda ) LINK> >NAME ;

.( Basic control structures ) 10 EMIT

: COMPILE ( -- ) R> DUP 2+ >R @ , ;

: IF ( -- sys ) COMPILE ?BRANCH >MARK ; IMMEDIATE
: ELSE ( sys -- sys ) COMPILE BRANCH >MARK SWAP >RESOLVE ; IMMEDIATE
: THEN ( sys -- ) >RESOLVE ; IMMEDIATE

: (DO) R> -ROT SWAP >R >R >R ;
: (LOOP) R> R> 1+ DUP R@ = -ROT >R >R ;
: (+LOOP) R> R> ROT + DUP R@ = -ROT >R >R ;
: (LOOP-EXIT) R> R> R> 2DROP >R ;
: I R> R@ SWAP >R ;
: (LEAVE) R> R> DROP R@ >R >R ;
: LEAVE COMPILE (LEAVE) ; IMMEDIATE

.( Misc commands ) 10 EMIT

: RECURSE ( -- ) LAST @ NAME> , ; IMMEDIATE
: TAIL-RECURSE ( -- ) LAST @ NAME> >BODY COMPILE BRANCH <RESOLVE ; IMMEDIATE
: (.S) ( depth i -- ) 2DUP <> IF 2DUP - 1+ PICK 7 .R SPACE 1+ TAIL-RECURSE THEN ;
: .S ( -- ) DEPTH 0 (.S) 2DROP ;

: 3DROP ( n1 n2 n3 -- ) 2DROP DROP ;
: 3DUP ( n1 n2 n3 -- n1 n2 n3 n1 n2 n3 ) DUP 2OVER ROT ;
: MOVE ( addr1 addr2 u -- ) -ROT 2DUP U< IF ROT CMOVE> ELSE ROT CMOVE THEN ; ( Copies possibly overlapping memory addresses )
: PLACE ( addr len to -- ) 3DUP 1+ SWAP MOVE C! DROP ; ( Inserts a string to memory as a counted string )
: ON ( addr -- ) TRUE SWAP ! ;
: OFF ( addr -- ) FALSE SWAP ! ;
: ?UPPERCASE ( addr -- addr ) CAPS @ IF DUP COUNT UPPER THEN ;
: ERASE 0 FILL ;
: BLANK BL FILL ;

: PAD ( -- addr ) HERE 80 + ;
: MU/MOD >R 0 R@ UM/MOD R> SWAP >R UM/MOD R> ;

.( Parsing ) 10 EMIT

: SOURCE ( addr len ) BLK @ ?DUP IF ( BLOCK B/BUF ) ELSE TIB #TIB @ THEN  ;
: /STRING ( addr len n -- addr' len' ) OVER MIN ROT OVER + -ROT - ;
: PARSE      ( char -- addr len ) >R SOURCE      >IN @ /STRING         OVER SWAP R> SCAN >R OVER - DUP R>     0<>   - >IN +! ;
: PARSE-WORD ( char -- addr len ) >R SOURCE TUCK >IN @ /STRING R@ SKIP OVER SWAP R> SCAN >R OVER - ROT R> DUP 0<> + - >IN ! ;
: 'WORD ( -- addr ) HERE ;
: WORD ( char -- addr ) ( Parse word ) PARSE-WORD ( Insert into 'WORD as a counted string) 'WORD PLACE ( Insert a space at the end ) 'WORD COUNT + BL SWAP C! ( Return 'WORD ) 'WORD ;
: ASCII ( -- u ) BL WORD 1+ C@ STATE @ IF [COMPILE] LITERAL THEN ; IMMEDIATE
: DEFINED ( -- cfa ) BL WORD ?UPPERCASE FIND ;
: (.ID) ( addr len -- ) DUP 0<> IF 0 (DO) [ <MARK ] DUP I + C@ EMIT (LOOP) ?BRANCH [ <RESOLVE ] (LOOP-EXIT) 32 EMIT ELSE DROP THEN DROP ;
: .ID ( addr -- ) COUNT (.ID) ;

.( Control structures ) 10 EMIT

: ?ERROR ( addr len flag -- ) IF (.ID) CR ABORT ELSE 2DROP THEN ;
: (ABORT") ( flag -- ) R@ COUNT ROT ?ERROR R> COUNT + >R ;
: ," 34 PARSE TUCK 'WORD PLACE 1+ ALLOT ;
: (") R> COUNT 2DUP + >R ;
: " COMPILE (") ," ; IMMEDIATE
: ABORT" COMPILE (ABORT") ," ; IMMEDIATE
: ?CONDITION ( flag -- ) NOT ABORT" Conditionals Wrong" ;
: ?>MARK ( -- flag addr ) TRUE >MARK ;
: ?>RESOLVE ( flag addr -- ) SWAP ?CONDITION >RESOLVE ;
: ?<MARK ( -- flag addr ) TRUE <MARK ;
: ?<RESOLVE ( flag addr -- ) SWAP ?CONDITION <RESOLVE ;

.( If )

: IF ( -- sys ) COMPILE ?BRANCH ?>MARK ; IMMEDIATE
: ELSE ( sys -- sys ) COMPILE BRANCH ?>MARK 2SWAP ?>RESOLVE ; IMMEDIATE
: THEN ( sys -- ) ?>RESOLVE ; IMMEDIATE

.( Begin )

: BEGIN ( -- sys ) ?<MARK ; IMMEDIATE
: UNTIL ( sys -- ) COMPILE ?BRANCH ?<RESOLVE ; IMMEDIATE
: AGAIN ( sys -- ) COMPILE BRANCH ?<RESOLVE ; IMMEDIATE
: WHILE ( sys -- sys ) [COMPILE] IF ; IMMEDIATE
: REPEAT ( sys -- ) SWAP [COMPILE] AGAIN [COMPILE] THEN ; IMMEDIATE

.( Do )

: DO ( w1 w2 -- sys ) COMPILE (DO) ?<MARK ; IMMEDIATE
: LOOP  ( sys -- ) COMPILE (LOOP)  COMPILE ?BRANCH ?<RESOLVE COMPILE (LOOP-EXIT) ; IMMEDIATE
: +LOOP ( sys -- ) COMPILE (+LOOP) COMPILE ?BRANCH ?<RESOLVE COMPILE (LOOP-EXIT) ; IMMEDIATE

.( Misc commands ) 10 EMIT

: .S' ( -- ) DEPTH ?DUP IF 0 DO DEPTH I - 1- PICK 7 U.R SPACE LOOP ELSE ." Empty" THEN ;

.( Deferred words ) 10 EMIT

: CRASH   TRUE ABORT"  Uninitialized execution vector." ;
: DEFER CREATE ['] CRASH HERE !  2 ALLOT  5 LAST @ NAME> C! ;
: (IS) R@ @ >BODY ! R> 2+ >R ;
: IS STATE @ IF COMPILE (IS) ELSE ' >BODY ! THEN ;

.( ONLY vocabulary ) 10 EMIT

: VOCABULARY ( -- ) CREATE 0 , DOES> CONTEXT ! ;
: DEFINITIONS ( -- ) CONTEXT @ CURRENT ! ;

\ Set the code pointer of FORTH
' VOCABULARY >BODY 10 + ' FORTH 1+ !

CONTEXT DUP @ SWAP 2+ !
VOCABULARY ROOT ROOT DEFINITIONS

: ALSO ( -- ) CONTEXT DUP 2+ #VOCS 2- 2* CMOVE> ;
: ONLY ( -- ) ['] ROOT >BODY CONTEXT #VOCS 1- 2* 2DUP ERASE + ! ROOT ;
: DEFINITIONS DEFINITIONS ;
: FORTH FORTH ;
: ORDER ( -- ) CR ." Context: " CONTEXT   #VOCS 0 DO   DUP @ ?DUP IF BODY> >NAME .ID THEN   2+ LOOP DROP CR ." Current: " CURRENT @ BODY> >NAME .ID ;
: PRINT-VOC ( -- ) ?DUP IF DUP .ID N>LINK @ TAIL-RECURSE THEN ;
: WORDS ( -- ) CONTEXT #VOCS 0 DO DUP @ ?DUP IF DUP BODY> >NAME CR .ID ." : " @ PRINT-VOC THEN 2+ LOOP ;

ONLY FORTH ALSO DEFINITIONS

.( Number output ) 10 EMIT

: HEX ( -- )     16 BASE ! ;
: DECIMAL ( -- ) 10 BASE ! ;
: OCTAL ( -- )    8 BASE ! ;

: HOLD ( char -- ) -1 HLD +! HLD @ C! ;
: <# ( -- ) PAD HLD ! ;
: #> ( d# -- addr len ) 2DROP HLD @ PAD OVER - ;
: SIGN ( n1 -- ) 0< IF ASCII - HOLD THEN ;
: # ( -- ) BASE @ MU/MOD ROT 9 OVER < IF 7 + THEN ASCII 0 + HOLD ;
: #S ( -- ) BEGIN # 2DUP OR 0= UNTIL ;

.( Interpreter )

: RUN   ( -- ) STATE @ IF   ]   STATE @ NOT IF   INTERPRET   THEN ELSE   INTERPRET   THEN   ;

.( Block I/O )

\ These have been copied from KERNEL86.BLK

DEFER READ-BLOCK    ( buffer-header -- )
DEFER WRITE-BLOCK   ( buffer-header -- )
: .FILE   ( adr -- ) COUNT ?DUP IF  ASCII @ + EMIT ." :"  THEN 8 2DUP -TRAILING TYPE + ." ." 3 TYPE SPACE  ;
: FILE?   ( -- )   FILE @ .FILE  ;
: SWITCH   ( -- )   FILE @ IN-FILE @ FILE ! IN-FILE !  ;

VOCABULARY DOS   DOS DEFINITIONS
: !FILES   ( fcb -- )   DUP FILE !  IN-FILE !  ;
: DISK-ABORT   ( fcb a n -- ) TYPE ."  "  .FILE  ABORT  ;
: ?DISK-ERROR  ( fcb n -- ) DUP DISK-ERROR ! IF  " Disk error" DISK-ABORT ELSE DROP THEN  ;

CREATE FCB1   B/FCB ALLOT
: CLR-FCB    ( fcb -- )    DUP  B/FCB ERASE 1+ 11 BLANK ;
: SET-DMA    ( adr -- )   26 BDOS  DROP ;
: RECORD#    ( fcb -- adr )   33 + ;
: MAXREC#    ( fcb -- adr )   38 + ;
: IN-RANGE   ( fcb -- fcb ) DUP MAXREC# @ OVER RECORD# @ U<  DUP DISK-ERROR ! IF  1 BUFFER# ON  " Out of Range" DISK-ABORT  THEN  ;
: REC-READ   ( fcb -- ) DUP IN-RANGE  33 BDOS  ?DISK-ERROR ;
: REC-WRITE  ( fcb -- ) DUP IN-RANGE  34 BDOS  ?DISK-ERROR ;

: SET-IO     ( buf-header -- file buffer rec/blk 0 ) DUP 2@ REC/BLK * OVER RECORD# ! SWAP 4 + @ ( buf-addr )   REC/BLK 0  ;
: FILE-READ  ( buffer-header -- ) SET-IO DO   2DUP SET-DMA  DUP REC-READ   1 SWAP RECORD# +!  B/REC + LOOP  2DROP  ;
: FILE-WRITE ( buffer-header -- ) SET-IO DO   2DUP SET-DMA  DUP REC-WRITE  1 SWAP RECORD# +!  B/REC + LOOP  2DROP  ;
: FILE-IO    ( -- ) ['] FILE-READ IS READ-BLOCK  ['] FILE-WRITE IS WRITE-BLOCK ;

FORTH DEFINITIONS
: CAPACITY  ( -- n ) [ DOS ]   FILE @ MAXREC# @ 1+ 0 8 UM/MOD NIP ;
: LATEST?   ( n fcb -- fcb n ö a f ) DISK-ERROR OFF SWAP OFFSET @ + 2DUP   1 BUFFER# 2@   D= IF   2DROP   1 BUFFER# 4 + @   FALSE   R> DROP  THEN  ;
: ABSENT?   ( n fcb -- a f ) LATEST?  FALSE #BUFFERS 1+ 2 DO  DROP 2DUP I BUFFER# 2@ D= IF  2DROP I LEAVE  ELSE  FALSE  THEN LOOP  ?DUP IF  BUFFER# DUP >BUFFERS 8 CMOVE   >R  >BUFFERS DUP 8 + OVER R> SWAP  -  CMOVE>     1 BUFFER# 4 + @ FALSE ELSE  >BUFFERS 2! TRUE  THEN  ;

: UPDATE   ( -- )   >UPDATE ON   ;
: DISCARD  ( -- )   1 >UPDATE ! ( 1 BUFFER# ON ) ;
: MISSING  ( -- ) >END 2- @ 0< IF  >END 2- OFF  >END 8 - WRITE-BLOCK  THEN >END 4 - @  >BUFFERS 4 + ! ( buffer )  1 >BUFFERS 6 + ! >BUFFERS DUP 8 + #BUFFERS 8* CMOVE>   ;
: (BUFFER) ( n fcb -- a )   PAUSE  ABSENT? IF  MISSING  1 BUFFER#   4 + @  THEN  ;
: BUFFER   ( n -- a )   FILE @ (BUFFER)  ;
: (BLOCK)  ( n fcb -- a ) (BUFFER)  >UPDATE @ 0> IF  1 BUFFER#  DUP READ-BLOCK  6 + OFF  THEN  ;
: BLOCK    ( n -- a )   FILE @ (BLOCK)  ;
: IN-BLOCK ( n -- a )   IN-FILE @ (BLOCK)  ;

: EMPTY-BUFFERS ( -- ) FIRST LIMIT OVER - ERASE >BUFFERS #BUFFERS 1+ 8* ERASE FIRST 1 BUFFER#   #BUFFERS 0 DO   DUP ON  4 +  2DUP !   SWAP B/BUF + SWAP  4 + LOOP   2DROP   ;
: SAVE-BUFFERS  ( -- ) 1 BUFFER#   #BUFFERS 0 DO   DUP @ 1+ IF  DUP 6 + @ 0< IF  DUP WRITE-BLOCK  DUP 6 + OFF  THEN 8 + THEN   LOOP   DROP   ;
: FLUSH         ( -- ) SAVE-BUFFERS  0 BLOCK DROP  EMPTY-BUFFERS  ;
: VIEW#         ( -- addr )    FILE @ 40 +   ;

DOS DEFINITIONS
: FILE-SIZE   ( fcb -- n )   DUP 35 BDOS  DROP  RECORD# @ ;
: DOS-ERR?    ( -- f )   255 =    ;
: OPEN-FILE   ( -- )   IN-FILE @ DUP 15 BDOS DOS-ERR? IF  " Open error" DISK-ABORT  THEN DUP FILE-SIZE 1-  SWAP MAXREC# !  ;
HEX 5C CONSTANT DOS-FCB   DECIMAL
FORTH DEFINITIONS
: DEFAULT    ( -- )   [ DOS ]   FCB1 DUP IN-FILE !  DUP FILE ! CLR-FCB   DOS-FCB 1+ C@ BL <> IF   DOS-FCB FCB1 12 CMOVE  OPEN-FILE   THEN   ;
: (LOAD)     ( n -- )  [ DOS ] FILE @ >R   BLK @ >R   >IN @ >R >IN OFF  BLK !   IN-FILE @ FILE !   RUN   R> >IN !   R> BLK ! R> !FILES  ;
DEFER LOAD

.( File I/O )

\ These have been copied from EXTEND.BLK

: ?ENOUGH   ( n -- ) DEPTH 1- > ABORT" Stack underflow"   ;
: THRU   ( n1 n2 -- ) 2 ?ENOUGH   1+ SWAP 2DUP <> IF DO   I LOAD   LOOP THEN  ;
: +THRU   ( n1 n2 -- ) BLK @ + SWAP   BLK @ + SWAP   THRU   ;
: -->      ( -- ) >IN OFF   1 BLK +!   ;   IMMEDIATE

DOS DEFINITIONS

CREATE FCB2   B/FCB ALLOT
: RESET    ( -- )   0     13 BDOS DROP ;
: CLOSE    ( fcb -- )     16 BDOS DOS-ERR? ABORT" Close error"  ;
: SEARCH0  ( fcb -- n )   17 BDOS ;
: SEARCH   ( fcb -- n )   18 BDOS ;
: DELETE   ( fcb -- n )   19 BDOS ;
: READ     ( fcb -- )     20 BDOS DOS-ERR? ABORT" Read error"  ;
: WRITE    ( fcb -- )     21 BDOS DOS-ERR? ABORT" Write error"  ;
: MAKE-FILE   ( fcb -- )  22 BDOS DOS-ERR? ABORT" Can't MAKE File "  ;

: (!FCB) ( Addr len FCB-addr --- ) DUP B/FCB ERASE   DUP 1+ 11 BLANK >R  OVER 1+ C@ ASCII : = IF   OVER C@ [ ASCII A 1- ] LITERAL -  R@ C!   2 /STRING THEN   R> 1+ -ROT 0 DO   DUP C@ ASCII . = IF   SWAP 8 I - + ELSE   2DUP C@ SWAP C!   SWAP 1+  THEN  SWAP 1+ LOOP 2DROP   ;
: !FCB   ( FCB-addr ) BL WORD COUNT  CAPS @ IF  2DUP UPPER  THEN  ROT  (!FCB)  ;
: SELECT ( drive -- ) ( DUP 9 BIOS 0= ABORT" Illegal drive " )  14 BDOS  DROP ;

DEFER HEADER   ' NOOP IS HEADER
: SAVE   ( Addr len --- ) FCB2 DUP !FCB  DUP DELETE DROP  DUP MAKE-FILE -ROT  HEADER BOUNDS 2DUP <> IF DO  I SET-DMA  DUP WRITE  128 +LOOP THEN  CLOSE  ;
FORTH DEFINITIONS
: MORE        ( n -- )       [ DOS ] 1 ?ENOUGH  CAPACITY SWAP   DUP 8* FILE @ MAXREC# +!  BOUNDS 2DUP <> IF DO  I BUFFER B/BUF BLANK UPDATE  LOOP THEN SAVE-BUFFERS  FILE @ CLOSE  ;
: CREATE-FILE ( #blocks -- ) [ DOS ]  FCB2 DUP !FILES  DUP !FCB  MAKE-FILE  MORE  ;

DOS DEFINITIONS
: .NAME   ( n -- ) #OUT @ C/L > IF CR THEN  32 * PAD + 1+ 8 2DUP TYPE SPACE + 3 TYPE 3 SPACES  ;
FORTH DEFINITIONS
: DIR    ( -- )   [ DOS ] " ????????.???" FCB2 (!FCB) CR  PAD SET-DMA  FCB2 SEARCH0 BEGIN  .NAME  FCB2 SEARCH  DUP DOS-ERR? UNTIL  DROP  ;
: DRIVE? ( -- )   0 25 BDOS ASCII A + EMIT ." : "  ;
: A:     ( -- )   [ DOS ]  0 SELECT ;
: B:     ( -- )   [ DOS ]  1 SELECT ;
DOS DEFINITIONS