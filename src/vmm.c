#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "common.h"
#include "vmm.h"
#include "tlb.h"
#include "pt.h"
#include "pm.h"

static unsigned int read_count = 0;
static unsigned int write_count = 0;
static FILE* vmm_log;

void vmm_init (FILE *log)
{
  // Initialise le fichier de journal.
  vmm_log = log;
}


// NE PAS MODIFIER CETTE FONCTION
static void vmm_log_command (FILE *out, const char *command,
                             unsigned int laddress, /* Logical address. */
		             unsigned int page,
                             unsigned int frame,
                             unsigned int offset,
                             unsigned int paddress, /* Physical address.  */
		             char c) /* Caractère lu ou écrit.  */
{
  if (out)
    fprintf (out, "%s[%c]@%05d: p=%d, o=%d, f=%d pa=%d\n", command, c, laddress,
	     page, offset, frame, paddress);
}

bool framesUsed[NUM_FRAMES];

static int findFrameNumber(unsigned int page) {
    int frameNumber = tlb_lookup(page, 0);
    if (frameNumber < 0) {
        frameNumber = pt_lookup(page);
        if (frameNumber < 0) {
            for (int i = 0; i < NUM_FRAMES; i++) {
                if (framesUsed[i] == false) {
                    frameNumber = i;
                    framesUsed[i] = true;
                    break;
                }
            }
            pm_download_page(page, frameNumber);
            tlb_add_entry(page, frameNumber, 0);
            pt_set_entry(page, frameNumber);
        }
    }

    return frameNumber;
}

/* Effectue une lecture à l'adresse logique `laddress`.  */
char vmm_read (unsigned int laddress)
{
  char c = '!';
  read_count++;
  /* ¡ TODO: COMPLÉTER ! */
    unsigned int page = laddress / 256;
    unsigned int offset = laddress % 256;

    int frameNumber = findFrameNumber(page);

    unsigned int physAddress = frameNumber * PAGE_FRAME_SIZE + offset;
    c = pm_read(physAddress);

  // TODO: Fournir les arguments manquants.
  vmm_log_command (stdout, "READING", laddress, page, frameNumber, offset, physAddress, c);
  return c;
}

/* Effectue une écriture à l'adresse logique `laddress`.  */
void vmm_write (unsigned int laddress, char c)
{
  write_count++;
  /* ¡ TODO: COMPLÉTER ! */
    unsigned int page = laddress / 256;
    unsigned int offset = laddress % 256;

    int frameNumber = findFrameNumber(page);

    unsigned int physAddress = frameNumber * PAGE_FRAME_SIZE + offset;
    pm_write(physAddress, c);

  // TODO: Fournir les arguments manquants.
  vmm_log_command (stdout, "WRITING", laddress, page, frameNumber, offset, physAddress, c);
}


// NE PAS MODIFIER CETTE FONCTION
void vmm_clean (void)
{
  fprintf (stdout, "VM reads : %4u\n", read_count);
  fprintf (stdout, "VM writes: %4u\n", write_count);
}
