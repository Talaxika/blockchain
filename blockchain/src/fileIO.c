
#include "include/fileIO.h"

iResult write_to_file(Blockchain blockchain)
{
    // Open the file for writing
    HANDLE hFile = CreateFile(
        (LPCSTR)"Blockchain.txt",   // File name
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
    DWORD bytesWritten = 0;
    char buffer[256] = {0};

    for (uint32_t num_blk = 0; num_blk < blockchain.num_blocks; num_blk++)
    {
        block_t block = blockchain.blocks[num_blk];

        snprintf(buffer, sizeof(buffer), "Block: %d\r\n", block.index);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        snprintf(buffer, sizeof(buffer), "\tTimestamp: %llu\r\n", block.timestamp);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        snprintf(buffer, sizeof(buffer), "\tPrevious hash: %llu\r\n", block.previous_hash);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        snprintf(buffer, sizeof(buffer), "\tCurrent hash: %llu\r\n", block.current_hash);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        snprintf(buffer, sizeof(buffer), "\tNonce: %llu\r\n", block.nonce);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        snprintf(buffer, sizeof(buffer), "\tTransactions: %d\r\n", block.num_transactions);
        WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

        for (uint32_t num_trx = 0; num_trx < block.num_transactions; num_trx++)
        {
            snprintf(buffer, sizeof(buffer), "\t\t-Index: %d\r\n", block.transactions[num_trx].index);
            WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

            snprintf(buffer, sizeof(buffer), "\t\tSender_id: %d\r\n", block.transactions[num_trx].sender_id);
            WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

            snprintf(buffer, sizeof(buffer), "\t\tAmount: %d\r\n", block.transactions[num_trx].amount);
            WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);

            snprintf(buffer, sizeof(buffer), "\t\tTimestamp: %llu\r\n", block.transactions[num_trx].timestamp);
            WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL);
        }
    }

    // Close the file
    CloseHandle(hFile);

    printf("Structure written to file successfully\n");

    return RET_CODE_SUCCESS;
}