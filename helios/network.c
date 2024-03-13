#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

#include <utlist.h>

#include <modbus/modbus.h>

#include <digitalSTROM/dsuid.h>
#include <dsvdc/dsvdc.h>

#include "helios.h"


struct memory_struct {
  char *memory;
  size_t size;
};

struct data {
  char trace_ascii; /* 1 or 0 */
};


static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct memory_struct *mem = (struct memory_struct *) userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    vdc_report(LOG_ERR, "network module: not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  
  return realsize;
}

static void DebugDump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex) {
  size_t i;
  size_t c;

  unsigned int width = 0x10;

  if (nohex)
    /* without the hex output, we can fit more on screen */
    width = 0x40;

  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long) size, (long) size);

  for (i = 0; i < size; i += width) {
    fprintf(stream, "%4.4lx: ", (long) i);

    if (!nohex) {
      /* hex not disabled, show it */
      for (c = 0; c < width; c++)
        if (i + c < size)
          fprintf(stream, "%02x ", ptr[i + c]);
        else
          fputs("   ", stream);
    }

    for (c = 0; (c < width) && (i + c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */
      if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A) {
        i += (c + 2 - width);
        break;
      }
      fprintf(stream, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */
      if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A) {
        i += (c + 3 - width);
        break;
      }
    }
    fputc('\n', stream); /* newline */
  }
  fflush(stream);
}

int decodeURIComponent (char *sSource, char *sDest) {
  int nLength;
  for (nLength = 0; *sSource; nLength++) {
    if (*sSource == '%' && sSource[1] && sSource[2] && isxdigit(sSource[1]) && isxdigit(sSource[2])) {
      sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
      sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
      sDest[nLength] = 16 * sSource[1] + sSource[2];
      sSource += 3;
      continue;
    }
    sDest[nLength] = *sSource++;
  }
  sDest[nLength] = '\0';
  return nLength;
}

bool is_scene_configured(int scene) {
  char scene_str[5];
  sprintf(scene_str, "-%d-", scene);
  if (strstr(helios.kwl.configured_scenes, scene_str) != NULL) {
    return TRUE;
  } else return FALSE;
}

scene_t* get_scene_configuration(int scene) {
  scene_t* scene_data;
  int v;
  int i;
  
  scene_data = malloc(sizeof(scene_t));
  if (!scene_data) {
    return NULL;
  }
  memset(scene_data, 0, sizeof(scene_t));
  
  v = 0;
  while (1) {
    if (&helios.kwl.scenes[v] != NULL) {
      if (helios.kwl.scenes[v].dsId == scene) {
        i = 0;
        while (1) {
          if (helios.kwl.scenes[v].modbus_data[i].modbus_register != -1) {
            scene_data->modbus_data[i] = helios.kwl.scenes[v].modbus_data[i];
            
            i++;
          } else {
            while (i < MAX_MODBUS_VALUES) {
              scene_data->modbus_data[i].modbus_register = -1;
              scene_data->modbus_data[i].modbus_value = -1;
              i++;
            }        
            break;
          }
        }
        break;
      }
      
      v++;
    } else {
      break;
    }
  }
    
  return scene_data;
}


int helios_write_modbus_register(int modbus_register, int value) {
  vdc_report(LOG_NOTICE, "write modbus register %d = %d\n", modbus_register, value);
  
  modbus_t *mb;
  mb = modbus_new_rtu("/dev/ttyUSB0", 19200, 'E', 8, 1);
        
  if (mb == NULL) {
    vdc_report(LOG_ERR, "Unable to allocate libmodbus context\n");
    return -1;
  }

  modbus_set_debug(mb, TRUE);  
  modbus_set_response_timeout(mb, 0,1000000);  
  modbus_set_slave(mb, 1);  
    
  if (modbus_connect(mb) == -1) {
    vdc_report(LOG_ERR, "Modbus connection failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    return -1;
  } 
    
  vdc_report(LOG_NOTICE, "modbus connected\n");
  
  modbus_write_register(mb, modbus_register, value);
  
  modbus_close(mb);
  modbus_free(mb); 
    
  return 0;
}

int helios_get_values() { 
  bool changed_values = FALSE;
  time_t now;
    
  now = time(NULL);

  vdc_report(LOG_NOTICE, "network: reading Helios KWL values\n");

  modbus_t *mb;
  mb = modbus_new_rtu("/dev/ttyUSB0", 19200, 'E', 8, 1);
        
  if (mb == NULL) {
    vdc_report(LOG_ERR, "Unable to allocate libmodbus context\n");
    return -1;
  }

  modbus_set_debug(mb, TRUE);  
  modbus_set_response_timeout(mb, 0,1000000);  
  modbus_set_slave(mb, 1);  
    
  if (modbus_connect(mb) == -1) {
    vdc_report(LOG_ERR, "Modbus connection failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    return -1;
  } 
  
  vdc_report(LOG_NOTICE, "modbus connected\n");  
  uint16_t dest[1];
  
  modbus_read_registers(mb,4369,1,dest);  

  sensor_value_t* svalue;
  svalue = find_sensor_value_by_name("internalTemperature");
  if (svalue == NULL) {
    vdc_report(LOG_WARNING, "value internalTemperature is not configured for evaluation - ignoring\n");
  } else {
    vdc_report(LOG_WARNING, "network: get sensor value internalTemperature: %d\n", dest[0]);        
    if ((svalue->last_reported == 0) || (svalue->last_value != dest[0])) {
      changed_values = TRUE;
    } 
    svalue->last_value = svalue->value;
    svalue->value = dest[0];
    svalue->last_query = now;
  }
  
  modbus_read_registers(mb,4370,1,dest);  
   
  svalue = find_sensor_value_by_name("internalHumidity");
  if (svalue == NULL) {
    vdc_report(LOG_WARNING, "value internalHumidity is not configured for evaluation - ignoring\n");
  } else {
    vdc_report(LOG_WARNING, "network: get sensor value internalHumidity: %d\n", dest[0]);        
    if ((svalue->last_reported == 0) || (svalue->last_value != dest[0])) {
      changed_values = TRUE;
    } 
    svalue->last_value = svalue->value;
    svalue->value = dest[0];
    svalue->last_query = now;
  }
  
  modbus_close(mb);
  modbus_free(mb); 
  
  if (changed_values ) {
    return 0;
  } else return 1;
}



