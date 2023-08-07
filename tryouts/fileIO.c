#include <stdio.h>
#include <string.h>
#include <windows.h>

// Define a sample structure
typedef struct {
    int id;
    char name[50];
    float salary;
} Employee;

int main() {
    // Create an instance of the structure
    Employee emp;
    emp.id = 1;
    strcpy(emp.name, "John Doe");
    emp.salary = 50000.0;

    // Open the file for writing
    HANDLE hFile = CreateFile(
        "employees.txt",            // File name
        GENERIC_WRITE,              // Access mode (write)
        0,                          // No sharing
        NULL,                       // Security attributes
        CREATE_ALWAYS,              // Create if not exists, overwrite if exists
        FILE_ATTRIBUTE_NORMAL,      // File attributes
        NULL                        // Template file
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        perror("Error opening file");
        return 1;
    }

    // Convert the structure fields to bytes and write to the file
    DWORD bytesWritten;
    char buffer[256];

    // ID
    snprintf(buffer, sizeof(buffer), "ID: %d\r\n", emp.id);
    WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

    // Name
    snprintf(buffer, sizeof(buffer), "Name: %s\r\n", emp.name);
    WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

    // Salary
    snprintf(buffer, sizeof(buffer), "Salary: %.2f\r\n", emp.salary);
    WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

    // Close the file
    CloseHandle(hFile);

    printf("Structure written to file successfully\n");

    return 0;
}
