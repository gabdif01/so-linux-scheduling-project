#include "fake_process.h"
#include "linked_list.h"
#pragma once

//aggiunta predicted burst

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  double pred_burst;
  double rem_burst;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

//aggiunta last_pred_burst q(t)

typedef struct FakeOS{
  FakePCB* running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;

  ListHead processes;
  double last_pred_burst;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
