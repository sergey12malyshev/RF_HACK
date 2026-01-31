#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#include "sheduler.h"
#include "main.h"
#include "iwdg.h"
#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "workStates.h"
#include "encoderDriver.h"
#include "configFile.h"

#include "application_Thread.h"
#include "button_Thread.h"
#include "subGHz_RX_Thread.h"
#include "subGHz_TX_Thread.h"
#include "spectrumScan_Thread.h"
#include "jammer_Thread.h"

#include "button_Thread.h"
#include "gps_Thread.h"

#include "runBootloader.h"
#include "buzzer_driver.h"


#include "cli_driver.h"
#include "cli_thread.h"

static struct pt application_pt, cli_pt, rf_pt, sub_tx_pt, button_pt, specrum_pt, jammer_pt, gps_pt;

static void initProtothreads(void)
{
  PT_INIT(&application_pt);
  PT_INIT(&cli_pt);
  PT_INIT(&rf_pt);
  PT_INIT(&sub_tx_pt);
  PT_INIT(&button_pt);
  PT_INIT(&specrum_pt);
  PT_INIT(&jammer_pt);
  PT_INIT(&gps_pt);
}

static Work_state_t determine_work_mode(void) 
{
  if (getTxButtonState() || TX_MODE_ALWAYS) 
  {
    return TX_MODE;
  } 
  else if (getjammButtonState()) 
  {
    return JAMMER_MODE;
  } 
  else if (getScanButtonState()) 
  {
    return SCAN_MODE;
  } 
  else if (getGpsButtonState()) 
  {
    return GPS_MODE;
  }
  return RX_MODE;
}

void setting_the_operating_mode(Work_state_t new_mode)
{
  static Work_state_t current_mode = NUMBER_STATE;
    
  if (current_mode == new_mode) 
  {
    return;
  }
  else
  {
    current_mode = new_mode;
  }

  switch (new_mode)
  {
    case TX_MODE:
    {
      PT_INIT(&sub_tx_pt);
      setWorkSate(TX_MODE);
      debugPrintf("TX Mode"CLI_NEW_LINE);
      break;
    }
    case RX_MODE:
    {
      PT_INIT(&rf_pt);
      setWorkSate(RX_MODE);
      debugPrintf("RX Mode"CLI_NEW_LINE);
      break;
    }
    case SCAN_MODE:
    {
      PT_INIT(&specrum_pt);
      setWorkSate(SCAN_MODE);
      debugPrintf("SCAN Mode"CLI_NEW_LINE);
      break;
    }
    case JAMMER_MODE:
    {
      PT_INIT(&jammer_pt);
      setWorkSate(JAMMER_MODE);
      debugPrintf("JAMMER Mode"CLI_NEW_LINE);
      break;
    }
    case GPS_MODE:
    {
      PT_INIT(&gps_pt);
      setWorkSate(GPS_MODE);
      debugPrintf("GPS Mode"CLI_NEW_LINE);
      break;
    }
    default:
      assert_param(0U);
    break;
  }
}

static void execute_current_mode_thread(void)
{
  switch (getWorkState())
  {
    case TX_MODE:
      subGHz_TX_Thread(&sub_tx_pt);
      break;
    case RX_MODE:
      subGHz_RX_Thread(&rf_pt);
      break;
    case SCAN_MODE:
      spectrumScan_Thread(&specrum_pt);
      break;
    case JAMMER_MODE:
      jammer_Thread(&jammer_pt);
      break;
    case GPS_MODE:
      gps_Thread(&gps_pt);
      break;
    default:
      assert_param(0U);
    break;
  }
}

noreturn void scheduler(void)
{
  initProtothreads();

  while (true)
  {
    IWDG_reload();

    if (getBootButtonState())
    {
      runBootloader();
    }

    Application_Thread(&application_pt);
    Button_Thread(&button_pt);
#if(CLI_ENABLE)
    CLI_Thread(&cli_pt);
#endif

    if (getBootingScreenMode() == false)
    {
      setting_the_operating_mode(determine_work_mode());
      execute_current_mode_thread();
    }
  }
}