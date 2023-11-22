: ? @ . ;
: 2? 2DUP SWAP . . ;
: 8* 8 * ;
: PAUSE ;
: NOOP ;

CR .( Constants )

0 NOT CONSTANT TRUE
0 CONSTANT FALSE
32 CONSTANT BL
64 CONSTANT C/L
16 CONSTANT L/SCR
7 CONSTANT BELL

1 CONSTANT DOCREATE
2 CONSTANT DOCONSTANT
291 CONSTANT DOCOLON
292 CONSTANT DODOES
293 CONSTANT DODEFER

CR .( Block I/O Constants )

( These have been copied from KERNEL86.BLK )

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
: BUFFER#    ( n -- adr ) 8* >BUFFERS +   ;
: >UPDATE    ( -- adr )   1 BUFFER# 6 +  ;

CR .( Variables )

VARIABLE  OFFSET      ( RELATIVE TO ABSOLUTE DISK BLOCK 0 )
VARIABLE  HLD         ( POINTS TO LAST CHARACTER HELD IN PAD )
VARIABLE  FILE        ( POINTS TO FCB OF CURRENTLY OPEN FILE )
VARIABLE  IN-FILE     ( POINTS TO FCB OF CURRENTLY OPEN FILE )
VARIABLE  PRINTING

VARIABLE END?
VARIABLE 'TIB
VARIABLE WARNING TRUE WARNING !
VARIABLE FENCE

TIB 'TIB !
TRUE CAPS !

CR .( Address manipulation )

: TRAVERSE ( addr dir -- addr ) OVER C@ 1+ * + ;

: BODY> ( pfa -- cfa ) 4 - ;
: NAME> ( nfa -- cfa ) 1 TRAVERSE 4 + ;
: LINK> ( lfa -- cfa ) 2+ ;
: >BODY ( cfa -- pfa ) 4 + ;
: >NAME ( cfa -- nfa ) 4 - -1 TRAVERSE ;
: >LINK ( cfa -- lfa ) 2- ;
: N>LINK ( nfa -- lfa ) NAME> >LINK ;
: L>NAME ( lda -- nda ) LINK> >NAME ;
: >VIEW >NAME 4 - ;
: VIEW> 4 + NAME> ;

CR .( Basic control structures )

: COMPILE ( -- ) R> DUP 2+ >R @ , ;
: C, HERE C! 1 ALLOT ;

: <MARK    HERE ;
: <RESOLVE , ;
: >MARK    HERE 0 , ;
: >RESOLVE HERE SWAP ! ;

: IF ( -- sys ) COMPILE ?BRANCH >MARK ; IMMEDIATE
: ELSE ( sys -- sys ) COMPILE BRANCH >MARK SWAP >RESOLVE ; IMMEDIATE
: THEN ( sys -- ) >RESOLVE ; IMMEDIATE

: ALIGN ( -- ) HERE 1 AND IF BL C, THEN ; IMMEDIATE
: EVEN ( n -- n ) DUP 1 AND + ; IMMEDIATE

: (DO) R> -ROT SWAP >R >R >R ;
: (LOOP) R> R> 1+ DUP R@ = -ROT >R >R ;
: (+LOOP) R> SWAP R> DUP R@ = -ROT + >R SWAP >R ;
: (LOOP-EXIT) R> R> R> 2DROP >R ;
: I 1 RPICK ;
: J 3 RPICK ;
: (LEAVE) R> R> DROP R@ 1- >R >R ;
: LEAVE COMPILE (LEAVE) ; IMMEDIATE
: (?LEAVE) IF R> R> DROP R@ >R >R THEN ;
: ?LEAVE COMPILE (?LEAVE) ; IMMEDIATE
: UNLOOP COMPILE (LOOP-EXIT) ; IMMEDIATE

: BEGIN ( -- sys ) <MARK ; IMMEDIATE
: UNTIL ( sys -- ) COMPILE ?BRANCH <RESOLVE ; IMMEDIATE
: AGAIN ( sys -- ) COMPILE BRANCH <RESOLVE ; IMMEDIATE
: WHILE ( sys -- sys ) [COMPILE] IF ; IMMEDIATE
: REPEAT ( sys -- ) 2SWAP [COMPILE] AGAIN [COMPILE] THEN ; IMMEDIATE

: DO ( w1 w2 -- sys ) -1 [COMPILE] LITERAL [COMPILE] IF COMPILE (DO) <MARK ; IMMEDIATE
: ?DO ( w1 w2 -- sys ) COMPILE 2DUP COMPILE <> [COMPILE] IF COMPILE (DO) <MARK ; IMMEDIATE
: LOOP  ( sys -- ) COMPILE (LOOP)  COMPILE ?BRANCH <RESOLVE COMPILE (LOOP-EXIT) [COMPILE] ELSE COMPILE 2DROP [COMPILE] THEN ; IMMEDIATE
: +LOOP ( sys -- ) COMPILE (+LOOP) COMPILE ?BRANCH <RESOLVE COMPILE (LOOP-EXIT) [COMPILE] ELSE COMPILE 2DROP [COMPILE] THEN ; IMMEDIATE

CR .( Misc commands )

: >=    1- > ;
: <=    1+ < ;
: 0>=   0< NOT ;
: 0<=   0> NOT ;

: RECURSE ( -- ) LAST @ NAME> , ; IMMEDIATE
: TAIL-RECURSE ( -- ) LAST @ NAME> >BODY COMPILE BRANCH <RESOLVE ; IMMEDIATE
: (.S) ( depth i -- ) 2DUP <> IF 2DUP - 1+ PICK 7 .R SPACE 1+ TAIL-RECURSE THEN ;
: .S ( -- ) DEPTH 0 (.S) 2DROP ;

: ?STACK ;

( These have been copied from KERNEL86.BLK )

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

: COMPARE CAPS @ IF CAPS-COMP ELSE COMP THEN ;

: MU/MOD >R 0 R@ UM/MOD R> SWAP >R UM/MOD R> ;
: ?NEGATE 0< IF NEGATE THEN ;
: M/MOD   ( d# n1 -- rem quot )
   ?DUP
   IF  DUP >R  2DUP XOR >R  >R DABS R@ ABS  UM/MOD
     SWAP R> ?NEGATE
     SWAP R> 0< IF  NEGATE OVER IF  1- R@ ROT - SWAP  THEN THEN
     R> DROP
   THEN  ;

CR .( Create )

: VIEW#  ( -- addr ) FILE @ 40 + ;
: ,VIEW  ( -- )      BLK @ DUP , IF VIEW# @ ELSE 0 THEN , ;
: (CREATE)   ( str type -- )
    SWAP
    ( Set name )    COUNT DUP >R HERE 4 + PLACE R> DUP HERE 4 + + 1+ C!
    ,VIEW
    ( Set immd )    0 HERE 1 TRAVERSE 1+ C!
    ( Set link )    LAST @ HERE N>LINK !
    ( Set type )    HERE NAME> !
    ( Set code p )  HERE NAME> DUP 2+ !
    ( Set LAST )    HERE LAST !
    ( Set CURRENT ) HERE CURRENT @ !
    ( Allot )       HERE NAME> >BODY HERE - ALLOT
    ;

CR .( DEFER BLOCK )
( 5         B              L              O              C              K )
  5 HERE C! 66 HERE 1 + C! 76 HERE 2 + C! 79 HERE 3 + C! 67 HERE 4 + C! 75 HERE 5 + C!
HERE DODEFER (CREATE) 2 ALLOT

CR .( DEFER EMIT )
' EMIT
( 4         E              M              I              T )
  4 HERE C! 69 HERE 1 + C! 77 HERE 2 + C! 73 HERE 3 + C! 84 HERE 4 + C!
HERE DODEFER (CREATE) 2 ALLOT
' EMIT >BODY !

CR .( DEFER TYPE )
' TYPE
( 4         T              Y              P              E )
  4 HERE C! 84 HERE 1 + C! 89 HERE 2 + C! 80 HERE 3 + C! 69 HERE 4 + C!
HERE DODEFER (CREATE) 2 ALLOT
' TYPE >BODY !

CR .( Printing )

( These have been copied from KERNEL86.BLK )

8 CONSTANT BS

: (CONSOLE)   PAUSE 6 BDOS DROP 1 #OUT +! ;
: PR-STAT ( -- f )   TRUE   ( 0 15 BIOS )   ;
: (PRINT)   ( char -- )
   BEGIN  PAUSE  PR-STAT  UNTIL  5 BDOS DROP  1 #OUT +!  ;
: (EMIT)   ( char -- )
   PRINTING @ IF  DUP (PRINT)  -1 #OUT +!  THEN  (CONSOLE)  ;
: CRLF   ( -- )  13 EMIT   10 EMIT   #OUT OFF  1 #LINE +! ;
: CR CRLF ;
: (TYPE)  ( addr len -- )  0 ?DO  COUNT EMIT  LOOP   DROP   ;

: SPACE  ( -- )     BL EMIT   ;
: SPACES ( n -- )   0 MAX   0 ?DO   SPACE   LOOP   ;
: BACKSPACES   ( n -- )     0 ?DO   BS EMIT   LOOP   ;
: BEEP   ( -- )     BELL EMIT   ;

CR .( Parsing )

( These have been copied from KERNEL86.BLK )

: SOURCE ( addr len ) BLK @ ?DUP IF BLOCK B/BUF ELSE TIB #TIB @ THEN  ;
: /STRING ( addr len n -- addr' len' ) OVER MIN ROT OVER + -ROT - ;
: PARSE      ( char -- addr len ) >R SOURCE      >IN @ /STRING         OVER SWAP R> SCAN >R OVER - DUP R>     0<>   - >IN +! ;
: PARSE-WORD ( char -- addr len ) >R SOURCE TUCK >IN @ /STRING R@ SKIP OVER SWAP R> SCAN >R OVER - ROT R> DUP 0<> + - >IN ! ;
: 'WORD ( -- addr ) HERE ;
: WORD ( char -- addr ) ( Parse word ) PARSE-WORD ( Insert into 'WORD as a counted string) 'WORD PLACE ( Insert a space at the end ) 'WORD COUNT + BL SWAP C! ( Return 'WORD ) 'WORD ;
: ASCII ( -- u ) BL WORD 1+ C@ STATE @ IF [COMPILE] LITERAL THEN ; IMMEDIATE
: DEFINED ( -- cfa ) BL WORD ?UPPERCASE DUP C@ IF FIND ELSE DROP END? ON  ['] NOOP 1 THEN ;
: (.ID) ( addr len -- ) DUP 0<> IF 0 (DO) [ <MARK ] DUP I + C@ EMIT (LOOP) ?BRANCH [ <RESOLVE ] (LOOP-EXIT) 32 EMIT ELSE DROP THEN DROP ;
: .ID ( addr -- ) COUNT (.ID) ;
: >TYPE  TUCK PAD SWAP CMOVE PAD SWAP TYPE ;

: ?ERROR ( addr len flag -- ) IF (.ID) CR ABORT ELSE 2DROP THEN ;
: (ABORT") ( flag -- ) R@ COUNT ROT ?ERROR R> COUNT + >R ;
: ," 34 PARSE TUCK 'WORD PLACE 1+ ALLOT ;
: (") R> COUNT 2DUP + >R ;
: " COMPILE (") ," ; IMMEDIATE
: ABORT" COMPILE (ABORT") ," ; IMMEDIATE
: (.") R@ COUNT TYPE R> COUNT + >R ;
: ." COMPILE (.") ," ; IMMEDIATE

: ?MISSING ( f -- ) IF   'WORD COUNT TYPE   TRUE ABORT"  ?"   THEN   ;

: WARM ( -- ) TRUE ABORT" Warm Start";

CR .( Comments )

: .( ASCII ) PARSE TYPE ; IMMEDIATE
: ( ASCII ) PARSE 2DROP ; IMMEDIATE
: (S ASCII ) PARSE 2DROP ; IMMEDIATE
: \S END? ON ; IMMEDIATE

: \  
  BLK @
  IF
    >IN @ NEGATE C/L MOD >IN +!
  ELSE
    SOURCE >IN ! DROP
  THEN
; IMMEDIATE

CR .( Definitions Part 1 )

: ' ( -- addr ) DEFINED 0= ?MISSING ;
: ['] ( -- ) ' [COMPILE] LITERAL ; IMMEDIATE
: [COMPILE] ( -- ) ' , ; IMMEDIATE
: CREATE ( -- )  BL WORD ?UPPERCASE DOCREATE (CREATE) ;
: CONSTANT ( value -- ) CREATE , DOES> @ ;
: VARIABLE ( -- ) CREATE 0 , ;

CR .( Double extension set )

: 2CONSTANT ( dvalue -- ) CREATE , , DOES> 2@ ;
: 2VARIABLE ( -- ) 0 0 2CONSTANT DOES> ;

CR .( Deferred words )

: CRASH ."  Uninitialized execution vector." ABORT ;
: DEFER CREATE ['] CRASH HERE !  2 ALLOT  DODEFER LAST @ NAME> ! ;
: (IS) R@ @ >BODY ! R> 2+ >R ;
: IS STATE @ IF COMPILE (IS) ELSE ' >BODY ! THEN ; IMMEDIATE

DEFER KEY?
DEFER KEY

CR .( Control structures )

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
: REPEAT ( sys -- ) 2SWAP [COMPILE] AGAIN [COMPILE] THEN ; IMMEDIATE

.( Do )

: DO ( w1 w2 -- sys ) -1 [COMPILE] LITERAL [COMPILE] IF COMPILE (DO) ?<MARK ; IMMEDIATE
: ?DO ( w1 w2 -- sys ) COMPILE 2DUP COMPILE <> [COMPILE] IF COMPILE (DO) ?<MARK ; IMMEDIATE
: LOOP  ( sys -- ) COMPILE (LOOP)  COMPILE ?BRANCH ?<RESOLVE COMPILE (LOOP-EXIT) [COMPILE] ELSE COMPILE 2DROP [COMPILE] THEN ; IMMEDIATE
: +LOOP ( sys -- ) COMPILE (+LOOP) COMPILE ?BRANCH ?<RESOLVE COMPILE (LOOP-EXIT) [COMPILE] ELSE COMPILE 2DROP [COMPILE] THEN ; IMMEDIATE

CR .( Misc commands )

: .S' ( -- ) DEPTH ?DUP IF 0 DO DEPTH I - 1- PICK 7 U.R SPACE LOOP ELSE ." Empty" THEN ;
DEFER STATUS ' CR IS STATUS

CR .( ONLY vocabulary )

: VOCABULARY ( -- ) CREATE 0 , DOES> CONTEXT ! ;
: DEFINITIONS ( -- ) CONTEXT @ CURRENT ! ;

( Set the code pointer of FORTH )
' VOCABULARY >BODY 10 + ' FORTH 2+ !

CONTEXT DUP @ SWAP 2+ !
VOCABULARY ROOT ROOT DEFINITIONS

: ALSO ( -- ) CONTEXT DUP 2+ #VOCS 2- 2* CMOVE> ;
: ONLY ( -- ) ['] ROOT >BODY CONTEXT #VOCS 1- 2* 2DUP ERASE + ! ROOT ;
: DEFINITIONS DEFINITIONS ;
: FORTH FORTH ;
: ORDER ( -- )
    CR ." Context: "
    CONTEXT
    #VOCS 0 DO
        DUP @
        ?DUP IF
            BODY> >NAME .ID
        THEN
        2+
    LOOP
    DROP
    CR ." Current: "
    CURRENT @ BODY> >NAME .ID ;
: PRINT-VOC ( -- )
    ?DUP IF
        DUP .ID
        N>LINK @ TAIL-RECURSE
    THEN ;
: WORDS ( -- )
    CONTEXT
    #VOCS 0 DO
        DUP @
        ?DUP IF
            DUP BODY> >NAME CR .ID
            ." : "
            @ PRINT-VOC
        THEN
        2+
    LOOP ;

ONLY FORTH ALSO DEFINITIONS

CR .( Number output )

\ These have been copied from KERNEL86.BLK

: HEX ( -- )     16 BASE ! ;
: DECIMAL ( -- ) 10 BASE ! ;
: OCTAL ( -- )    8 BASE ! ;

: HOLD ( char -- ) -1 HLD +! HLD @ C! ;
: <# ( -- ) PAD HLD ! ;
: #> ( d# -- addr len ) 2DROP HLD @ PAD OVER - ;
: SIGN ( n1 -- ) 0< IF ASCII - HOLD THEN ;
: # ( -- ) BASE @ MU/MOD ROT 9 OVER < IF 7 + THEN ASCII 0 + HOLD ;
: #S ( -- ) BEGIN # 2DUP OR 0= UNTIL ;

: (U.)  ( u -- a l )   0    <# #S #>   ;
: U.    ( u -- )       (U.)   TYPE SPACE   ;
: U.R   ( u l -- )     >R   (U.)   R> OVER - SPACES   TYPE   ;

: (.)   ( n -- a l )   DUP ABS 0   <# #S   ROT SIGN   #>   ;
: .     ( n -- )       (.)   TYPE SPACE   ;
: .R    ( n l -- )     >R   (.)   R> OVER - SPACES   TYPE   ;

: (UD.) ( ud -- a l )  <# #S #>   ;
: UD.   ( ud -- )      (UD.)   TYPE SPACE   ;
: UD.R  ( ud l -- )    >R   (UD.)   R> OVER - SPACES   TYPE  ;

: (D.)  ( d -- a l )   TUCK DABS   <# #S   ROT SIGN  #>   ;
: D.    ( d -- )       (D.)   TYPE SPACE   ;
: D.R   ( d l -- )     >R   (D.)   R> OVER - SPACES   TYPE   ;

CR .( Number input )

\ These have been copied from KERNEL86.BLK

VARIABLE DPL

: BETWEEN  ( n1 min max -- f ) >R  OVER > SWAP R> > OR NOT ;
: WITHIN   ( n1 min max -- f ) 1- BETWEEN  ;
: BOUNDS  ( adr len -- lim first ) OVER + SWAP ;

: DOUBLE?   ( -- f ) DPL @ 1+ 0<> ;
: CONVERT   ( +d1 adr1 -- +d2 adr2 )
    BEGIN
        1+
        DUP >R
        C@ BASE @ DIGIT
    WHILE
        SWAP BASE @ UM* DROP ROT
        BASE @ UM*  D+
        DOUBLE? IF
            1 DPL +!
        THEN
        R>
    REPEAT
    DROP
    R> ;

: (NUMBER?) ( adr -- d flag )
    0 0 ROT
    DUP 1+ C@ ASCII - = DUP >R - -1 DPL !
    BEGIN
        CONVERT
        DUP C@ ASCII , ASCII / BETWEEN
    WHILE
        0 DPL !
    REPEAT
    -ROT
    R> IF
        DNEGATE
    THEN
    ROT
    C@ BL =  ;
: NUMBER?   ( adr -- d flag )
    FALSE OVER
    COUNT BOUNDS
    2DUP <> IF DO
        I C@ BASE @ DIGIT
        NIP IF
            DROP TRUE
            LEAVE
        THEN
    LOOP THEN
    IF
        (NUMBER?)
    ELSE
        DROP
        0 0 FALSE
    THEN  ;
: (NUMBER)  ( adr -- d# ) NUMBER? NOT ?MISSING  ;

DEFER NUMBER ' (NUMBER) IS NUMBER

CR .( Interpreter )

: DLITERAL   SWAP [COMPILE] LITERAL [COMPILE] LITERAL ; IMMEDIATE
: DONE?     ( n -- f ) STATE @ <>   END? @ OR   END? OFF   ;

: PRINT-HEX BASE @ 16 BASE ! SWAP 0 <# # # # # #> TYPE BASE ! space ;
: (.SH) ( depth i -- ) 2DUP <> IF 2DUP - 1+ PICK 3 spaces print-hex SPACE 1+ TAIL-RECURSE THEN ;
: .SH ( -- ) DEPTH 0 (.SH) 2DROP ;

: [ ( -- ) STATE OFF ; IMMEDIATE
: ] ( -- )
    STATE ON
    BEGIN
        DEFINED
        DUP IF
            0> IF
                EXECUTE
            ELSE
                ,
            THEN
        ELSE
            DROP NUMBER
            DOUBLE? IF
                [COMPILE] DLITERAL
            ELSE
                DROP [COMPILE] LITERAL
            THEN
        THEN
        TRUE DONE?
    UNTIL ;

( : dump 2dup 0 do dup i + c@ . loop drop 0 do dup i + c@ emit loop drop ; )

: VISIBLE? [ HEX ] 21 7E BETWEEN [ DECIMAL ] ;
: DUMP ( addr len -- )
    BASE @ >R HEX
    DUP 16 MOD 0<> ABORT" Length%16!=0"
    0 DO
        CR DUP 0 <# # # # # #> TYPE SPACE
        16 0 DO
            SPACE
            DUP I + C@ 0 <# # # #> TYPE
        LOOP
        SPACE ." |"
        16 0 DO
            DUP I + C@ DUP VISIBLE? IF
                EMIT
            ELSE
                DROP ASCII . EMIT
            THEN
        LOOP
        ." |"
        16 +
    16 +LOOP
    DROP
    R> BASE ! ;

: INTERPRET ( -- )
    BEGIN
        ( cr ." file:" file @ 1+ 13 type ."  blk:" blk @ . ." offset:" offset @ . source >in @ /string swap ." addr:" . ." len:" . )
        DEFINED IF
        ( dup >name ." executing " .id )
            EXECUTE
        ELSE
        ( dup ." pushing " .id )
            NUMBER DOUBLE? NOT
            IF DROP THEN
        THEN
        ( cr .s )
        FALSE DONE?
    UNTIL ;
: RUN ( -- )
    STATE @ IF
        ]
        STATE @ NOT IF
            INTERPRET
        THEN
    ELSE
        INTERPRET
    THEN ;

CR .( Definitions Part 2 )

VARIABLE CSP -1 CSP !
: !CSP ( -- ) DEPTH CSP ! ;
: ?CSP ( -- ) DEPTH CSP @ <> -1 CSP @ <> AND ABORT" Stack Changed" ;

: HIDE     LAST @ ( DUP ) N>LINK @ ( SWAP ) CURRENT @ ( HASH ) ! ;
: REVEAL   LAST @ ( DUP   N>LINK     SWAP ) CURRENT @ ( HASH ) ! ;
: RECURSIVE REVEAL ; IMMEDIATE

: ; ( -- )
    ?CSP
    COMPILE EXIT
    REVEAL
    [COMPILE] [
; IMMEDIATE

: : ( -- )
    !CSP
    CURRENT @ CONTEXT !
    CREATE
    HIDE
    ]
    LAST @ NAME>
    DUP DOCOLON SWAP !
    DUP SWAP 2+ !
;

DROP ( C implementation of : pushes the address to the stack, but our implementation of ; does not pop it )

: (;USES) R> @ LAST @ NAME> ! ;
: ;USES   ?CSP COMPILE (;USES) [COMPILE] [ REVEAL ; IMMEDIATE

DEFER UNNEST
' EXIT IS UNNEST

CR .( Block I/O )

\ These have been copied from KERNEL86.BLK

DEFER READ-BLOCK    ( buffer-header -- )
DEFER WRITE-BLOCK   ( buffer-header -- )
: .FILE   ( adr -- ) COUNT ?DUP IF  ASCII @ + EMIT ." :"  THEN 8 2DUP -TRAILING TYPE + ." ." 3 TYPE SPACE  ;
: FILE?   ( -- )   FILE @ .FILE  ;
: SWITCH   ( -- )   FILE @ IN-FILE @ FILE ! IN-FILE !  ;

VOCABULARY DOS   DOS DEFINITIONS
: !FILES   ( fcb -- )   DUP FILE !  IN-FILE !  ;
: DISK-ABORT   ( fcb a n -- ) TYPE ."  "  .FILE  ABORT  ;
: ?DISK-ERROR  ( fcb n -- ) DUP DISK-ERROR ! IF DISK-ABORT ELSE DROP THEN  ;

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
: LATEST?   ( n fcb -- fcb n | a f )
   DISK-ERROR OFF
   SWAP OFFSET @ + 2DUP   1 BUFFER# 2@   D=
   IF   2DROP   1 BUFFER# 4 + @   FALSE   R> DROP THEN  ;
: ABSENT?   ( n fcb -- a f )
   LATEST?  FALSE #BUFFERS 1+ 2
   DO  DROP 2DUP I BUFFER# 2@ D=
     IF  2DROP I LEAVE  ELSE  FALSE  THEN
   LOOP  ?DUP
   IF  BUFFER# DUP >BUFFERS 8 CMOVE   >R  >BUFFERS DUP 8 +
     OVER R> SWAP  -  CMOVE>     1 BUFFER# 4 + @ FALSE
   ELSE  >BUFFERS 2! TRUE  THEN ;

: UPDATE   ( -- )   >UPDATE ON   ;
: DISCARD  ( -- )   1 >UPDATE ! ( 1 BUFFER# ON ) ;
: MISSING  ( -- ) >END 2- @ 0< IF  >END 2- OFF  >END 8 - WRITE-BLOCK  THEN >END 4 - @  >BUFFERS 4 + ! ( buffer )  1 >BUFFERS 6 + ! >BUFFERS DUP 8 + #BUFFERS 8* CMOVE>   ;
: (BUFFER) ( n fcb -- a )   PAUSE  ABSENT?  IF  MISSING  1 BUFFER#   4 + @  THEN  ;
: BUFFER   ( n -- a )   FILE @ (BUFFER)  ;
: (BLOCK)  ( n fcb -- a ) (BUFFER)  >UPDATE @ 0> IF  1 BUFFER#  DUP READ-BLOCK  6 + OFF  THEN  ;
: DBLOCK    ( n -- a )   FILE @ (BLOCK)  ;
: IN-BLOCK ( n -- a )   IN-FILE @ (BLOCK)  ;

' DBLOCK IS BLOCK

: EMPTY-BUFFERS ( -- ) FIRST LIMIT OVER - ERASE >BUFFERS #BUFFERS 1+ 8* ERASE FIRST 1 BUFFER#   #BUFFERS 0 DO   DUP ON  4 +  2DUP !   SWAP B/BUF + SWAP  4 + LOOP   2DROP   ;
: SAVE-BUFFERS  ( -- ) 1 BUFFER#   #BUFFERS 0 DO   DUP @ 1+ IF  DUP 6 + @ 0< IF  DUP WRITE-BLOCK  DUP 6 + OFF  THEN 8 + THEN   LOOP   DROP   ;
: FLUSH         ( -- ) SAVE-BUFFERS  0 BLOCK DROP  EMPTY-BUFFERS  ;

DOS DEFINITIONS
: FILE-SIZE   ( fcb -- n )   DUP 35 BDOS  DROP  RECORD# @ ;
: DOS-ERR?    ( -- f )   255 =    ;
: OPEN-FILE   ( -- )   IN-FILE @ DUP 15 BDOS DOS-ERR? IF  " Open error" DISK-ABORT  THEN DUP FILE-SIZE 1-  SWAP MAXREC# !  ;
HEX 5C CONSTANT DOS-FCB   DECIMAL
FORTH DEFINITIONS
: DEFAULT    ( -- )   [ DOS ]   FCB1 DUP IN-FILE !  DUP FILE ! CLR-FCB   DOS-FCB 1+ C@ BL <> IF   DOS-FCB FCB1 12 CMOVE  OPEN-FILE   THEN   ;
: (LOAD)     ( n -- )  [ DOS ] FILE @ >R   BLK @ >R   >IN @ >R >IN OFF  BLK !   IN-FILE @ FILE !   RUN   R> >IN !   R> BLK ! R> !FILES  ;
DEFER LOAD
' (LOAD) IS LOAD

CR .( File I/O )

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
: C:     ( -- )   [ DOS ]  2 SELECT ;
DOS DEFINITIONS

: FILE:   ( -- fcb ) >IN @   CREATE   >IN !   HERE DUP   B/FCB ALLOT   !FCB DOES>  !FILES  ;
: ?DEFINE ( -- fcb ) >IN @  DEFINED IF   NIP >BODY   ELSE   DROP  >IN ! FILE:   THEN  ;
FORTH DEFINITIONS
: DEFINE  ( -- )   [ DOS ]  ?DEFINE  DROP  ;
: OPEN    ( -- )   [ DOS ]  ?DEFINE  !FILES     OPEN-FILE  ;
: FROM    ( -- )   [ DOS ]  ?DEFINE  IN-FILE !  OPEN-FILE  ;
: SAVE-SYSTEM   ( -- ) [ DOS HEX ] 100 HERE SAVE  ;  DECIMAL

CREATE VIEW-FILES   40 ALLOT  VIEW-FILES 40 ERASE
: VIEWS   ( n -- )   [ DOS ] ?DEFINE 2DUP  40 + !   BODY> SWAP 2* VIEW-FILES + !  ;

\ 1 VIEWS KERNEL86.BLK
\ 2 VIEWS EXTEND86.BLK
\ 3 VIEWS CPU8086.BLK
\ 4 VIEWS UTILITY.BLK

FILE-IO

\ These have been copied from UTILITY.BLK

VARIABLE SCR 0 SCR !

: .SCR   ( -- )   ." Screen # " SCR ? 8 SPACES ;
: LIST   ( n -- )
    1 ?ENOUGH
    CR
    DUP SCR !
    .SCR
    L/SCR 0 DO
        CR I 3 .R SPACE
        DUP BLOCK I C/L * + C/L -TRAILING >TYPE
    KEY? ?LEAVE
    LOOP
    DROP CR ;

: N   ( -- )      1 SCR +!  DISK-ERROR OFF  ;
: B   ( -- )     -1 SCR +!  DISK-ERROR OFF  ;
: L   ( -- )     SCR @ LIST   ;
: ESTABLISH   ( n -- )   FILE @ SWAP  1 BUFFER# 2! ;
: (COPY)   ( from to -- )
   OFFSET @ + SWAP IN-BLOCK DROP  ESTABLISH UPDATE ;
: COPY   FLUSH (COPY) FLUSH ;
: @VIEW   ( code-field -- scr file# )
   >VIEW DUP @ DUP 0= ABORT" entered at terminal"
   SWAP 2+ @  ;
: VIEW   ( -- )   [ DOS ]  ' @VIEW  ?DUP
   IF   2* VIEW-FILES + @  ." is in: " 2DUP >BODY .FILE
     ." screen " . EXECUTE OPEN-FILE
   ELSE  ." may be in current file: " FILE? ." screen "
DUP . THEN LIST ;

CR .( KEY )

: (KEY?)  0 11 BDOS 0<> ;
: (KEY)   BEGIN PAUSE (KEY?) UNTIL 0 8 BDOS ;

: MOCK-KEY? FALSE ;
: MOCK-KEY + ;

' MOCK-KEY? IS KEY?
' MOCK-KEY IS KEY

: RAW-MODE ['] (KEY?) IS KEY?
           ['] (KEY)  IS KEY ;

CR .( Terminal I/O commands )

\ These have been copied from KERNEL86.BLK

: BS-IN   ( n c -- 0 | n-1 )
   DROP DUP IF   1-   BS   ELSE   BELL   THEN   EMIT   ;
: (DEL-IN)   ( n c -- 0 | n-1 )
   DROP DUP IF  1-  BS EMIT SPACE BS  ELSE  BELL  THEN  EMIT  ;
: BACK-UP ( n c -- 0 )
   DROP   DUP BACKSPACES   DUP SPACES   BACKSPACES   0   ;
: RES-IN   ( c -- )
   FORTH   TRUE ABORT" Reset" ;
: P-IN  ( c -- )
   DROP   PRINTING @ NOT PRINTING !  ;

: CR-IN ( m a n c -- m a m )
   DROP   SPAN !   OVER   BL EMIT   ;
: (CHAR)   ( a n char -- a n+1 )
   3DUP EMIT + C!   1+   ;
DEFER CHAR ' (CHAR) IS CHAR
DEFER DEL-IN ' (DEL-IN) IS DEL-IN

VARIABLE CC
CREATE CC-FORTH
 ] CHAR    CHAR   CHAR   CHAR   CHAR   CHAR    CHAR   CHAR
   BS-IN   CHAR   CHAR   CHAR   CHAR   CR-IN   CHAR   CHAR
   P-IN    CHAR   CHAR   CHAR   CHAR   BACK-UP CHAR   CHAR
   BACK-UP CHAR   RES-IN CHAR   CHAR   CHAR    CHAR   CHAR [

' CC-FORTH >BODY CC !

: PERFORM @ EXECUTE ;

: EXPECT   ( adr len -- )
   DUP SPAN !   SWAP 0   ( len adr 0 )
   BEGIN   2 PICK OVER - ( len adr #so-far #left )
   WHILE   KEY DUP BL <
     IF   DUP 2* CC @ + PERFORM
     ELSE DUP 127 = IF   DEL-IN   ELSE   CHAR   THEN
     THEN REPEAT    2DROP DROP   ;

DEFER $JEMMA ' NOOP IS $JEMMA

: QUERY   ( -- )
   TIB 255 EXPECT SPAN @ #TIB !
   $JEMMA BLK OFF  >IN OFF  ;

: CTOGGLE ( n addr -- addr ) DUP C@ ROT XOR SWAP C! ;

: QUIT   ( -- )
   SP0 @ 'TIB !    BLK OFF   [COMPILE] [
   BEGIN RP0 @ RP! STATUS QUERY  RUN
      STATE @ NOT IF   ."  ok"   THEN   AGAIN  ;

: START ( -- ) EMPTY-BUFFERS ( DEFAULT ) ;
DEFER BOOT
: HELLO ( -- )
    CR ." Virtual Forth 83"
    CR ." Version 0.1"
    START
    ONLY FORTH ALSO DEFINITIONS
;
' HELLO IS BOOT

: COLD RAW-MODE BOOT QUIT ;
: WARM TRUE ABORT" Warm Start" ;

1 CONSTANT INITIAL
: OK    ( -- )   INITIAL LOAD ;
: BYE   ( -- )
   CR   HERE 0 256 UM/MOD NIP 1+   DECIMAL U.   ." Pages"
   0 0 BDOS  ;

CR .( VIDIO )

HEX

: combine ( high low -- n ) FF and swap 100 * + ;
: split ( n -- high low ) dup 100 / swap FF and ;

: (vidio) ( ax bx cx dx -- ax bx cx dx ) 0 0 int10h 2drop ;
: vidio02 ( -- row col ) 0300 0 0 0 (vidio) >r 3drop r> split ;
: vidio10 ( mode -- ) 0 0 0 (vidio) 2drop 2drop ;
: vidio10cha ( char -- ) FF and 900 or 2 1 0 (vidio) 2drop 2drop ;
: vidio20c ( row col -- ) combine 0200 swap 0 swap 0 swap (vidio) 2drop 2drop ;
: vidio21 ( row col -- color ) 0d00 -rot 0 -rot (vidio) 3drop ;
: vidio30 ( row col color -- ) -rot 0 -rot (vidio) 2drop 2drop ;
: vidio30h ( di bp ax -- ) -rot swap 0 -rot 0 -rot 0 -rot int10h 3drop 3drop ;
: vidio40 ( dx cx bx ax -- ) swap >r >r swap r> -rot r> -rot (vidio) 2drop 2drop ;
: vidio ( ax bx cx dx -- ax bx cx dx ) (vidio) ;
: ibm-at ( col row -- ) swap combine 200 swap 0 swap 0 swap (vidio) 2drop 2drop ;
: ibm-dark ( -- ) 0700 0 0 0 (vidio) 2drop 2drop ;
: ibm-blot   ( col -- )   80 SWAP - SPACES   ;
: zerofii drop ;

DECIMAL

: debug breakpoint ;
: unbug cr ." unbug: Not implemented" ;
: bye 0 0 BDOS ;
: clear depth 0 ?do drop loop ;

: save-system defined drop cr ." save-system: Not implemented" ;

CR .( Screen utils )

\ These have been copied fro UTIL.BLK

VARIABLE HOPPED   ( # screens copy is offset )
VARIABLE U/D
DEFER CONVEY-COPY   ' (COPY) IS CONVEY-COPY
: HOP   ( n -- ) ( specifies n screens to skip )  HOPPED ! ;
: .TO  ( #1 #2 -- #1 #2 )  CR  OVER . ." to "  DUP . ;
: (CONVEY)   (S blk n -- blk+-n )
   0 ?DO   KEY? ?LEAVE   DUP DUP HOPPED @ + .TO
      CONVEY-COPY   U/D @ +   LOOP   FLUSH   ;
: CONVEY   (S first last -- )
   FLUSH   HOPPED @ 0< IF   1+ OVER - 1
   ELSE   DUP 1+ ROT - -1   THEN U/D !   #BUFFERS /MOD
   >R (CONVEY) R> 0 ?DO #BUFFERS (CONVEY) LOOP   DROP   ;
: TO   ( #1st-source #last-source -- #1st-source #last-source )
(  #1st-dest must follow TO )
   SWAP   BL WORD  NUMBER DROP   OVER -   HOP   SWAP   ;

CR .( Additional ANS FORTH Words )

: >NUMBER  ( ud1 c-addr1 u1 -- ud2 c-addr2 u2 )
    DUP 0 ?DO
        SWAP
        DUP C@ BASE @ DIGIT
        NOT IF
            DROP
            SWAP
            LEAVE
        ELSE
            0
            ( ud1 u1 c-addr1 value 0 )
            2ROT
            BASE @ 1 M*/
            D+
            ( u1 c-addr1 base*ud1+value )
            2SWAP
            1+
            SWAP
            1-
        THEN
    LOOP
;
: ACCEPT   ( addr +n -- +n ) SPAN @ -ROT EXPECT SWAP SPAN ! ;
: ALIGNED  ( n -- n ) DUP 1 AND + ;
: CELLS    ( n -- n ) 2* ;
: CELL+    ( n -- n ) 2+ ;
: CHAR     ( n -- n ) [COMPILE] ASCII ;
: [CHAR]   ( n -- n ) [COMPILE] ASCII ; IMMEDIATE
: CHARS    ( n -- n ) ;
: CHAR+    ( n -- n ) 1+ ;
: INVERT   ( n -- n ) NOT ;
: S"                  [COMPILE] " ; IMMEDIATE

: POSTPONE
    '
    DUP >link 1- C@
    IF
        ,
    ELSE
        [COMPILE] LITERAL COMPILE ,
    THEN
; IMMEDIATE

EMPTY-BUFFERS

