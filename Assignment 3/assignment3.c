#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOGICAL_ADDRESS_SPACE_SIZE 65536
#define PHYSICAL_ADDRESS_SPACE_SIZE 32768
#define PAGE_SIZE 256
#define NUM_PAGES (LOGICAL_ADDRESS_SPACE_SIZE / PAGE_SIZE)
#define NUM_FRAMES (PHYSICAL_ADDRESS_SPACE_SIZE / PAGE_SIZE)
#define TLB_SIZE 16

#define ADDRESSES_FILE "addresses.txt"
#define BACKING_STORE_FILE "BACKING_STORE.bin"

typedef struct {
    int page_number;
    int frame_number;
    int valid;
} TLBEntry;

static signed char physical_memory[PHYSICAL_ADDRESS_SPACE_SIZE];
static int page_table[NUM_PAGES];
static int frame_to_page[NUM_FRAMES];
static TLBEntry tlb[TLB_SIZE];

static int tlb_next_to_replace = 0;
static int next_frame_to_replace = 0;
static int total_frames_loaded = 0;

int search_TLB(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_number == page_number) {
            return tlb[i].frame_number;
        }
    }
    return -1;
}

void TLB_Add(int page_number, int frame_number) {
    tlb[tlb_next_to_replace].page_number = page_number;
    tlb[tlb_next_to_replace].frame_number = frame_number;
    tlb[tlb_next_to_replace].valid = 1;

    tlb_next_to_replace = (tlb_next_to_replace + 1) % TLB_SIZE;
}

int TLB_Update(int old_page, int new_page, int new_frame) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_number == old_page) {
            tlb[i].page_number = new_page;
            tlb[i].frame_number = new_frame;
            tlb[i].valid = 1;
            return 1;
        }
    }
    return 0;
}

int main(void) {
    FILE *address_file = fopen(ADDRESSES_FILE, "r");
    if (address_file == NULL) {
        perror("Error opening addresses file");
        return EXIT_FAILURE;
    }

    int backing_store_fd = open(BACKING_STORE_FILE, O_RDONLY);
    if (backing_store_fd == -1) {
        perror("Error opening backing store");
        fclose(address_file);
        return EXIT_FAILURE;
    }

    unsigned char *backing_store = mmap(NULL,
                                        LOGICAL_ADDRESS_SPACE_SIZE,
                                        PROT_READ,
                                        MAP_PRIVATE,
                                        backing_store_fd,
                                        0);
    if (backing_store == MAP_FAILED) {
        perror("Error mapping backing store");
        close(backing_store_fd);
        fclose(address_file);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i] = -1;
    }

    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_to_page[i] = -1;
    }

    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
        tlb[i].valid = 0;
    }

    int logical_address;
    int total_addresses = 0;
    int page_faults = 0;
    int tlb_hits = 0;

    while (fscanf(address_file, "%d", &logical_address) == 1) {
        total_addresses++;

        int page_number = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        int frame_number = search_TLB(page_number);

        if (frame_number != -1) {
            tlb_hits++;
        } else {
            frame_number = page_table[page_number];

            if (frame_number == -1) {
                page_faults++;

                if (total_frames_loaded < NUM_FRAMES) {
                    frame_number = total_frames_loaded;
                    total_frames_loaded++;
                } else {
                    frame_number = next_frame_to_replace;
                    int old_page = frame_to_page[frame_number];

                    if (old_page != -1) {
                        page_table[old_page] = -1;
                    }

                    next_frame_to_replace = (next_frame_to_replace + 1) % NUM_FRAMES;
                }

                memcpy(physical_memory + (frame_number * PAGE_SIZE),
                       backing_store + (page_number * PAGE_SIZE),
                       PAGE_SIZE);

                if (frame_to_page[frame_number] != -1) {
                    int old_page = frame_to_page[frame_number];
                    if (!TLB_Update(old_page, page_number, frame_number)) {
                        TLB_Add(page_number, frame_number);
                    }
                } else {
                    TLB_Add(page_number, frame_number);
                }

                frame_to_page[frame_number] = page_number;
                page_table[page_number] = frame_number;
            } else {
                TLB_Add(page_number, frame_number);
            }
        }

        int physical_address = (frame_number * PAGE_SIZE) + offset;
        signed char value = physical_memory[physical_address];

        printf("Virtual address: %d Physical address = %d Value=%d\n",
               logical_address,
               physical_address,
               value);
    }

    printf("\nNumber of translated addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", (double) page_faults / total_addresses);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (double) tlb_hits / total_addresses);

    munmap(backing_store, LOGICAL_ADDRESS_SPACE_SIZE);
    close(backing_store_fd);
    fclose(address_file);

    return EXIT_SUCCESS;
}
