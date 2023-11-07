( -*- forth -*- )

checking ================= REQUIRED WORD SET ====================

checking Nucleus layer

checks: ! *

checks:  !  *  */  */MOD  +  +!  -  /  /MOD  0<  0=  0>  1+  1-  2+

checks:  2-  2/  <  =  >  >R  ?DUP  @  ABS  AND  C!  C@  CMOVE

checks:  CMOVE>  COUNT  D+  D<  DEPTH  DNEGATE  DROP  DUP  EXECUTE

checks:  EXIT  FILL  I  J  MAX  MIN  MOD  NEGATE  NOT  OR  OVER  PICK

checks:  R>  R@  ROLL  ROT  SWAP  U<  UM*  UM/MOD  XOR

checking Device layer

checks:  BLOCK  BUFFER  CR  EMIT  EXPECT  FLUSH  KEY  SAVE-BUFFERS

checks:  SPACE  SPACES  TYPE  UPDATE


checking Interpreter layer

checks:  #  #>  #S  #TIB  '  (  -TRAILING  .  .(  <#  >BODY  >IN

checks:  ABORT  BASE  BLK  CONVERT  DECIMAL  DEFINITIONS  FIND

checks:  FORGET  FORTH  FORTH-83  HERE  HOLD  LOAD  PAD  QUIT  SIGN

checks:  SPAN  TIB  U.  WORD


checking  Compiler layer

checks:   +LOOP  ,  ."  :  ;  ABORT"  ALLOT  BEGIN  COMPILE  CONSTANT

checks:   CREATE  DO  DOES>  ELSE  IF  IMMEDIATE  LEAVE  LITERAL  LOOP

checks:   REPEAT  STATE  THEN  UNTIL  VARIABLE  VOCABULARY  WHILE  [

checks:   [']  [COMPILE]  ]

