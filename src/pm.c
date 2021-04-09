#include <stdio.h>
#include <string.h>


#include "conf.h"
#include "pm.h"

static FILE *pm_backing_store;
static FILE *pm_log;
static unsigned int download_count = 0;
static unsigned int backup_count = 0;
static unsigned int read_count = 0;
static unsigned int write_count = 0;

// Initialise la mémoire physique
void pm_init (FILE *backing_store, FILE *log)
{
  pm_backing_store = backing_store;
  pm_log = log;
  memset (pm_memory, '\0', sizeof (pm_memory));
}

// Charge la page demandée du backing store
void pm_download_page (unsigned int page_number, unsigned int frame_number)
{
  download_count++;
  unsigned int count = 0;
  fpos_t pos;
  fgetpos(pm_backing_store, &pos);
  int char_read;
  unsigned int start_pm = frame_number * PAGE_FRAME_SIZE;
  unsigned int start_bs = page_number * PAGE_FRAME_SIZE;
  unsigned int end_bs = (page_number+1) * PAGE_FRAME_SIZE;
  unsigned int size_backing_store = NUM_PAGES * PAGE_FRAME_SIZE;
  while(count < size_backing_store && count < end_bs){
      char_read = fgetc(pm_backing_store);
      if(count >= start_bs){
          pm_memory[start_pm + (count - start_bs)] = (char) char_read;
      }
      count++;
  }
  fsetpos(pm_backing_store, &pos);
}

// Sauvegarde la frame spécifiée dans la page du backing store
void pm_backup_page (unsigned int frame_number, unsigned int page_number)
{
  backup_count++;
  /*Portion du code pour faire un log
  //on fait un tableau de char pour sauvegarder le frame number et le page number
  int size_fr = log10(frame_number);
  int size_pg = ;
  char fr_n[2];
  char pg_n[3];
  char log_line[7];
   */
  char frame[PAGE_FRAME_SIZE];
  fpos_t pos;
  fgetpos(pm_backing_store, &pos);
  unsigned int start_bs = page_number * PAGE_FRAME_SIZE;
  unsigned int start_pm = frame_number * PAGE_FRAME_SIZE;
  for(int i=0; i<PAGE_FRAME_SIZE; i++){
      frame[i] = pm_memory[start_pm+i];
  }
  fseek(pm_backing_store, start_bs, SEEK_SET);
  fwrite(frame, PAGE_FRAME_SIZE, 1,pm_backing_store);
  fsetpos(pm_backing_store, &pos);
  /* ¡ TODO: COMPLÉTER ! */
}

char pm_read (unsigned int physical_address)
{
  read_count++;
  return pm_memory[physical_address];
}

void pm_write (unsigned int physical_address, char c)
{
  write_count++;
  /* ¡ TODO: COMPLÉTER ! */
  pm_memory[physical_address] = c;
}


void pm_clean (void)
{
  // Enregistre l'état de la mémoire physique.
  if (pm_log)
    {
      for (unsigned int i = 0; i < PHYSICAL_MEMORY_SIZE; i++)
	{
	  if (i % 80 == 0)
	    fprintf (pm_log, "%c\n", pm_memory[i]);
	  else
	    fprintf (pm_log, "%c", pm_memory[i]);
	}
    }
  fprintf (stdout, "Page downloads: %2u\n", download_count);
  fprintf (stdout, "Page backups  : %2u\n", backup_count);
  fprintf (stdout, "PM reads : %4u\n", read_count);
  fprintf (stdout, "PM writes: %4u\n", write_count);
}
