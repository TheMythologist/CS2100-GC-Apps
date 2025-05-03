#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  9
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

void binaryToFloatRepresentation(const char *binaryStr, char *integerPart, char *fractionalPart) {
    char sign = binaryStr[0] == '0' ? '+' : '-';
    char exponentArray[8];
    char mantissaArray[23];

    // Fill exponentArray with the exponent bits
    for (int i = 0; i < 8; i++) {
        exponentArray[i] = binaryStr[i + 1];
    }

    // Fill mantissaArray with the mantissa bits
    for (int i = 0; i < 23; i++) {
        mantissaArray[i] = binaryStr[i + 9];
    }

    // Calculate exponent value directly from binary string
    int exponentValue = -127;
    for (int i = 0; i < 8; i++) {
        if (exponentArray[i] == '1') {
            exponentValue += (1 << (7 - i));
        }
    }

    // Calculate mantissa value directly from binary string
    float mantissaValue = 1.0f; // Start with the implicit leading 1
    float fractionBase = 0.5f;
    for (int i = 0; i < 23; i++) {
        if (mantissaArray[i] == '1') {
            mantissaValue += fractionBase;
        }
        fractionBase /= 2.0f;
    }

    // Calculate the float value using base 10 arithmetic
    float floatValue = mantissaValue;
    if (exponentValue > 0) {
        for (int i = 0; i < exponentValue; i++) {
            floatValue *= 2.0f;
        }
    } else {
        for (int i = 0; i < -exponentValue; i++) {
            floatValue /= 2.0f;
        }
    }

    // Apply the sign
    if (sign == '-') {
        floatValue = -floatValue;
    }

    // Separate the float value into integer and fractional parts
    int intPart = (int)floatValue;
    float fracPart = floatValue - intPart;

    // Convert integer part to string
    snprintf(integerPart, 12, "%d", intPart);

    // Convert fractional part to string
    int fracIndex = 0;
    for (int i = 0; i < 6; i++) { // Limit to 6 decimal places
        fracPart *= 10;
        int digit = (int)fracPart;
        fractionalPart[fracIndex++] = '0' + digit;
        fracPart -= digit;
    }
    fractionalPart[fracIndex] = '\0';
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
    char integerPart[12], fractionalPart[12];
    binaryToFloatRepresentation(binaryStr, integerPart, fractionalPart);

    /* Print the hex input */
    os_PutStrFull("Hex: ");
    os_PutStrFull(paddedHex);
    os_NewLine();

    /* Print the binary representation */
    printBinary(binaryStr);

    /* Print the sign, exponent, and mantissa */
    unsigned int sign = binaryStr[0] - '0';

    unsigned int exponent = 0;
    char exponentBinary[9];
    for (int i = 1; i <= 8; i++) {
        exponent = (exponent << 1) | (binaryStr[i] - '0');
        exponentBinary[i - 1] = binaryStr[i];
    }
    exponentBinary[8] = '\0';

    unsigned int mantissa = 0;
    char mantissaBinary[24];
    for (int i = 9; i < 32; i++) {
        mantissa = (mantissa << 1) | (binaryStr[i] - '0');
        mantissaBinary[i - 9] = binaryStr[i];
    }
    mantissaBinary[23] = '\0';

    snprintf(response, RESP_SIZE, "Sign: %u", sign);
    os_PutStrFull(response);
    os_NewLine();

    snprintf(response, RESP_SIZE, "E: %u (%s)", exponent, exponentBinary);
    os_PutStrFull(response);
    os_NewLine();
    // Convert mantissa binary to mantissa decimal
    double mantissaDecimal = 1.0; // Start with the implicit leading 1
    double fractionValue = 0.5; // The value of the first fractional bit (2^-1)

    for (int i = 0; i < 23; i++) {
        if (mantissaBinary[i] == '1') {
            mantissaDecimal += fractionValue;
        }
        fractionValue /= 2.0; // Move to the next fractional bit (2^-2, 2^-3, etc.)
    }

    snprintf(response, RESP_SIZE, "M: %.10f", mantissaDecimal);
    os_PutStrFull(response);
    os_NewLine();

    /* Clear the homescreen and display the formatted output */
    snprintf(response, RESP_SIZE, "Float: %s.%s", integerPart, fractionalPart);
    os_PutStrFull(response);
    os_NewLine();
    /* Calculate and print the float in scientific notation */
    double floatValue = mantissaDecimal;
    int exponentValue = exponent - 127;
    if (exponentValue > 0) {
        for (int i = 0; i < exponentValue; i++) {
            floatValue *= 2;
        }
    } else {
        for (int i = 0; i < -exponentValue; i++) {
            floatValue /= 2;
        }
    }
    // Manually format the float value in scientific notation
    char scientificNotation[RESP_SIZE];
    int sciExponent = 0;

    // Normalize the float value to the range [1, 10)
    if (floatValue != 0.0) {
        while (floatValue >= 10.0) {
            floatValue /= 10.0;
            sciExponent++;
        }
        while (floatValue < 1.0) {
            floatValue *= 10.0;
            sciExponent--;
        }
    }

    // Format the float value and exponent into scientific notation
    snprintf(scientificNotation, RESP_SIZE, "S: %.10fE%d", floatValue, sciExponent);
    os_PutStrFull(scientificNotation);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}
