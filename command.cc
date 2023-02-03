
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_appendFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}
	
	if ( _appendFile ) {
		free( _appendFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_appendFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Append	  Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _appendFile?_appendFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::changedir()
{
	if (_simpleCommands[0]->_arguments[1] != NULL ){
		if (chdir(_simpleCommands[0]->_arguments[1]) == -1){
	   		printf("Wrong path!\n");
	   	}
	   	else{
	   		printf("%s>",_simpleCommands[0]->_arguments[1]);
			chdir(_simpleCommands[0]->_arguments[1]);
		}
	}
	else{
		printf("/home/mariem>");
		chdir("/home/mariem");
	}
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	print();
	if (strcmp(_simpleCommands[0]->_arguments[0],"cd")==0){
		changedir();
		clear();
		prompt();
	}
	else{	

	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	
	int outfd = open( _currentCommand._outFile, O_TRUNC|O_WRONLY);
	int appendfd = open( _currentCommand._appendFile, O_APPEND|O_WRONLY);
	int infd = open( _currentCommand._inputFile, O_RDONLY);
	
	int i = 0;
	int fdpipe[_numberOfSimpleCommands-1][2];
	int pid;
	for(i=0; i<_numberOfSimpleCommands; i++)
	{
		if ( pipe(fdpipe[i]) == -1) {
			perror( "Error: pipe\n");
			exit( 2 );
		}
	}
	
	for(i=0; i<_numberOfSimpleCommands; i++)
	{	
		if(i == 0){
			if(infd>0){
				dup2( infd, 0 );
				close(infd);
			}
			else{
				dup2( defaultin, 0 );
				close(defaultin);
			}
			
			dup2( defaulterr, 2 );
			dup2( fdpipe[i][1], 1 );
			close(fdpipe[i][1]);
		}
		if(i == _numberOfSimpleCommands-1){
			dup2( fdpipe[i-1][0], 0 );
			close(fdpipe[i-1][0]);
			dup2( defaulterr, 2 );
			
			if(outfd>0){
				dup2(outfd,1);
				close(outfd);
			}
			else if(appendfd > 0){
				dup2(appendfd,1);
				close(appendfd);
			}
			
			else
				dup2( defaultout ,1);
			dup2( defaulterr, 2 );
		}
		if( i != 0 && i != _numberOfSimpleCommands-1){	
			dup2( fdpipe[i-1][0], 0 );
			close(fdpipe[i-1][0]);
			dup2( defaulterr, 2 );
		
			dup2( fdpipe[i][1], 1 );
			close(fdpipe[i][1]);
			dup2( defaulterr, 2 );
		}

		int pid = fork();
		if ( pid == -1 ) {
			perror( "cat_grep: fork\n");
			exit( 2 );
		}

		if (pid == 0) {
			for (int  n=0; n<_numberOfSimpleCommands; n++)
			{
				close(fdpipe[n][0]);
				close(fdpipe[n][1]);
			}
			close( defaultin );
			close( defaultout );
			close( defaulterr );

			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror( "cat_grep: exec cat");
			//exit( 2 );
			}
			
		if(i==_numberOfSimpleCommands-1 && _background != 1){
			waitpid( pid, 0, 0 );
		}
	}

	// Restore input, output, and error
	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );
	
	close( defaultin );
	close( defaultout );
	close( defaulterr );
	
	clear();
	prompt();
}	
}

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}
void
handler(int signo){
	FILE *out_file = fopen("logfile", "a");
	time_t t = time(NULL);	
	fprintf(out_file, "Child terminated\t%s", ctime(&t));
	fclose(out_file);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	signal(SIGINT, SIG_IGN);
	signal (SIGCHLD, handler);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

