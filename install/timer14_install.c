#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include    <sys/stat.h>

#define 	MAX_COMMAND_SIZE	1024
#define 	SUN4M			"sun4m"
#define 	SUN4U			"sun4u"
#define 	SOL2_5			"5.5"
#define 	SOL7			"5.7"

void install_sun4m ( void )
{
		struct  stat	statBuf;
		char	*metsdir,
				command[MAX_COMMAND_SIZE];

	if ( stat ( "/dev/timer14", &statBuf ) != 0 )
	{
		sprintf( command, "ln -s ../devices/obio/timer14:timer14 /dev/timer14" );
		system( command );
	}	

	system( "if [ -z \"`grep -s 'name=timer14' /etc/devlink.tab`\" ] \nthen \necho \"type=SUNW,Sun 4_75;name=timer14	\\D\" \>\> /etc/devlink.tab \nfi" );

	metsdir = getenv( "METSDIR" );
	system( "rem_drv timer14" );
	sprintf( command, "cp %s/bin/timer14m /kernel/drv/timer14", metsdir );
	system( command );
	sprintf( command, "cp %s/etc/timer14m.conf /kernel/drv/timer14.conf", metsdir );
	system( command );
	system( "add_drv -m \"* 0666 bin bin\" /kernel/drv/timer14" );
}

void install_sun4u ( void )
{
		struct  stat	statBuf;
		char	*metsdir,
				command[MAX_COMMAND_SIZE];

	if ( stat ( "/dev/timer14", &statBuf ) != 0 )
	{
		sprintf( command, "ln -s ../devices/pseudo/timer14@0:timer14 /dev/timer14" );
		system( command );
	}	

	system( "if [ -z \"`grep -s 'name=timer14' /etc/devlink.tab`\" ] \nthen \necho \"type=ddi_pseudo;name=timer14	\\D\" \>\> /etc/devlink.tab \nfi" );
	
	metsdir = getenv( "METSDIR" );
	system( "rem_drv timer14" );
	sprintf( command, "cp %s/bin/timer14u /kernel/drv/timer14", metsdir );
	system( command );
	sprintf( command, "cp %s/etc/timer14u.conf /kernel/drv/timer14.conf", metsdir );
	system( command );
	system( "add_drv -m \"* 0666 bin bin\" /kernel/drv/timer14" );
}

void install_sun4v7 ( void )
{
		struct  stat	statBuf;
		char	*metsdir,
				command[MAX_COMMAND_SIZE];

	if ( stat ( "/dev/timer14", &statBuf ) != 0 )
	{
		sprintf( command, "ln -s ../devices/pseudo/timer14@0:timer14 /dev/timer14" );
		system( command );
	}	

	system( "if [ -z \"`grep -s 'name=timer14' /etc/devlink.tab`\" ] \nthen \necho \"type=ddi_pseudo;name=timer14	\\D\" \>\> /etc/devlink.tab \nfi" );
	
	metsdir = getenv( "METSDIR" );
	system( "rem_drv timer14" );
	sprintf( command, "cp %s/bin/timer14v7 /kernel/drv/timer14", metsdir );
	system( command );
	sprintf( command, "cp %s/etc/timer14v7.conf /kernel/drv/timer14.conf", metsdir );
	system( command );
	system( "add_drv -m \"* 0666 bin bin\" /kernel/drv/timer14" );
}

main( int argc,  char *argv[] )
{
		char command[MAX_COMMAND_SIZE];

	sysinfo ( SI_MACHINE, command, MAX_COMMAND_SIZE);

	if ( setuid(0) )
		printf( "ERROR[%s]: unable to set uid.\n", argv[0] );

	printf( "Installing timer14 driver for %s architecture.\n", command );
	if ( strcmp ( command, SUN4U ) == 0 )
	{
		sysinfo ( SI_RELEASE, command, MAX_COMMAND_SIZE);
		printf( "OS Release %s\n", command );
		
		if ( strncmp ( command, SOL2_5, strlen ( SOL2_5 ) ) == 0 )
			install_sun4u ();
		else if ( strncmp ( command, SOL7, strlen ( SOL7 ) ) == 0 )
			install_sun4v7 ();		
	}
	else
	{
		install_sun4m ();
	} 
}
