#include "tx_api.h"
#include "tx_timer.h"
#include "tx_port.h"
#include "tx_execution_profile.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdint.h>

EXECUTION_TIME bsp_GetRunTime(void);
EXECUTION_TIME bsp_CheckRunTime(EXECUTION_TIME _LastTime);

