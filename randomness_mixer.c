#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/evp.h>

#define KEYS_FILE "keys_and_times.txt"
#define MIXED_FILE "random512.txt"

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
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

int main() {
    FILE *fp;
    char c;
    struct timespec ts;

    // Open /dev/random for reading
    FILE *random_fp = fopen("/dev/random", "r");
    if (random_fp == NULL) {
        perror("Error opening /dev/random");
        return 1;
    }

    // Open the keys file for reading and appending
    fp = fopen(KEYS_FILE, "a");
    if (fp == NULL) {
        perror("Error opening file " KEYS_FILE);
        return 1;
    }

    printf("Press keys. Press Enter to finish.\n");

    // Loop until Enter is pressed
    while ((c = getch()) != '\n') {
        // Get current time with nanosecond precision
        clock_gettime(CLOCK_REALTIME, &ts);
        
        // Append the key and the corresponding timestamp to the file
        fprintf(fp, "Key: %c, Time: %llu ns\n", c, (long long unsigned int)(ts.tv_sec * 1000000000 + ts.tv_nsec));
        
        // Print the key and the corresponding timestamp
        printf("Key: %c, Time: %llu ns\n", c, (long long unsigned int)(ts.tv_sec * 1000000000 + ts.tv_nsec));
    }

    // Rewind the keys file to the beginning
    rewind(fp);

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
        return 1;
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);

    // Read the file line by line and update the hash
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        EVP_DigestUpdate(mdctx, line, strlen(line));
    }

    // Finalize the hash
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);

    // Close the keys file
    fclose(fp);


    unsigned char randomdata[hash_len];
    
    // Read the same number of bytes as the hash from /dev/random and XOR it with the hash
    size_t bytes_read = fread(randomdata, 1, hash_len, random_fp);
    if (bytes_read != hash_len) {
        perror("Error reading from /dev/random");
        fclose(random_fp);
        return 1;
    }
		
	for (int i = 0; i < hash_len; i++) {
	  hash[i] ^= randomdata[i]; // XOR hash[i] with randomdata[i] and store the result back in hash[i]
	}

    // Close /dev/random
    fclose(random_fp);


    // Open the file for writing the mixed data
    fp = fopen(MIXED_FILE, "w");
    if (fp == NULL) {
        perror("Error opening file " MIXED_FILE);
        return 1;
    }

    // Write the mixed data to the file in hexadecimal format
    for (int i = 0; i < hash_len; i++) {
        fprintf(fp, "%02x", hash[i]);
    }

    // Close the file
    fclose(fp);

    printf("\nWARNING! It's a proof of concept code, it writes the pressed keys in a file in the current directory!\n");
    printf("\nKeys and timestamps are written in " KEYS_FILE "\n");
    printf("Mixed data written to " MIXED_FILE "\n");

    return 0;
}

