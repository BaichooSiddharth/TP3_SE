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

struct Frame {
    unsigned int page;
    int count;
    bool dirty;
};
struct Frame frames[NUM_FRAMES];
/*
struct node {
    unsigned int frame;
    unsigned int page;
    struct node *next;
};

struct buffer {
    struct node *first;
    struct node *last;
    int length;
};

static void buffer_add_first( struct buffer *t, int frameNumber , int pageNumber ) {
    struct node *new_node;
    new_node->next = t->first;
    new_node->frame = frameNumber;
    new_node->page = pageNumber;

    t->first = new_node;
    t->length++;

}

void buffer_add_last( struct buffer *t, int frameNumber , int pageNumber ){
    struct node *new_node;
    new_node->next=NULL;
    new_node->frame = frameNumber;
    new_node->page = pageNumber;

    t->last->next = new_node;
    t->last = new_node;
    t->length++;
}

int buffer_lookup( struct buffer *t, int frameNumber ){

    struct node *current = t->first;

    if( current->frame == frameNumber ){

        return current->page;
    }

    else {
        for (int i = 1; i < t->length ; i++) {

            if ( current->next->frame == frameNumber ) {
                struct node *temp = t->first;

                t->first = current->next;
                current->next = current->next->next;
                t->first->next = temp;

                return t->first->page;
            }
        }

        return -1;
    }
}

struct buffer buffer_init ( int length ){
    struct buffer new;

    for( int i = 0 ; i < length ; i++ ){
        buffer_add_first(&new,NULL,NULL);
    }

    return new;
}
*/
static int findFrameNumber(unsigned int page) {
    int frameNumber = tlb_lookup(page, 0);
    if (frameNumber < 0) {
        frameNumber = pt_lookup(page);
        if (frameNumber < 0) {
            // Page fault, finding the least used frame as the new host for the requested page

            int leastUsedFrame = 0;
            for (int i = 0; i < NUM_FRAMES; i++) {
                if (frames[i].count == 0) {
                    leastUsedFrame = i;
                    break;
                } else if (frames[i].count < frames[leastUsedFrame].count) leastUsedFrame = i;
            }
            frameNumber = leastUsedFrame;

            // If usage is detected, store page data in the backing store
            if (frames[frameNumber].dirty) {
                pm_backup_page(leastUsedFrame, frames[frameNumber].page);
                pt_unset_entry(frames[frameNumber].page);
                frames[frameNumber].count = 0;
            }

            pm_download_page(page, frameNumber);
            tlb_add_entry(page, frameNumber, 0);
            pt_set_entry(page, frameNumber);
            frames[frameNumber].page = page;
        }
    }
    frames[frameNumber].count++;

    return frameNumber;
}

/* Effectue une lecture à l'adresse logique `laddress`.  */
char vmm_read (unsigned int laddress)
{
  char c;
  read_count++;
  /* ¡ TODO: COMPLÉTER ! */
    unsigned int page = laddress / 256;
    unsigned int offset = laddress % 256;

    int frameNumber = findFrameNumber(page);

    unsigned int physAddress = frameNumber * PAGE_FRAME_SIZE + offset;
    c = pm_read(physAddress);

  vmm_log_command (stdout, "READING", laddress, page, frameNumber, offset, physAddress, c);
  return c;
}

/* Effectue une écriture à l'adresse logique `laddress`.  */
void vmm_write (unsigned int laddress, char c)
{
  write_count++;

    unsigned int page = laddress / 256;
    unsigned int offset = laddress % 256;

    int frameNumber = findFrameNumber(page);
    frames[frameNumber].dirty = true;

    unsigned int physAddress = frameNumber * PAGE_FRAME_SIZE + offset;
    pm_write(physAddress, c);

  vmm_log_command (stdout, "WRITING", laddress, page, frameNumber, offset, physAddress, c);
}


// NE PAS MODIFIER CETTE FONCTION
void vmm_clean (void)
{
  fprintf (stdout, "VM reads : %4u\n", read_count);
  fprintf (stdout, "VM writes: %4u\n", write_count);
}
