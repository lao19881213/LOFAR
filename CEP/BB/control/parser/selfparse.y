%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parser/selfparse.h"

    /* <todo> Not all three of these are needed. Did I choose ok? </todo> */
    //int topLevel  = 1; /* true that is, if we have a toplevel to deal with */
   unsigned int level; /* look at the "control" produktion rules for explanation */
    //int topScript = 1; /* first token indicates script */

%}

//%debug
//%defines
    //%destructor
//%locations
%pure-parser
//%verbose


%union {
      long int number;
      float real;
      char *branchId;
      char *block;
      char *command;
      char *options;
      char *param;
      char *dimension;
      char *subScript;
}

%token <branchId> BRANCH
%token <branchId> REF
%token <branchId> REFBRANCH
%token <number> NUMBER
%token <real> REAL
%token LBRACE
%token RBRACE
%token COMMA
%token RANGESIGN
%token MINUSSIGN
%token PLUSSIGN
%token TIMESSIGN
%token DEVIDESIGN
%token LBRACKET
%token RBRACKET
%token PART_TOKEN
%token <command> COMMAND
%token <command> PEELING
%token <command> DIVISION
%token <command> BRUTE_FORCE
%token SCRIPT_TOKEN
%token COMPETITION
%token COLLABORATION
%token <dimension> DIMENSION
%token <dimension> FREQUENCY
%token <dimension> TIME
%token <dimension> SOURCE
%token <dimension> FIELD
%token <dimension> BASELINE

%start script

%type <block> script part_block parts part control_block control
%type <block> defining_part referencing_part

%type <branchId> topbranch

%type <options> param_block

%type <command> commandname

%type <subScript> definition_part

%type <param> range

%type <param> tactic_param

%type <param> tactic_params

%type <real> exp

%type <dimension> dimension

%left NEG     /* negation--unary minus */

%left MINUSSIGN
%left PLUSSIGN
%left TIMESSIGN
%left DEVIDESIGN
%left POWERSIGN
%left BRANCH
%%


script :
         SCRIPT_TOKEN { level = 0; newSiblings(); } control_block {
  //          printf("\nscript parsed:\n%s\nend of script\n",$2);
          saveScript("callibrate_observation", $3);
       }
       | topbranch {
       /*
         It doesn't matter whether this is a branch or a ref,
         the numbering should continue.
       */
          branch = $1; level = 0; newSiblings();
       }
       definition_part {
	 //          printf("\nbranch-parsed:\n%s\ndefinition-part:\n%s\nend of parsed branch\n",$1,$3);
          saveScript($1, $3);
       }
       ;

part : defining_part {$$ = $1; }
     | referencing_part { $$ = $1; }
     ;

defining_part : PART_TOKEN definition_part {
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
   //   fprintf(stderr,"defining-part-parsed:\n%s\nend of defining part",$2);
   free ($2);
}
              ;

definition_part : commandname control_block {
                   if(level == 1)
                   {
                      char * bn = strdup(calculateBrancheNumber());
		      //                      fprintf(stderr,"\ndefinition-commandname:\n%s\ncontrol block:\n%s\nend of definition part", $1, $2);
                      saveSubScript(bn, $1 , NULL , $2 );
                      $$ = bn;
                   }
                   else
                   {
                      $$ = (char*)(malloc(strlen($1) + strlen($2) + 4));
                      sprintf($$,"%s\n%s\n", $1, $2);
                      free($1);
                      free($2);
                   }
                }
                | commandname param_block control_block {
                   if(level == 1) /* replace and save the nested part */
                   {
                      char * bn = strdup(calculateBrancheNumber());
		      //                      fprintf(stderr,"\ndefinition-commandname:\n%s\nparameters:\n%s\ncontrol:\n%s\nend of definition part\n", $1, $2, $3);
                      saveSubScript(bn, $1 , $2 , $3 );
                      $$ = bn;
                   }
                   else /* just copy it */
                   {
                      $$ = (char*)(malloc(strlen($1) + strlen($2) + strlen($3) + 5));
                      sprintf($$,"%s %s\n%s\n", $1, $2, $3);
                      free($1);
                      free($2);
                      free($3);
                   }
                }
;

referencing_part : PART_TOKEN BRANCH {
  //   printf("referencing part parsed: %s", $2);
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
}
                 | PART_TOKEN REF {
  //   printf("referencing part parsed: %s", $2);
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
}
                 | PART_TOKEN REFBRANCH {
		   //   printf("referencing part parsed: %s", $2);
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
}
                 ;

param_block : LBRACKET dimension tactic_params RBRACKET {
   $$ = (char*)(malloc(strlen($2)+strlen($3)+5));
   sprintf($$,"[%s %s]",$2,$3);
   //   fprintf(stderr,"param-block:\n%s\nend of parameter block\n",$$);
   free($3);
}
             ;

tactic_params : tactic_param { $$ = $1; }
              | tactic_param COMMA tactic_params {
                 $$ = (char*)(malloc(strlen($1)+strlen($3)+2));
                 sprintf($$,"%s,%s",$1,$3);
                 free($1);
                 free($3);
}
              ;

tactic_param : exp   {
                 int seiz=20;
                 $$ = (char*)(malloc(seiz));
                 seiz = snprintf($$,seiz,"%f",$1);
                        /* <todo>check if it fits</todo> */
                 if(seiz >= 20)
                 {
                    report("double didn't fit in slot.");
                 }
}
             | range { $$ = $1; }
             | error { $$ = (char*)(malloc(2)); strcpy($$,"\0");}
             ;

range : exp RANGESIGN exp {
 /*<todo>strcat it into this string</todo>*/
   $$ = (char*)(malloc(44));
   sprintf($$,"%g:%g",$1,$3);
}
      ;

exp : NUMBER               { $$ = $1;         }
        | REAL                { $$ = $1;         }
    | exp PLUSSIGN exp                 { $$ = $1 + $3;    }
    | exp MINUSSIGN exp                 { $$ = $1 - $3;    }
    | exp TIMESSIGN exp                 { $$ = $1 * $3;    }
    | exp DEVIDESIGN exp                 { $$ = $1 / $3;    }
    | MINUSSIGN exp  %prec NEG          { $$ = -$2;        }
    | exp POWERSIGN exp                 { $$ = pow ($1, $3); }
    | '(' exp ')'                 { $$ = $2;         }

part_block : LBRACE parts RBRACE {
   $$ = (char*)(malloc(strlen($2) + 4));
   sprintf($$,"{%s}\n",$2);
   free($2);
}
           ;

control_block : LBRACE control RBRACE {
   $$ = (char *)(malloc(strlen($2) + 4));
   sprintf($$,"{%s}\n",$2);
   free($2);
}
              | LBRACE RBRACE { $$ = (char *)(malloc(4)); sprintf($$,"{}\n"); }
              ;

control : COMPETITION { level ++; /* step in deeper */} part_block  {
   $$ = (char *)(malloc(strlen($3) + 3 + 11));/*strlen("competition")*/
   sprintf($$,"competition %s\n",$3);
   free($3);
   level--; /* step back */
}
        | COLLABORATION { level ++; } part_block {
   $$ = (char*)(malloc(strlen($3) + 3 + 13));/*strlen("collaboration")*/
   sprintf($$,"collaboration %s\n",$3);
   free($3);
   level--;
}
        ;

parts : part { $$ = $1; }
      | part parts {
   $$ = (char *)(malloc(strlen($1) + strlen($2) + 2));
   sprintf($$,"%s\n%s",$1,$2);
   free($1);
   free($2);
}
      ;


commandname : COMMAND { $$ = $1; }
            ;

dimension : DIMENSION { $$ = $1; }
          ;

topbranch: BRANCH { $$ = $1;}
         | REF { $$ = $1;}
         | REFBRANCH { $$ = $1;}
         ;
%%
