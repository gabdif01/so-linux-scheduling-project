#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;


void schedSJF(FakeOS* os, void* args_){


 
 
 // Aggiornamento predizioni per i processi in catena di ready, in base alla last prediction aggiornata.
 // Questo mi consente di calcolare attendibilmente il processo tra i nuovi e i vecchi che ha la stima minore per schedulare.
 
 /*ListItem* aux=os->ready.first;
 
 while (aux){
 	
 	FakePCB* pcb=(FakePCB*)aux;
 	aux=aux->next;
 	ProcessEvent* e=(ProcessEvent*) pcb->events.first;
	assert(e->type==CPU);
 	
 	FakeOS_updPredBurst(os,pcb,e);
 	
 }*/
 
  SchedSJFArgs* args=(SchedSJFArgs*)args_;
  
  while (os->runnings.size<args->cpu_num){
  	
  	  // look for the first process in ready
  		// if none, return
 	if (!os->ready.first) 
    	return;
  	
  	// Calcolo il processo candidato 
  	
  	FakePCB* pcb_min = (FakePCB*) os->ready.first;
  	double min_burst = pcb_min->pred_burst;
  
  	ListItem* aux=os->ready.first->next;
  
  	while(aux) {
    
	  FakePCB* pcb_cur=(FakePCB*)aux;
  	  aux=aux->next;
  	  if (pcb_cur->pred_burst<min_burst){
  	  	
  	  	min_burst = pcb_cur->pred_burst;
  	  	pcb_min = pcb_cur;
  	  	
		}
  	  	
  	}
  	
  	// Tolgo dalla catena di ready il processo candidato
	pcb_min=(FakePCB*)List_detach(&os->ready, (ListItem*)pcb_min);
	// Aggiungo il processo candidato alla lista dei processi in running
  	List_pushBack(&os->runnings, (ListItem*) pcb_min);
  	
 }

// controllo se ho ancora processi in ready 

	if (!os->ready.first)
    	return;	
    
    int f_exit = 0; // flag che mi indica se continuare il ciclo (calcolo min + verifica preemption)
    
    /* Da impostare la condizione di uscita */
    
    while (!f_exit) {
    	
    	// Calcolo del minimo solo sui processi nuovi. Considero solo i nuovi perchè in questo step sto analizzando la possibilità di preemption.
		
	    FakePCB* pcb_min = 0;
		double min_burst = -1;
	    ListItem* aux=os->ready.first;
	    // Inizio calcolo pcb_min sui processi nuovi
	    while (aux){
	    	FakePCB* pcb=(FakePCB*) aux;
			aux = aux->next;
	    	if (pcb->arrival_time == os->timer){
	    		
	    		if ((!pcb_min && min_burst < 0) || (pcb->pred_burst<min_burst)){
	    			
	    			pcb_min = pcb;
	    			min_burst = pcb->pred_burst;
	    			
				}
	    		
			}
		}
		// Fine calcolo minimo
		// Ho scelto il minimo tra i processi nuovi 
		
		if (pcb_min){ 
		
			// verifico se sono soddisfatte le condizioni di preemption per ogni processo in running
			  
			  ListItem* aux = os->runnings.first;
			  int f_preemption=0;
			  while (aux){
			  	
			  	FakePCB* running=(FakePCB*) aux;
			  	aux = aux->next;
			   	ProcessEvent* e=(ProcessEvent*) running->events.first;
	    	  	assert(e->type==CPU);
	    	  
			  	if (pcb_min->pred_burst < running->pred_burst) {
			  	
				  	// Eseguo la preemption 
				  	// Tolgo dalla catena di ready il processo candidato
				  	printf("\t\texecute preemption pid:%d\n", running->pid);
				  	printf("\t\tmove to ready\n");
				  	pcb_min = (FakePCB*)List_detach(&os->ready, (ListItem*)pcb_min);
				  	running = (FakePCB*)List_detach(&os->runnings, (ListItem*)running);
				  	// Imposto il processo il running 
				  	// Aggiungo il processo candidato alla lista dei processi in running
	  				List_pushBack(&os->runnings, (ListItem*) pcb_min);
				  	
					  // Inserisco l'attuale processo in running in catena di ready poichè avendo fatto preemption lui non ha terminato
				  	
				  	List_pushBack(&os->ready, (ListItem*) running);
				  	f_preemption = 1;
				  	break;
			  	
			  	}
			  }
			  if (!f_preemption)
			  		f_exit = 1;
		}else 
			f_exit = 1; // non ho necessità di continuare, quindi setto il flag a 1 per uscire	
	}

}


int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  srr_args.firstPredBurst=INIT_PRED_BURST;
  srr_args.alfa = ALFA;
  srr_args.cpu_num = atoi(argv[1]);
  printf ("Numero di CPU scelte: %d\n", srr_args.cpu_num);
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
  for (int i=2; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]); // leggo il nome del file 
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.runnings.first
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
