#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  20
#define RESP_SIZE   100

void show_bits(unsigned u, int num_bits, char *bitStr) {
    for (int bit = 0; bit < num_bits; bit++) {
        bitStr[bit] = (u & (1 << (num_bits - bit - 1))) ? '1' : '0';
    }
    bitStr[num_bits] = '\0';
}

void floatToBinary(int integerPart, int decimalPart, int leadingZeros, char *binaryStr) {
    int sign = 0, exponent = 0, mantissa = 0;
    
    char floatStr[RESP_SIZE];
    if (leadingZeros > 0) {
        snprintf(floatStr, RESP_SIZE, "Float: %d.%0*d%d", integerPart, leadingZeros, 0, decimalPart);
    } else {
        snprintf(floatStr, RESP_SIZE, "Float: %d.%d", integerPart, decimalPart);
    }
    os_PutStrFull(floatStr);
    os_PutStrFull(" ");
    // char partsStr[RESP_SIZE];
    // snprintf(partsStr, RESP_SIZE, "(%d,%d,%d)", integerPart, decimalPart, leadingZeros);
    // os_PutStrFull(partsStr);
    // os_NewLine();
    // Determine the sign bit: 1 for negative, 0 for positive
    sign = (integerPart < 0) ? 1 : 0;
    os_PutStrFull(sign ? "Sign: 1 (-)" : "Sign: 0 (+)");
    os_NewLine();

// 123.123
// 123 246 492 984 968 936 872 744 488 976 952 904 808 616 232 464 928 856 712 424
//     0   0   0   1   1   1   1   1   0   1   1   1   1   1   0   0   1   1   1
// mantissa = 1.0001 001
// 59 
// 118-64 = 54    (1)
// 108-64 = 44    (1)
// 88-64 = 24     (1)
// 56             (0)
// 112-64 = 48    (1)
// 96-64 = 32     (1)
// 64-64 = 0      (0)
// exponent = 4
    bool negativeExp = false;
    if (integerPart == 0) {
        negativeExp = true;
    }
    // Normalize the integer part to the range [1, 2)
    exponent = 0;
    mantissa = 0;
    int numShiftsLeftover = 23;
    // Find the largest power of 2 that is still less than integerPart
    int largestPowerOf2 = 1;
    while ((largestPowerOf2 << 1) <= integerPart) {
        largestPowerOf2 <<= 1;
        exponent++;
        // os_PutStrFull("x");
    }
    // 85 - 64 = 21                  1
    // 21*2 = 42                     0
    // 42*2 = 84, 84 - 64 = 20       1
    // 20*2 = 40                     0
    // 40*2 = 80, 80 - 64 = 16       1
    // 16*2 = 32                     0
    // 32*2 = 64, 64 - 64 = 0        1
    integerPart -= largestPowerOf2; // removing the 1.0 in 1.something
    while (integerPart > 0 && numShiftsLeftover > 0) {
        integerPart <<= 1;
        mantissa <<= 1;
        if (integerPart >= largestPowerOf2) {
            mantissa |= 1;
            integerPart -= largestPowerOf2;
        }
        numShiftsLeftover--;
    }
    // Print the mantissa in binaryString
    // char temp[24];
    // show_bits(mantissa, 23, temp);
    // os_PutStrFull(temp);
    // os_NewLine();

    int decimalValue = decimalPart;
    int decimalShift = (int)log10(decimalPart) + 1 + leadingZeros;
    int powerOfTen = 1;
    for (int j = 0; j < decimalShift; j++) {
        powerOfTen *= 10;
    }
    if (negativeExp) {
        // 125 
        while (decimalValue <= powerOfTen) {
            decimalValue <<= 1;
            exponent--;
        }
        decimalValue -= powerOfTen;
    }
    while(numShiftsLeftover > 0) {
        decimalValue *= 2;
        mantissa <<= 1;
        if (decimalValue >= powerOfTen) {
            mantissa |= 1;
            decimalValue -= powerOfTen;
        }
        numShiftsLeftover--;
    }
    // Display the exponent without bias
    char exponentNoBiasStr[10];
    sprintf(exponentNoBiasStr, "%d", exponent);
    os_PutStrFull("E:");
    os_PutStrFull(exponentNoBiasStr);
    os_PutStrFull("+127=");
    // Adjust the exponent for the IEEE 754 bias
    exponent += 127;
    char exponentStr[10];
    sprintf(exponentStr, "%u", exponent);
    os_PutStrFull(exponentStr);
    os_PutStrFull("(");

    // Convert and display the exponent with bias in binary
    char exponentBinary[9];
    show_bits(exponent, 8, exponentBinary);
    os_PutStrFull(exponentBinary);
    os_PutStrFull(")");
    os_NewLine();

    os_PutStrFull("M:1.");

    // Convert and display the mantissa in binary
    char mantissaBinary[24];
    show_bits(mantissa, 23, mantissaBinary);
    os_PutStrFull(mantissaBinary);
    os_PutStrFull("(");

    // Convert mantissaBinary to decimal
    float mantissaDecimal = 1.0;
    float fraction = 0.5;
    for (int i = 0; i < 23; i++) {
        if (mantissaBinary[i] == '1') {
            mantissaDecimal += fraction;
        }
        fraction /= 2;
    }

    // Print the mantissa in decimal
    char mantissaDecimalStr[10];
    sprintf(mantissaDecimalStr, "%.7f", mantissaDecimal);
    os_PutStrFull(mantissaDecimalStr);
    os_PutStrFull(")");
    os_NewLine();

    // Convert the sign, exponent, and mantissa to a binary string
    binaryStr[0] = sign ? '1' : '0';
    for (int i = 0; i < 8; i++) {
        binaryStr[1 + i] = exponentBinary[i];
    }
    for (int i = 0; i < 23; i++) {
        binaryStr[9 + i] = mantissaBinary[i];
    }
    binaryStr[32] = '\0';
}

void binaryToHex(const char *binaryStr, char *hexStr) {
    for (int i = 0; i < 8; i++) {
        unsigned int hexDigit = 0;
        for (int j = 0; j < 4; j++) {
            if (binaryStr[i * 4 + j] == '1') {
                hexDigit |= (1 << (3 - j));
            }
        }
        if (hexDigit < 10) {
            hexStr[i] = '0' + hexDigit;
        } else {
            hexStr[i] = 'a' + (hexDigit - 10);
        }
    }
    hexStr[8] = '\0';
}

void stringToFloatParts(const char *str, int *integerPart, int *decimalPart, int *leadingZeros) {
    *integerPart = 0;
    *decimalPart = 0;
    *leadingZeros = 0;
    int sign = 1;
    int i = 0;

    // Handle sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convert integer part
    while (str[i] != '\0' && (str[i] >= '0' && str[i] <= '9')) {
        *integerPart = *integerPart * 10 + (str[i] - '0');
        i++;
    }

    // Convert decimal part if present
    if (str[i] == '.') {
        i++;
        int decimalDivisor = 1;
        while (str[i] != '\0' && (str[i] >= '0' && str[i] <= '9')) {
            if (*decimalPart == 0 && str[i] == '0') {
                (*leadingZeros)++;
            } else {
                *decimalPart = *decimalPart * 10 + (str[i] - '0');
            }
            decimalDivisor *= 10;
            i++;
        }
    }

    // Apply sign
    *integerPart *= sign;
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char binaryStr[33];
    char hexStr[9];
    char response[RESP_SIZE];
    int integerPart = 0, decimalPart = 0, leadingZeros = 0;

    /* Clear the homescreen */
    os_ClrHome();

    /* Ask the user to type a float number */
    os_PutStrFull("DM @leekaixuan for bugs :)");
    os_NewLine();
    os_GetStringInput("Enter a float: ", inputBuffer, INPUT_SIZE);
    os_ClrHome();
    stringToFloatParts(inputBuffer, &integerPart, &decimalPart, &leadingZeros);
    floatToBinary(integerPart, decimalPart, leadingZeros, binaryStr);
    binaryToHex(binaryStr, hexStr);

    /* Clear the homescreen and display the formatted output */
    
    /* Display each piece of information on its own line */
    
    os_PutStrFull("B:");
    for (int j = 0; binaryStr[j] != '\0'; j += 4) {
        os_PutStrFull((char[]){binaryStr[j], binaryStr[j+1], binaryStr[j+2], binaryStr[j+3], ' ', '\0'});
    }
    os_NewLine();
    
    os_PutStrFull("H:");
    os_PutStrFull(hexStr);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}
