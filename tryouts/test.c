#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Function to compute the greatest common divisor (GCD) of two numbers
int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

// Function to compute the modular inverse using the extended Euclidean algorithm
int mod_inverse(int a, int m) {
    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return x;
        }
    }
    return -1;
}

// Function to perform modular exponentiation (base^exp % mod)
int mod_exp(int base, int exp, int mod) {
    int result = 1;
    base = base % mod;

    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }

        exp = exp >> 1;
        base = (base * base) % mod;
    }

    return result;
}

// Function to encrypt a message using public key
int encrypt(int plaintext, int publicKey, int modulus) {
    return mod_exp(plaintext, publicKey, modulus);
}

// Function to decrypt a ciphertext using private key
int decrypt(int ciphertext, int privateKey, int modulus) {
    return mod_exp(ciphertext, privateKey, modulus);
}

int main() {
    // Key generation (simplified for demonstration)
    int p = 61; // Prime number
    int q = 53; // Prime number
    int modulus = p * q;
    int phi = (p - 1) * (q - 1);
    int publicKey = 17; // Public exponent
    int privateKey = mod_inverse(publicKey, phi);

    // Original message
    int plaintext = 88;

    // Encrypt the message using public key
    int encrypted = encrypt(plaintext, publicKey, modulus);

    // Decrypt the encrypted message using private key
    int decrypted = decrypt(encrypted, privateKey, modulus);

    // Display results
    printf("Original plaintext: %d\n", plaintext);
    printf("Encrypted ciphertext: %d\n", encrypted);
    printf("Decrypted plaintext: %d\n", decrypted);

    return 0;
}