
%token	<string_val> WORD

%token 	NOTOKEN GREAT APPEND NEWLINE PIPE AND LESS CD



%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
} 
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: complex_command
        ;

simple_command:
	command_and_args NEWLINE {
		printf("   Yacc: Execute command \n");
			Command::_currentCommand.execute();
	}
	|command_and_args iomodifier_list NEWLINE {
		printf("   Yacc: Execute command \n");
			Command::_currentCommand.execute();
	}
	|command_and_args Pipe_list NEWLINE {
		printf("   Yacc: Execute command \n");
			Command::_currentCommand.execute();
		
	}
	|command_and_args Pipe_list iomodifier_list NEWLINE {
		printf("   Yacc: Execute command \n");
			Command::_currentCommand.execute();
	}
	|AND_command NEWLINE {
		printf("   Yacc: Execute command \n");
		Command::_currentCommand.
		insertSimpleCommand( Command::_currentSimpleCommand );
			Command::_currentCommand.execute();
	}
	|command_word iomodifier_list NEWLINE {
		printf("   Yacc: Execute command \n");
			Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;
	
complex_command:
	simple_command
	|complex_command simple_command
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;
	
command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	|arg_list argument
	| /* can be empty */
	;


argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);
	       Command::_currentSimpleCommand->insertArgument($1);\
	}
	;



command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
	
	
	
iomodifier_list:
	iomodifier_opt
	| iomodifier_list iomodifier_opt
	;
	

iomodifier_opt:
	APPEND WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
	 	Command::_currentCommand._appendFile = $2;
	 	Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	| GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2); 
		Command::_currentCommand._inputFile = $2;
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
		}
		;

Pipe_list:
	Pipe_command arg_list 
	|Pipe_list Pipe_command arg_list 
	; 
	
	
Pipe_command:
	PIPE WORD{
		printf("   Yacc: insert command \"%s\"\n", $2);
		Command::_currentSimpleCommand = new SimpleCommand();
	        Command::_currentSimpleCommand->insertArgument($2);
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;	
	
AND_command:
	WORD AND{
		printf("   Yacc: insert command \"%s\"\n", $1);
		Command::_currentSimpleCommand = new SimpleCommand();
	        Command::_currentSimpleCommand->insertArgument($1);
	        Command::_currentCommand._background=1;
	}


	
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
