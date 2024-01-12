#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;



/*void schedRR(FakeOS* os, void* args_){
  SchedRRArgs* args=(SchedRRArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;

  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  os->running=pcb;
  
  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
};*/

void schedSJF(FakeOS* os, void* args_){

  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  //look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

    FakePCB* pcb_min = (FakePCB*) os->ready.first;
    double min_burst = pcb_min->pred_burst;

    ListItem* aux=os->ready.first->next;

    while(aux){

      FakePCB* pcb_cur=(FakePCB*)aux;
      aux=aux->next;
      if (pcb_cur->pred_burst<min_burst){

        min_burst = pcb_cur->pred_burst;
        pcb_min = pcb_cur;

      }

    }

  //Decisione di schedulazione. Se ho posto metto in running direttamente il processo selezionato,
  //altrimenti controllo se il burst time pred è minore della duration del processo in running

  if (!os->running) // Se ho spazio lo carico
    os->running = pcb_min;

    //se non ho spazio verifico se il processo candidato è nuovo
    //if yes posso verificare se ci sono le condizioni di preemption altrimenti non faccio nulla

  else if (pcb_min->arrival_time == os->timer){

      //verifico se sono soddisfatte le condizioni di preemption

      FakePCB* running=os->running;
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      if (pcb_min->pred_burst < e->duration) {

        // Eseguo la preemption
        // Tolgo dalla catena di ready il processo candidato
        List_detach(&os->ready, (ListItem*) pcb_min);
        // Imposto il processo il running
        os->running = pcb_min;

        // Aggiorno la pred_burst del processo pre relazionato

        double p_pred_burst = args->alfa * e->duration + args->alfa * os->last_pred_burst;
        running->pred_burst = p_pred_burst;

        // Inserisco l'attuale processo in running in catena di ready poichè avendo fatto preemption lui non ha terminato

        List_pushBack(&os->ready, (ListItem*) running); 

      }
  }


}

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  srr_args.firstPredBurst=5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
