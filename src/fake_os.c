#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

void FakeOS_init(FakeOS* os) {
  List_init(&os->runnings);
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
  os->last_pred_burst=-1; // inizializzazione nuovo campo della struttura last_pred_burst
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  
  ListItem* aux=os->runnings.first;
  while(aux){
  	FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }
 
  aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->events=p->events;
  new_pcb->arrival_time=p->arrival_time;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
   
  switch(e->type){
  	
	  case CPU:
	  	// Calcolo il pred_burst el processo corrente ed aggiorno la last prediction del sistema operativo al tempo 'timer'
	  	FakeOS_updPredBurst(os,new_pcb,e);
	    List_pushBack(&os->ready, (ListItem*) new_pcb);
	    break;
	  case IO:
	    List_pushBack(&os->waiting, (ListItem*) new_pcb);
	    break;
	  default:
	    assert(0 && "illegal resource");
	    ;
  }
}


void FakeOS_simStep(FakeOS* os){
  
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started
  //and create all processes starting now
  ListItem* aux=os->processes.first;
  int p_new = 0; // variabile segnalino(flag) valorizzata ad 1 quando acquisisco un nuovo processo
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){
      new_process=proc;
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
      FakeOS_createProcess(os, new_process);

      free(new_process);
      p_new = 1;
    }
  }

  // scan waiting list, and put in ready all items whose event terminates
  aux=os->waiting.first;
  while(aux) {
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO);
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);
      free(e);
      List_detach(&os->waiting, (ListItem*)pcb);
      if (! pcb->events.first) {
        // kill process
        printf("\t\tend process\n");
        free(pcb);
      } else {
        //handle next event
        e=(ProcessEvent*) pcb->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          FakeOS_updPredBurst(os,pcb,e);
          List_pushBack(&os->ready, (ListItem*) pcb);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb);
          break;
        }
      }
    }
  }

  

  // decrement the duration of running
  // if event over, destroy event
  // and reschedule process
  // if last event, destroy running
  
  // scan running list, consume duration and put in ready or waiting any process that terminates
  
  aux=os->runnings.first;
  while (aux){
  	
  	FakePCB* running=(FakePCB*)aux;
    aux=aux->next;
    
    printf("\trunning pid: %d\n", running?running->pid:-1);
	  if (running) {
	    ProcessEvent* e=(ProcessEvent*) running->events.first;
	    assert(e->type==CPU);
	    e->duration--;
	    printf("\t\tremaining time:%d\n",e->duration);
	    if (e->duration==0){
	      printf("\t\tend burst\n");
	      List_popFront(&running->events);
	      free(e);
	      if (! running->events.first) {
	        printf("\t\tend process\n");
	        running= (FakePCB*)List_detach(&os->runnings, (ListItem*)running);
	        free(running); // kill process
	      } else {
	        e=(ProcessEvent*) running->events.first;
	        switch (e->type){
	        case CPU:
	          printf("\t\tmove to ready\n");
	          running=(FakePCB*)List_detach(&os->runnings, (ListItem*)running);
	          FakeOS_updPredBurst(os,running,e);
	          List_pushBack(&os->ready, (ListItem*) running);
	          break;
	        case IO:
	          printf("\t\tmove to waiting\n");
	          running=(FakePCB*)List_detach(&os->runnings, (ListItem*)running);
	          List_pushBack(&os->waiting, (ListItem*) running);
	          break;
	        }
	      }
	    }
	  }
  }
  
  // call schedule, if defined
  if (os->schedule_fn && (os->runnings.size < CPU_NUMBER || p_new )){
    (*os->schedule_fn)(os, os->schedule_args); 
  }

  // if running not defined and ready queue not empty
  // put the first in ready to run
  if (!os->schedule_fn && os->runnings.size<CPU_NUMBER && os->ready.first) {
    FakePCB* aux =(FakePCB*) List_popFront(&os->ready);
    List_pushBack(&os->runnings, (ListItem*) aux); 
  }

  ++os->timer;

}

void FakeOS_updPredBurst(FakeOS* os, FakePCB* pcb, ProcessEvent* e){
	
  
  SchedSJFArgs* os_args = (SchedSJFArgs*) os->schedule_args;
  
  double p_last_pred_burst = -1; 
  double p_pred_burst = -1; 
  
    // recupero la last_pred_burst. Se � la prima predizione che faccio os->last_pred_burst vale -1 quindi utilizzo la predizione di default configurata
  	// altrimenti utilizzo la ultima predizione calcolata dal sistema operativo
  p_last_pred_burst = os->last_pred_burst<0 ? os_args->firstPredBurst : os->last_pred_burst;
  printf("\tpid:%d, p_last_pred_burst %f\n", pcb->pid, p_last_pred_burst);
  p_pred_burst = os_args->alfa * e->duration + (1-os_args->alfa) * p_last_pred_burst;
  printf("\tpid:%d, p_pred_burst %f\n", pcb->pid, p_pred_burst);
  pcb->pred_burst = p_pred_burst; // aggiornamento della predizione del processo
  os->last_pred_burst = p_pred_burst; // aggiornamento ultima predizione del SO
  
  
}

void FakeOS_destroy(FakeOS* os) {
}
