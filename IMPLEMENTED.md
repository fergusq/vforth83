# Forth 83 Standard Implementation Status
## REQUIRED WORD SET
### Nucleus layer
|Word|Is implemented|
|:--:|:------------:|
|`!`|✅|
|`*`|✅|
|`*/`|✅|
|`*/MOD`|✅|
|`+`|✅|
|`+!`|✅|
|`-`|✅|
|`/`|✅|
|`/MOD`|✅|
|`0<`|✅|
|`0=`|✅|
|`0>`|✅|
|`1+`|✅|
|`1-`|✅|
|`2+`|✅|
|`2-`|✅|
|`2/`|✅|
|`<`|✅|
|`=`|✅|
|`>`|✅|
|`>R`|✅|
|`?DUP`|✅|
|`@`|✅|
|`ABS`|✅|
|`AND`|✅|
|`C!`|✅|
|`C@`|✅|
|`CMOVE`|✅|
|`CMOVE>`|✅|
|`COUNT`|✅|
|`D+`|✅|
|`D<`|✅|
|`DEPTH`|✅|
|`DNEGATE`|✅|
|`DROP`|✅|
|`DUP`|✅|
|`EXECUTE`|✅|
|`EXIT`|✅|
|`FILL`|✅|
|`I`|✅|
|`J`|✅|
|`MAX`|✅|
|`MIN`|✅|
|`MOD`|✅|
|`NEGATE`|✅|
|`NOT`|✅|
|`OR`|✅|
|`OVER`|✅|
|`PICK`|✅|
|`R>`|✅|
|`R@`|✅|
|`ROLL`|✅|
|`ROT`|✅|
|`SWAP`|✅|
|`U<`|✅|
|`UM*`|✅|
|`UM/MOD`|✅|
|`XOR`|✅|
### Device layer
|Word|Is implemented|
|:--:|:------------:|
|`BLOCK`|✅|
|`BUFFER`|✅|
|`CR`|✅|
|`EMIT`|✅|
|`EXPECT`|✅|
|`FLUSH`|✅|
|`KEY`|✅|
|`SAVE-BUFFERS`|✅|
|`SPACE`|✅|
|`SPACES`|✅|
|`TYPE`|✅|
|`UPDATE`|✅|
### Interpreter layer
|Word|Is implemented|
|:--:|:------------:|
|`#`|✅|
|`#>`|✅|
|`#S`|✅|
|`#TIB`|✅|
|`'`|✅|
|`(`|✅|
|`-TRAILING`|✅|
|`.`|✅|
|`.(`|✅|
|`<#`|✅|
|`>BODY`|✅|
|`>IN`|✅|
|`ABORT`|✅|
|`BASE`|✅|
|`BLK`|✅|
|`CONVERT`|✅|
|`DECIMAL`|✅|
|`DEFINITIONS`|✅|
|`FIND`|✅|
|`FORGET`|✅|
|`FORTH`|✅|
|`FORTH-83`|✅|
|`HERE`|✅|
|`HOLD`|✅|
|`LOAD`|✅|
|`PAD`|✅|
|`QUIT`|✅|
|`SIGN`|✅|
|`SPAN`|✅|
|`TIB`|✅|
|`U.`|✅|
|`WORD`|✅|
### Compiler layer
|Word|Is implemented|
|:--:|:------------:|
|`+LOOP`|✅|
|`,`|✅|
|`."`|✅|
|`:`|✅|
|`;`|✅|
|`ABORT"`|✅|
|`ALLOT`|✅|
|`BEGIN`|✅|
|`COMPILE`|✅|
|`CONSTANT`|✅|
|`CREATE`|✅|
|`DO`|✅|
|`DOES>`|✅|
|`ELSE`|✅|
|`IF`|✅|
|`IMMEDIATE`|✅|
|`LEAVE`|✅|
|`LITERAL`|✅|
|`LOOP`|✅|
|`REPEAT`|✅|
|`STATE`|✅|
|`THEN`|✅|
|`UNTIL`|✅|
|`VARIABLE`|✅|
|`VOCABULARY`|✅|
|`WHILE`|✅|
|`[`|✅|
|`[']`|✅|
|`[COMPILE]`|✅|
|`]`|✅|
## The Double Number Extension Word Set Layers
### Nucleus layer
|Word|Is implemented|
|:--:|:------------:|
|`2!`|✅|
|`2@`|✅|
|`2DROP`|✅|
|`2DUP`|✅|
|`2OVER`|✅|
|`2ROT`|✅|
|`2SWAP`|✅|
|`D+`|✅|
|`D-`|✅|
|`D0=`|✅|
|`D2/`|✅|
|`D<`|✅|
|`D=`|✅|
|`DABS`|✅|
|`DMAX`|✅|
|`DMIN`|✅|
|`DNEGATE`|✅|
|`DU<`|✅|
### Interpreter layer
|Word|Is implemented|
|:--:|:------------:|
|`D.`|✅|
|`D.R`|✅|
### Compiler layer
|Word|Is implemented|
|:--:|:------------:|
|`2CONSTANT`|✅|
|`2VARIABLE`|✅|
## The Assembler Extension Word Set Layers
### Interpreter layer
|Word|Is implemented|
|:--:|:------------:|
|`ASSEMBLER`|❌|
### Compiler layer
|Word|Is implemented|
|:--:|:------------:|
|`;CODE`|❌|
|`CODE`|❌|
|`END-CODE`|❌|
## The System Extension Word Set Layers
### Nucleus layer
|Word|Is implemented|
|:--:|:------------:|
|`BRANCH`|✅|
|`?BRANCH`|✅|
### Interpreter layer
|Word|Is implemented|
|:--:|:------------:|
|`CONTEXT`|✅|
|`CURRENT`|✅|
### Compiler layer
|Word|Is implemented|
|:--:|:------------:|
|`<MARK`|✅|
|`<RESOLVE`|✅|
|`>MARK`|✅|
|`>RESOLVE`|✅|
## CONTROLLED REFERENCE WORDS
|Word|Is implemented|
|:--:|:------------:|
|`-->`|✅|
|`.R`|✅|
|`2*`|✅|
|`BL`|✅|
|`BLANK`|✅|
|`C,`|✅|
|`DUMP`|✅|
|`EDITOR`|❌|
|`EMPTY-BUFFERS`|✅|
|`END`|❌|
|`ERASE`|✅|
|`HEX`|✅|
|`INTERPRET`|✅|
|`K`|❌|
|`LIST`|✅|
|`OCTAL`|✅|
|`OFFSET`|✅|
|`QUERY`|✅|
|`RECURSE`|✅|
|`SCR`|✅|
|`SP@`|✅|
|`THRU`|✅|
|`U.R`|✅|
### UNCONTROLLED REFERENCE WORDS
|Word|Is implemented|
|:--:|:------------:|
|`!BITS`|❌|
|`**`|❌|
|`+BLOCK`|❌|
|`-'`|❌|
|`-MATCH`|❌|
|`-TEXT`|❌|
|`/LOOP`|❌|
|`1+!`|❌|
|`1-!`|❌|
|`;:`|❌|
|`;S`|❌|
|`<>`|✅|
|`<BUILDS`|❌|
|`<CMOVE`|❌|
|`><`|❌|
|`>MOVE<`|❌|
|`@BITS`|❌|
|`AGAIN`|✅|
|`ASCII`|✅|
|`ASHIFT`|❌|
|`B/BUF`|✅|
|`BELL`|✅|
|`CHAIN`|❌|
|`CONTINUED`|❌|
|`CUR`|❌|
|`DBLOCK`|✅|
|`DPL`|✅|
|`FLD`|❌|
|`H.`|❌|
|`I'`|❌|
|`IFEND`|❌|
|`IFTRUE`|❌|
|`INDEX`|❌|
|`LAST`|✅|
|`LINE`|❌|
|`LINELOAD`|❌|
|`LOADS`|❌|
|`MAP0`|❌|
|`MASK`|❌|
|`MOVE`|✅|
|`MS`|❌|
|`NAND`|❌|
|`NOR`|❌|
|`NUMBER`|✅|
|`O.`|❌|
|`OTHERWISE`|❌|
|`PAGE`|❌|
|`READ-MAP`|❌|
|`REMEMBER`|❌|
|`REWIND`|❌|
|`ROTATE`|❌|
|`S0`|❌|
|`SET`|❌|
|`SHIFT`|❌|
|`TEXT`|❌|
|`USER`|❌|
|`WORDS`|✅|
|`\LOOP`|❌|
## EXPERIMENTAL PROPOSALS
### SEARCH ORDER SPECIFICATION AND CONTROL
|Word|Is implemented|
|:--:|:------------:|
|`ONLY`|✅|
|`FORTH`|✅|
|`ALSO`|✅|
|`ORDER`|✅|
|`WORDS`|✅|
|`FORGET`|✅|
|`DEFINITIONS`|✅|
|`SEAL`|❌|
### DEFINITION FIELD ADDRESS CONVERSION OPERATORS
|Word|Is implemented|
|:--:|:------------:|
|`>BODY`|✅|
|`>NAME`|✅|
|`>LINK`|✅|
|`BODY>`|✅|
|`NAME>`|✅|
|`LINK>`|✅|
|`N>LINK`|✅|
|`L>NAME`|✅|
