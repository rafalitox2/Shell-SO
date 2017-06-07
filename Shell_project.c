/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/
#define ROJO "\x1b[31;1;1m"
#define NEGRO "\x1b[0m"
#define VERDE "\x1b[32;1;1m"
#define AZUL "\x1b[34;1;1m"
#define CIAN "\x1b[36;1;1m"
#define MARRON "\x1b[33;1;1m"
#define PURPURA "\x1b[35;1;1m"

#include "job_control.h"   // remember to compile with module job_control.c 
#include <string.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

//--------Entradita chula-----
void print_prompt(){
			char directorio[1000];
			char usuario[1000];
				getlogin_r(usuario,sizeof(usuario));
				getcwd(directorio,sizeof(directorio));
			printf("%s%s@my-terminal:%s",VERDE,usuario,NEGRO);
			printf("%s%s%s $ ",CIAN,directorio,NEGRO);
			fflush(stdout);
		}
// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	int respawnable;
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait, pid_waitMan, pid_Resp; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res,status_res_2; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */

	job *my_job_list;
	job *maneja2,*fgitem,*bgitem;

	 void manejador (int child){
			int contador;
			for(contador=1;contador<=list_size(my_job_list);contador++){
				maneja2=get_item_bypos(my_job_list,contador);
				if(!maneja2 -> state == FOREGROUND){
				pid_waitMan = waitpid(maneja2 -> pgid,&status,WNOHANG|WUNTRACED);
				if(pid_waitMan==maneja2 -> pgid){
				status_res=analyze_status(status,&info);
					if (status_res==SUSPENDED){
						printf("El proceso se ha suspendido\n");
						maneja2 -> state = STOPPED;
						printf("El estate es %s \n",state_strings[maneja2 -> state]);
					}else {
							printf("%sEl proceso ha terminado por el envío de una señal de finalización, si es RESPAWNABLE, se reiniciará\n %s",VERDE,NEGRO);
						
							if(maneja2->state==RESPAWNABLE){
								pid_Resp= fork();
								if(pid_Resp<0){
									printf("Error del copón %s\n",args[0]);
								}
								if(pid_Resp ==0){
									new_process_group(getpid());
									restore_terminal_signals();
								int no_funciona;
									no_funciona=execvp(maneja2->args[0],maneja2->args);			//solved, se puede llamar con ->args[0];
									printf("Error, command not found %d\n", no_funciona);
									exit(-1);
								}else{
								maneja2->pgid=pid_Resp;
								}
							}
							else{
								delete_job(my_job_list,maneja2);
								contador--;
						}
					}
				}
				
			}
			
		}
			
			
}
			
		my_job_list = new_list("Lista tareas");
	
	signal(SIGCHLD,manejador);
	signal(SIGTSTP,manejador);

	ignore_terminal_signals();

	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{   		
		print_prompt();
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		
		if(args[0]==NULL)    continue;   // if empty command
		respawnable=0;
				if (strcmp(args[0],"cd")==0){
					if(args[1]==NULL){
						printf("Debes añadir otro argumento a cd, por ejemplo, cd Desktop\n");
					}else{
						if(chdir(args[1])==0){
							printf("Directorio accedido con exito\n");
						}else{
							printf("No se ha podido acceder al directorio %s\n",args[1]);
						}
					}
					continue;
				}
				if(strcmp(args[0], "jobs")==0){
					if(empty_list(my_job_list)){
						printf("No se encuentra ningún proceso en ejecución\n"); 
					}else{
						print_job_list(my_job_list);
					}
					continue;
				}
				
				if(strcmp(args[0], "fg")==0){
					if(empty_list(my_job_list)){
						printf("No se encuentra ningún proceso en ejecución\n"); 
					}else{
						if(args[1] == NULL){
							fgitem = get_item_bypos(my_job_list, 1);
						}else{
							fgitem = get_item_bypos(my_job_list, atoi(args[1]));	
						}
						if (fgitem !=NULL){
							set_terminal(fgitem->pgid);
							fgitem -> state = FOREGROUND;  //Aquí yació FOREGORUND
							killpg(fgitem->pgid, SIGCONT); 
							pid_wait = waitpid(fgitem->pgid, &status, WUNTRACED);
							//hacer la misma comprobación que en el padre, si ha terminado se elimina de la lista, sino, se cambia el estado como en el padre
							set_terminal(getpid());
							status_res_2 = analyze_status(status, &info);
								
								printf("Foreground pid: %d, command: %s, %s\n", pid_wait, fgitem->command, status_strings[status_res_2]);
								
								if (status_res_2==SUSPENDED){
									printf("El proceso se ha suspendido\n");
									fgitem -> state = STOPPED;
								}else{
									delete_job(my_job_list,fgitem);
								}
						//que hacemos con las señales? solved,las señales ya se las dimos en la creación del hijo.
						}else{
							printf("No se encuentra ningún proceso con el dato dado\n"); 
						}
					}
					continue;
				}
				
				if(strcmp(args[0], "bg")==0){
					if(empty_list(my_job_list)){
						printf("No se encuentra ningún proceso en ejecución\n"); 
					}else{
						if(args[1] == NULL){ //se utiliza bg NULL?
							bgitem = get_item_bypos(my_job_list, 1);
						}else{
							bgitem = get_item_bypos(my_job_list, atoi(args[1]));	
						}
						//añadir comprobación NULL, como en fg
					
					if (bgitem -> state==STOPPED|| bgitem -> state == RESPAWNABLE){
						bgitem -> state = BACKGROUND;
						}
						killpg(bgitem->pgid, SIGCONT);
						printf("Background job runing... pid: %d, command: %s\n", bgitem->pgid, bgitem->command);
						//que hacemos con las señales?, solved, igual que arriba el proceso está creado.
					}
					
					continue;
				}
				
				for(int i=0;i<256;i++){
					if(args[i]==NULL){
						break;
					}else if(!strcmp(args[i],"#")){
						args[i]=NULL;
						respawnable=1;
					}
				}
				fflush(NULL);
				pid_fork=fork();
				
				
				
				
				if(pid_fork<0){
					printf("Error del copón %s\n",args[0]);
				}
				else if(pid_fork==0){
					new_process_group(getpid());
					
					if((background==0) && (respawnable ==0)){
						set_terminal(getpid());
					}
					
					restore_terminal_signals();
				int no_funciona;
					no_funciona=execvp(args[0],args);
					printf("Error, command not found %d\n", no_funciona);
					exit(-1);
				}
				
				new_process_group(pid_fork);
				
				//preguntar si hace falta en el padre los add_job SOLVED. Hacen falta si está en SUSPENDED o BACKGROUND.
				
				
				
				
					
				if((background==0) && (respawnable ==0)){
					set_terminal(pid_fork);
					int pidw = waitpid(pid_fork,&status,WUNTRACED);
					set_terminal(getpid());
					status_res = analyze_status(status,&info);
					printf("El estado es %s \n",status_strings[status_res]);
					
					if (status_res==SUSPENDED){
						add_job(my_job_list, new_job(pid_fork, args[0], STOPPED));
					}	
				}else{
					if(respawnable==1){
						add_job_respawnable(my_job_list, new_job(pid_fork,args[0],RESPAWNABLE),args);
						printf("%sRespawnable job running... pid %d, command: %s%s \n ",ROJO, pid_fork,args[0],NEGRO);
					}else{
						add_job(my_job_list, new_job(pid_fork, args[0], BACKGROUND));
						printf("%sBackground job running... pid %d, command: %s%s \n ",PURPURA, pid_fork,args[0],NEGRO);
						fflush(NULL);
					}
				}
		/* the steps are:
			 (1) fork a child process using fork()
			 (2) the child process will invoke execvp()
			 (3) if background == 0, the parent will wait, otherwise continue 
			 (4) Shell shows a status message for processed command 
			 (5) loop returns to get_commnad() function
		*/

	} // end while
}
