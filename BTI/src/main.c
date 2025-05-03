#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  20
#define RESP_SIZE   100

void hexToBinary(const char *hexStr, char *binaryStr) {
    int index = 0;
    for (int i = 0; i < 8; i++) {
        char hexDigit = hexStr[i];
        unsigned int value;
        if (hexDigit >= '0' && hexDigit <= '9') {
            value = hexDigit - '0';
        } else if (hexDigit >= 'a' && hexDigit <= 'f') {
            value = hexDigit - 'a' + 10;
        } else if (hexDigit >= 'A' && hexDigit <= 'F') {
            value = hexDigit - 'A' + 10;
        } else {
            value = 0;
        }
        for (int j = 3; j >= 0; j--) {
            binaryStr[index++] = (value & (1 << j)) ? '1' : '0';
        }
    }
    binaryStr[index] = '\0';
}

int binaryStringToInt(const char *binaryStr) {
    int value = 0;
    while (*binaryStr) {
        value = (value << 1) + (*binaryStr++ - '0');
    }
    return value;
}

void binaryToMIPSInstruction(const char *binaryStr, char *instruction, char *exactInstruction) {
    char opcode[7], rs[6], rt[6], rd[6], shamt[6], funct[7], immediate[17], address[27];
    strncpy(opcode, binaryStr, 6);
    opcode[6] = '\0';

    if (strcmp(opcode, "000000") == 0) { // R-type instruction
        strncpy(rs, binaryStr + 6, 5);
        rs[5] = '\0';
        strncpy(rt, binaryStr + 11, 5);
        rt[5] = '\0';
        strncpy(rd, binaryStr + 16, 5);
        rd[5] = '\0';
        strncpy(shamt, binaryStr + 21, 5);
        shamt[5] = '\0';
        strncpy(funct, binaryStr + 26, 6);
        funct[6] = '\0';
        snprintf(instruction, RESP_SIZE, "R-type: opcode=%s, rs=0b%s=%d, rt=0b%s=%d, rd=0b%s=%d, shamt=0b%s=%d, funct=0b%s", opcode, rs, binaryStringToInt(rs), rt, binaryStringToInt(rt), rd, binaryStringToInt(rd), shamt, binaryStringToInt(shamt), funct);

        // Hardcoded mappings for R-type functions
        if (strcmp(funct, "100000") == 0) {
            strcpy(exactInstruction, "add");
        } else if (strcmp(funct, "100010") == 0) {
            strcpy(exactInstruction, "sub");
        } else if (strcmp(funct, "100100") == 0) {
            strcpy(exactInstruction, "and");
        } else if (strcmp(funct, "100101") == 0) {
            strcpy(exactInstruction, "or");
        } else if (strcmp(funct, "101010") == 0) {
            strcpy(exactInstruction, "slt");
        } else if (strcmp(funct, "000000") == 0) {
            strcpy(exactInstruction, "sll");
        } else {
            strcpy(exactInstruction, "unknown R-type");
        }
    } else if (strcmp(opcode, "000010") == 0) { // J-type instruction
        strncpy(address, binaryStr + 6, 26);
        address[26] = '\0';
        snprintf(instruction, RESP_SIZE, "J-type: opcode=%s, address=%s", opcode, address);
        strcpy(exactInstruction, "j");
    } else if (strcmp(opcode, "000011") == 0) {
        strncpy(address, binaryStr + 6, 26);
        address[26] = '\0';
        snprintf(instruction, RESP_SIZE, "J-type: opcode=%s, address=%s", opcode, address);
        strcpy(exactInstruction, "jal");
    } else { // I-type instruction
        strncpy(rs, binaryStr + 6, 5);
        rs[5] = '\0';
        strncpy(rt, binaryStr + 11, 5);
        rt[5] = '\0';
        strncpy(immediate, binaryStr + 16, 16);
        immediate[16] = '\0';
        snprintf(instruction, RESP_SIZE, "I-type: opcode=%s, rs=0b%s=%d, rt=0b%s=%d, immediate=0b%s", opcode, rs, binaryStringToInt(rs), rt, binaryStringToInt(rt), immediate);

        // Hardcoded mappings for I-type opcodes
        if (strcmp(opcode, "001000") == 0) {
            strcpy(exactInstruction, "addi");
        } else if (strcmp(opcode, "001100") == 0) {
            strcpy(exactInstruction, "andi");
        } else if (strcmp(opcode, "001101") == 0) {
            strcpy(exactInstruction, "ori");
        } else if (strcmp(opcode, "100011") == 0) {
            strcpy(exactInstruction, "lw");
        } else if (strcmp(opcode, "101011") == 0) {
            strcpy(exactInstruction, "sw");
        } else if (strcmp(opcode, "000100") == 0) {
            strcpy(exactInstruction, "beq");
        } else if (strcmp(opcode, "000101") == 0) {
            strcpy(exactInstruction, "bne");
        } else {
            strcpy(exactInstruction, "unknown I-type");
        }
    }
}

void printBinary(const char *binaryStr) {
    char formattedBinaryStr[40]; // 32 bits + 7 spaces + 1 null terminator
    int index = 0;
    for (int i = 0; i < 32; i++) {
        formattedBinaryStr[index++] = binaryStr[i];
        if ((i + 1) % 4 == 0 && i != 31) { // Add space after every 4 bits except the last group
            formattedBinaryStr[index++] = ' ';
        }
    }
    formattedBinaryStr[index] = '\0';
    os_PutStrFull("Binary: ");
    os_PutStrFull(formattedBinaryStr);
    os_NewLine();
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char paddedHex[9]; // 8 characters for hex + 1 null terminator
    char binaryStr[33]; // 32 bits + 1 null terminator
    char response[RESP_SIZE];

    /* Clear the homescreen */
    os_ClrHome();

    /* Ask the user to type a hex number */
    os_PutStrFull("DM @leekaixuan for bugs :)");
    os_NewLine();
    os_GetStringInput("Enter a hex: ", inputBuffer, INPUT_SIZE);
    os_ClrHome();

    // Prepend zeros if the input is less than 8 characters
    int inputLength = strlen(inputBuffer);
    if (inputLength < 8) {
        memset(paddedHex, '0', 8 - inputLength);
        strcpy(paddedHex + (8 - inputLength), inputBuffer);
    } else {
        strncpy(paddedHex, inputBuffer, 8);
    }
    paddedHex[8] = '\0';

    hexToBinary(paddedHex, binaryStr);
    char instruction[RESP_SIZE];
    char exactInstruction[RESP_SIZE];
    binaryToMIPSInstruction(binaryStr, instruction, exactInstruction);

    /* Print the hex input */
    os_PutStrFull("Hex: ");
    os_PutStrFull(paddedHex);
    os_NewLine();

    /* Print the binary representation */
    printBinary(binaryStr);

    /* Print the MIPS instruction */
    os_PutStrFull(instruction);
    os_NewLine();

    /* Print the exact instruction */
    os_PutStrFull("I: ");
    /* Print additional information based on instruction type */
    const char *registerNames[] = {
        "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
    };

    if (strncmp(binaryStr, "000000", 6) == 0) { // R-type
        char rs[6], rt[6], rd[6], shamt[6];
        strncpy(rs, binaryStr + 6, 5);
        rs[5] = '\0';
        strncpy(rt, binaryStr + 11, 5);
        rt[5] = '\0';
        strncpy(rd, binaryStr + 16, 5);
        rd[5] = '\0';
        strncpy(shamt, binaryStr + 21, 5);
        shamt[5] = '\0';
        snprintf(response, RESP_SIZE, "%s %s, %s, %s", exactInstruction, registerNames[binaryStringToInt(rd)], registerNames[binaryStringToInt(rs)], registerNames[binaryStringToInt(rt)]);
        os_PutStrFull(response);
        os_NewLine();
    } else if (strncmp(binaryStr, "000010", 6) == 0 || strncmp(binaryStr, "000011", 6) == 0) { // J-type
        char address[27];
        strncpy(address, binaryStr + 6, 26);
        address[26] = '\0';
        snprintf(response, RESP_SIZE, "%s 0x%X", exactInstruction, binaryStringToInt(address));
        os_PutStrFull(response);
        os_NewLine();
    } else { // I-type
        char rs[6], rt[6], immediate[17];
        strncpy(rs, binaryStr + 6, 5);
        rs[5] = '\0';
        strncpy(rt, binaryStr + 11, 5);
        rt[5] = '\0';
        strncpy(immediate, binaryStr + 16, 16);
        immediate[16] = '\0';

        if (strncmp(binaryStr, "001000", 6) == 0 || strncmp(binaryStr, "001100", 6) == 0 || 
            strncmp(binaryStr, "001101", 6) == 0 || strncmp(binaryStr, "001110", 6) == 0 || 
            strncmp(binaryStr, "001010", 6) == 0) { // addi, andi, ori, xori, slti
            snprintf(response, RESP_SIZE, "%s %s, %s, %d", exactInstruction, registerNames[binaryStringToInt(rt)], registerNames[binaryStringToInt(rs)], binaryStringToInt(immediate));
        } else if (strncmp(binaryStr, "000100", 6) == 0 || strncmp(binaryStr, "000101", 6) == 0) { // beq, bne
            snprintf(response, RESP_SIZE, "%s %s, %s, %d", exactInstruction, registerNames[binaryStringToInt(rs)], registerNames[binaryStringToInt(rt)], binaryStringToInt(immediate));
        } else if (strncmp(binaryStr, "001111", 6) == 0) { // lui
            snprintf(response, RESP_SIZE, "%s %s, %d", exactInstruction, registerNames[binaryStringToInt(rt)], binaryStringToInt(immediate));
        } else { // lw, sw
            snprintf(response, RESP_SIZE, "%s %s, %d(%s)", exactInstruction, registerNames[binaryStringToInt(rt)], binaryStringToInt(immediate), registerNames[binaryStringToInt(rs)]);
        }

        os_PutStrFull(response);
        os_NewLine();
    }

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}
