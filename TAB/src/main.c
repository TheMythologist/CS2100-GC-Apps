#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  20
#define RESP_SIZE   100

int convertToDecimal(const char *str, int base) {
    int value = 0;
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        char c = str[i];
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            return -1; // Invalid character for the base
        }
        if (digit >= base) {
            return -1; // Invalid digit for the base
        }
        value = value * base + digit;
    }
    return value;
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char response[RESP_SIZE];

    /* Clear the homescreen */
    os_ClrHome();

    /* Ask the user to type a string */
    os_PutStrFull("DM @leekaixuan for bugs :)");
    os_NewLine();
    os_GetStringInput("Enter a string: ", inputBuffer, INPUT_SIZE);
    os_ClrHome();

    /* Try converting the input to decimal in each base from 2 to 16 */
    for (int base = 2; base <= 16; base++) {
        int decimalValue = convertToDecimal(inputBuffer, base);
        if (decimalValue != -1) {
            snprintf(response, RESP_SIZE, "%d: %d    ", base, decimalValue);
            os_PutStrFull(response);
        }
    }

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}

