#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <string.h>
#include <openssl/evp.h>
#include <errno.h>

// Function to read a single character from standard input without waiting for newline
char getch() {
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

void clear_line_and_move_cursor_up() {
    printf("\033[A"); // go up
    printf("\033[2K"); // clear line
}

int main(int argc, char *argv[]) {

    FILE *fp;
    char c;
    int return_status=0;

    int suppress_output = 0; // Flag for suppressing output

    // Handle usage error and check for minimum arguments
    if (argc < 2 || argc > 3) {
      fprintf(stderr, "Usage: %s [-s] <output_file>\n", argv[0]);
      return 1;
    }

    if (argc == 3 ) {
        // Check for "-n" flag and set suppression flag if present
        if ( strcmp(argv[2], "-s") == 0 ) {
          suppress_output = 1;
          printf("Silent mode enabled.\n");
        } else { // Handle unexpected argument argv[2]
          fprintf(stderr, "Error: Unexpected argument: %s\n", argv[2]);
          return 1;
        }
    }    

    // Check if the file already exists
    if (access(argv[1], F_OK) == 0) {
        fprintf(stderr, "Error: File '%s' already exists.\n", argv[1]);
        return 1;
    }

    // Open the file for writing the mixed data
    fp = fopen(argv[1], "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        return 1;
    }

    // Open /dev/random for reading
    FILE *random_fp = fopen("/dev/random", "r");
    if (random_fp == NULL) {
        perror("Error opening /dev/random");
        fclose(fp);
        return 1;
    }

    // Initialize SHA-512 context
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("sha512");
    if (!md) {
        perror("Error initializing SHA-512");
        fclose(fp);
        fclose(random_fp);
        return 1;
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);

    printf("Press keys. Press Enter to finish.\n\n\n");

    // Loop until Enter is pressed
    while ((c = getch()) != '\n') {  

        struct timespec ts;

        // Get current time with nanosecond precision
        clock_gettime(CLOCK_REALTIME, &ts);

        if (!suppress_output) {
            // Clear the current line before printing new content
            clear_line_and_move_cursor_up();

            printf("Key: %c, Seconds: %li, Nanoseconds: %li\n", c, ts.tv_sec, ts.tv_nsec);
        }

        // Update the hash directly with key and timestamp
        EVP_DigestUpdate(mdctx, &c, sizeof(c));
        EVP_DigestUpdate(mdctx, &ts.tv_sec, sizeof(ts.tv_sec));
        EVP_DigestUpdate(mdctx, &ts.tv_nsec, sizeof(ts.tv_nsec));
    }

    // Finalize the hash
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);

    // Read the same number of bytes from /dev/random (handle potential error)
    size_t bytes_read = 0;

    // Allocate temporary buffer for random data
    unsigned char *random_data = malloc(hash_len);
    if (random_data == NULL) {
      perror("Error allocating memory for random data");
      fclose(random_fp);
      fclose(fp);
      return 1;
    }

    bytes_read = fread(random_data, 1, hash_len, random_fp);

    if (bytes_read != hash_len) {
        // Print error message but continue
        perror("Error reading from /dev/random");
        perror("WARNING! We continue without mixing with data from /dev/random! This is dangerous.");
        return_status=1;
    } else {
        // XOR the hash with the random data
        for (int i = 0; i < hash_len; i++) {
            hash[i] ^= random_data[i];
        }
    }

    // Write the mixed data to the file in hexadecimal format
    for (int i = 0; i < hash_len; i++) {
        fprintf(fp, "%02x", hash[i]);
    }

    // Close the file
    fclose(fp);
    fclose(random_fp);

    fprintf(stderr, "Mixed data written to %s\n",  argv[1]);


    // Trying to clean as much as possible data from RAM
    
    EVP_cleanup();

    // Simply overwriting data with zeros is not considered a secure practice
    // for erasing sensitive information in C. But it's probably better than nothing.
    // I declared the variables as volatile to make a more likely overwriting them to work.

    // Here is how simply overwriting data with zeros looks like:

    // for (int i = 0; i < hash_len; i++) {
    //    hash[i] = 0;
    //    random_data[i] = 0;
    // }
    
    // I commented out the simple zeoing because I use explicit_bzero below and I am not sure
    // if the simple zeroing make it worse.


    // I did not found memset_s on my Linux.

    // int result = memset_s(random_data, sizeof(random_data), 0, sizeof(random_data));
    // if (result != 0) {
    //     fprintf(stderr, "Error filling random_data with zeroes: %d\n", result);
    //     return_status=1;
    // }


    // explicit_bzero // Requires #include <string.h>
    
    // Purpose: Attempts to securely erase the contents of the random_data memory region 
    //          to prevent potential sensitive data leakage.

    // Functionality: Fills the specified memory with zeros, aiming to make it harder for
    //                attackers to recover any information that was previously stored there.

    // Availability: The explicit_bzero function might not be available on all systems or
    //               compilers. Check your documentation for compatibility.

    explicit_bzero(random_data, sizeof(random_data));
    explicit_bzero(hash, sizeof(hash));
   
    free(random_data);
    if (errno != 0) {
      fprintf(stderr, "Error freeing random_data: %d\n", errno);
      return_status=1;
    }    

    return return_status;
}

