: N>LINK ( nfa -- lfa ) 3 + ;
: L>NAME ( lda -- nda ) 3 - ;
: BODY> ( pfa -- cfa ) 3 - ;
: NAME> ( nfa -- cfa ) 5 + ;
: LINK> ( lfa -- cfa ) 2+ ;
: >BODY ( cfa -- pfa ) 3 + ;
: >NAME ( cfa -- nfa ) 5 - ;
: >LINK ( cfa -- lfa ) 2- ;


: VOCABULARY ( -- ) CREATE 0 , DOES> CONTEXT ! ;
: DEFINITIONS ( -- ) CONTEXT @ CURRENT ! ;

VOCABULARY ONLY ONLY DEFINITIONS

: ALSO ( -- ) CONTEXT DUP 2+ #VOCS 2- 2* CMOVE> ;
: DEFINITIONS DEFINITIONS ;
: FORTH FORTH ;

ONLY FORTH ALSO DEFINITIONS


: COMPILE ( -- ) R> DUP 2+ >R @ , ;
: IF ( addr --) COMPILE ?BRANCH >MARK ; IMMEDIATE
: ELSE ( -- ) COMPILE BRANCH >MARK SWAP >RESOLVE ; IMMEDIATE
: THEN ( -- ) >RESOLVE ; IMMEDIATE