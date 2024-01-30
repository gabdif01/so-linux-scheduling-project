#include "fake_process.h"
#include "linked_list.h"
#pragma once
#define INIT_PRED_BURST 5
#define ALFA 0.5
#define CPU_NUMBER 2



// aggiunta predicted burst 

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  double pred_burst;
  double last_pred_burst;
  int arrival_time;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

// aggiunta last_pred_burst q(t)
// trasformazione variabile running da campo singola a lista per consentire la gestione multipla di CPU
typedef struct FakeOS{
  ListHead runnings;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;

  ListHead processes;
  double last_pred_burst;
} FakeOS;


// adjust structure to load alfa (alfa) and first predicted quantum (firstPredBurst) args
// the first predicted quantum will be used to calculate the first CPU burst prediction at T0 time

typedef struct {
  int firstPredBurst;
  double alfa;
} SchedSJFArgs;



void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
// metodo ausiliario che quando necessario aggiorna la pred_burst del processo che sta per essere inserito in catena di ready
void FakeOS_updPredBurst(FakeOS* os, FakePCB* pcb, ProcessEvent* e);
