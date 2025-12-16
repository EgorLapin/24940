#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

// Global variables for storing line information
LineInfo *line_table = NULL;
int total_lines = 0;
int table_capacity = 0;

// Global variables for memory mapping
char *mapped_data = NULL;  // Pointer to mapped file data
size_t file_size = 0;      // Size of the mapped file

int expand_table() {
    int new_capacity = table_capacity == 0 ? 10 : table_capacity * 2;
    LineInfo *new_table = realloc(line_table, new_capacity * sizeof(LineInfo));
    if (new_table == NULL) {
        perror("realloc");
        return -1;
    }
    line_table = new_table;
    table_capacity = new_capacity;
    return 0;
}

int add_line(long offset, int length) {
    if (total_lines >= table_capacity) {
        if (expand_table() == -1) {
            return -1;
        }
    }
    
    line_table[total_lines].offset = offset;
    line_table[total_lines].length = length;
    total_lines++;
    return 0;
}

int build_line_table() {
    long current_offset = 0;
    int line_start = 0;
    int line_length = 0;
    
    printf("Building line table...\n");
    
    // Process the mapped file data
    for (size_t i = 0; i < file_size; i++) {
        if (mapped_data[i] == '\n') {
            // Found newline character
            if (add_line(line_start, line_length) == -1) {
                return -1;
            }
            line_start = current_offset + 1;
            line_length = 0;
        } else {
            line_length++;
        }
        current_offset++;
    }
    
    // Handle the last line if the file doesn't end with \n
    if (line_length > 0) {
        if (add_line(line_start, line_length) == -1) {
            return -1;
        }
    }
    
    printf("Total lines in file: %d\n", total_lines);
    return 0;
}

int read_line(int line_number, char **buffer, int *buffer_size) {
    if (line_number < 0 || line_number >= total_lines) {
        return -1;
    }
    
    // Get line information from the table
    long line_offset = line_table[line_number].offset;
    int line_length = line_table[line_number].length;
    
    // Allocate memory for the line
    *buffer_size = line_length + 1;
    *buffer = malloc(*buffer_size);
    if (*buffer == NULL) {
        perror("malloc");
        return -1;
    }
    
    // Copy the line from mapped memory
    memcpy(*buffer, mapped_data + line_offset, line_length);
    (*buffer)[line_length] = '\0';
    
    return line_length;
}

void print_debug_table() {
    printf("\n=== DEBUG TABLE ===\n");
    printf("Line Number | Offset | Length\n");
    printf("------------|--------|-------\n");
    
    for (int i = 0; i < total_lines; i++) {
        printf("%11d | %6ld | %4d\n", 
               i + 1, line_table[i].offset, line_table[i].length);
    }
    printf("========================\n\n");
}

void print_entire_file() {
    if (mapped_data == NULL) {
        printf("Error: Mapped data not available\n");
        return;
    }
    
    // Print the entire mapped file content
    printf("%.*s", (int)file_size, mapped_data);
}

// Signal handler for timeout
void timeout_handler(int sig) {
    if (sig == SIGALRM) {
        printf("\nTimeout! 5 seconds elapsed. Printing entire file content...\n");
        print_entire_file();
        printf("Program terminated due to timeout.\n");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    
    char *filename = argv[1];
    int fd;
    struct stat file_stat;
    
    // Set up signal handler for timeout
    signal(SIGALRM, timeout_handler);
    
    // Open the file
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    // Get file size
    if (fstat(fd, &file_stat) == -1) {
        perror("fstat");
        close(fd);
        exit(1);
    }
    file_size = file_stat.st_size;
    
    // Map the file into memory
    mapped_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }
    
    printf("File '%s' opened and mapped successfully (size: %zu bytes)\n", filename, file_size);
    
    // Build the line table
    if (build_line_table() == -1) {
        munmap(mapped_data, file_size);
        close(fd);
        exit(1);
    }
    
    // Print debug table
    print_debug_table();
    
    // Main program loop
    int line_number;
    char *line_buffer = NULL;
    int buffer_size = 0;
    
    printf("Program ready. You have 5 seconds to enter line number (0 to exit):\n");

    // Set alarm for 5 seconds
    alarm(5);
    
    while (1) {
        printf("> ");
        
        if (scanf("%d", &line_number) != 1) {
            // Cancel alarm since we got input (even if invalid)
            alarm(0);
            printf("Input error. Enter a number.\n");
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        // Cancel alarm since we got valid input
        alarm(0);
        
        if (line_number == 0) {
            printf("Program termination.\n");
            break;
        }
        
        if (line_number > total_lines) {
            printf("Error: line number must be from 1 to %d\n", total_lines);
            continue;
        }
        
        // Read line (numbering starts from 1, but array from 0)
        int bytes_read = read_line(line_number - 1, &line_buffer, &buffer_size);
        if (bytes_read == -1) {
            printf("Error reading line %d\n", line_number);
            continue;
        }
        
        printf("Line %d: \"%s\"\n", line_number, line_buffer);
        
        // Free memory
        free(line_buffer);
        line_buffer = NULL;
    }
    
    // Free table memory and unmap file
    free(line_table);
    munmap(mapped_data, file_size);
    close(fd);
    return 0;
}
