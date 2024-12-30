#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:6262)
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <cstring>
#include <cassert>
#include <streambuf>
using namespace std;

const int MAX_LINE_LENGTH = 250;

int render(int lineLength, istream& inf, ostream& outf);
int isWordOverLine(int currentIndex, char line[], int length, int lineMax);
bool hasHyphen(int currentIndex, char line[]);
bool isParagraphBreak(int currentIndex, char line[]);
bool isRestParagraphBreaks(int currentIndex, char line[]);
bool noMoreWords(int currentIndex, char line[]);
void wordSegment(int index, char line[]);


//used to run test cases
void testRender(int lineLength, const char input[], const char expectedOutput[], int expectedReturnValue)
{
    istringstream iss(input);
    ostringstream oss;
    ostringstream dummy;
    streambuf* origCout = cout.rdbuf(dummy.rdbuf());
    int retval = render(lineLength, iss, oss);
    cout.rdbuf(origCout);
    if (!dummy.str().empty())
        cerr << "WROTE TO COUT INSTEAD OF THIRD PARAMETER FOR: " << input << endl;
    else if (retval != expectedReturnValue)
        cerr << "WRONG RETURN VALUE FOR: " << input << endl;
    else if (retval == 2)
    {
        if (!oss.str().empty())
            cerr << "WROTE OUTPUT WHEN LINELENGTH IS " << lineLength << endl;
    }
    else if (oss.str() != expectedOutput)
        cerr << "WRONG RESULT FOR: " << input << endl;
}

int main()
{

}




int render(int lineLength, istream& inf, ostream& outf) {
    if (lineLength < 1 || lineLength > MAX_LINE_LENGTH)
        return 2; //indicating invalid line length
    const int MAX_INPUT_LINE = 180;
    char inputReader[MAX_INPUT_LINE];
    int currentLength = 0; //initializes counter used to check if a word goes past the line length
    bool isFirstBreak = true;  //used to only print a single paragraph break when multiple @P@’s are present
    bool isLastInput = false; //tracks if dealing with last input line 
    int returnValue = 0; //returned at end of function and changed to 1 if word is too long
    int temporaryLength = 0; //used to hold spaces when checking if a word would go over the length
    int fillerParBreaks = 0; //accounts for ignored paragraph breaks at beginning of first input line
    bool noHyphen = true;
    bool firstWordOutted = false;
    while (inf.getline(inputReader, MAX_INPUT_LINE)) {
        if (inf.peek() == EOF) {
            isLastInput = true;
        }
        //initializes index past ignored paragraph breaks at the beginning of the input file
        while (isspace(inputReader[fillerParBreaks]) && inputReader[fillerParBreaks] != '\0' && !firstWordOutted) {
            fillerParBreaks++;
        }
        while (isParagraphBreak(fillerParBreaks, inputReader) && !firstWordOutted) {
            fillerParBreaks += 3;
            while (isspace(inputReader[fillerParBreaks]) && inputReader[fillerParBreaks] != '\0') {
                fillerParBreaks++;
            }
        }
        //divide each word segment with a space
        wordSegment(fillerParBreaks, inputReader);

        //increment through each line of input and output correct syntax
        for (int index = fillerParBreaks; inputReader[index] != '\0'; index++) {

            if (isLastInput && isRestParagraphBreaks(index, inputReader)) {
                break;
            }

            if (noMoreWords(index, inputReader))
                break;

            int lastCharIndex;
            if (index - 1 >= 0) {
                lastCharIndex = index - 1; //initializes previous character that will not be called later unless the value is over 0
            }
            else {
                lastCharIndex = -1;
            }

            //accounts for first word in every input line being a @P@
            while (isParagraphBreak(index, inputReader)) {
                if (isFirstBreak && !isLastInput) {
                    outf << endl << endl;
                    currentLength = 0;
                    isFirstBreak = false;
                }
                index += 3;
                //accounts for spaces after the paragraph break
                while (isspace(inputReader[index]) && inputReader[index] != '\0') {
                    index++;
                }
            }
            isFirstBreak = true;

            //accounts for first word in every input line being too long
            if (index == 0) {
                if (isWordOverLine(index, inputReader, temporaryLength, lineLength) == 1 && firstWordOutted) {
                    outf << endl;
                    currentLength = 0;
                    //accounts for cases where the input is larger than the lineLength
                    int extraLargeCounter = 0;
                    while (isWordOverLine(index, inputReader, currentLength, lineLength)) {
                        returnValue = 1;
                        extraLargeCounter++;
                        outf << inputReader[index];
                        index++;
                        currentLength++;
                        if (extraLargeCounter >= lineLength) {
                            outf << endl;
                            currentLength = 0;
                            extraLargeCounter = 0;
                        }
                    }
                }
                else if(isWordOverLine(index, inputReader, temporaryLength, lineLength) == 1 && !firstWordOutted){
                    //accounts for case where first word outputted is longer than line length while not outputting extra new line
                    returnValue = 1;
                    for (int i = 0; i < lineLength; i++) {
                        outf << inputReader[index];
                        index++;
                    }
                        outf << endl;
                        currentLength = 0;
                        //accounts for cases where the input is larger than the lineLength
                        int extraLargeCounter = 0;
                        while (isWordOverLine(index, inputReader, currentLength, lineLength)) {
                            returnValue = 1;
                            extraLargeCounter++;
                            outf << inputReader[index];
                            index++;
                            currentLength++;
                            if (extraLargeCounter >= lineLength) {
                                outf << endl;
                                currentLength = 0;
                                extraLargeCounter = 0;
                            }
                        }
                }
                else {
                    //checks if there is a punctuation mark and adds an extra space
                    if (lastCharIndex >= 0 && currentLength != 0) {
                        if (inputReader[lastCharIndex] == '.' || inputReader[lastCharIndex] == '!' || inputReader[lastCharIndex] == '?' || inputReader[lastCharIndex] == ':') {
                            outf << ' ';
                            currentLength++;
                        }
                        else if (inputReader[lastCharIndex] == '-') {
                            noHyphen = false;
                        }
                    }
                    //adds a space separate new words that do not start a new line 
                    if (currentLength != 0 && noHyphen) {
                        outf << ' ';
                        currentLength++;
                    }
                    noHyphen = true;
                }
            }

            if (isspace(inputReader[index])) {
                //lastCharIndex holds the index of the last character of the previous word
                //increments past spaces
                while (isspace(inputReader[index]) && inputReader[index] != '\0') {
                    index++;
                }

                //checks if a paragraph break is present, updating the output line length and index as needed
                while (isParagraphBreak(index, inputReader)) {
                    if (isFirstBreak && !isLastInput) {
                        outf << endl << endl;
                        currentLength = 0;
                        isFirstBreak = false;
                    }
                    index += 3;
                    //accounts for spaces after the paragraph break
                    while (isspace(inputReader[index]) && inputReader[index] != '\0') {
                        index++;
                    }
                }
                isFirstBreak = true;

                //account for spaces that have not been added in "isWordOverLine" calcuation
                temporaryLength = currentLength;
                if (lastCharIndex >= 0 && currentLength != 0) {
                    if (inputReader[lastCharIndex] == '.' || inputReader[lastCharIndex] == '!' || inputReader[lastCharIndex] == '?' || inputReader[lastCharIndex] == ':') {
                        temporaryLength++;
                    }
                    else if(inputReader[lastCharIndex] == '-'){
                        noHyphen = false;
                    }
                }
                if (currentLength != 0 && noHyphen) {
                    temporaryLength++;
                }
                noHyphen = true;
                //checks if new word goes over line length
                if (isWordOverLine(index, inputReader, temporaryLength, lineLength) == 1 && firstWordOutted) {
                    outf << endl;
                    currentLength = 0;
                    //accounts for cases where the input is larger than the lineLength
                    int extraLargeCounter = 0;
                    while (isWordOverLine(index, inputReader, currentLength, lineLength)) {
                        returnValue = 1;
                        extraLargeCounter++;
                        outf << inputReader[index];
                        index++;
                        currentLength++;
                        if (extraLargeCounter >= lineLength) {
                            outf << endl;
                            currentLength = 0;
                            extraLargeCounter = 0;
                        }
                    }
                }


                //checks if there is a punctuation mark and adds an extra space
                if (lastCharIndex >= 0 && currentLength != 0) {
                    if (inputReader[lastCharIndex] == '.' || inputReader[lastCharIndex] == '!' || inputReader[lastCharIndex] == '?' || inputReader[lastCharIndex] == ':') {
                        outf << ' ';
                        currentLength++;
                    }
                    else if (inputReader[lastCharIndex] == '-') {
                        noHyphen = false;
                    }
                }
                //adds a space to separate new words that do not start a new line 
                if (currentLength != 0 && noHyphen) {
                    outf << ' ';
                    currentLength++;
                }
                noHyphen = true;

            }
            
            //output next letter in the string if it a line break is not first needed
                outf << inputReader[index];
                currentLength++;
                firstWordOutted = true;

        }
        strcpy(inputReader, ""); //clear the inputReader to prepare for next line of input
        if (isLastInput) {
            outf << endl;
        }
        fillerParBreaks = 0;
    }
    return returnValue;
}


//returns 1 if the word about to be printed would go over lineLength
int isWordOverLine(int currentIndex, char line[], int length, int lineMax) {
    int wordLength = 0;
        while (isspace(line[currentIndex]) == false && line[currentIndex] != '\0') {
            wordLength++;
            currentIndex++;
        }
        if (wordLength + length > lineMax) {
            return 1; //exceeds max line length
        }
        return 0;
}

bool hasHyphen(int currentIndex, char line[]) {
    for (int index = currentIndex; isspace(line[index]) == false && line[index] != '\0'; index++) {
        if (line[index] == '-') {
            return true;
        }
    }
    return false;
}

bool isParagraphBreak(int currentIndex, char line[]) {
    //verifies that checking the characters after the currentIndex would not go out of boudns
    for (int i = currentIndex; i < currentIndex + 3; i++) {
        if (line[i] == '\0') {
            return false;
        }
    }
    //checks that @P@ is present
    if (line[currentIndex] == '@' && line[currentIndex + 1] == 'P' && line[currentIndex + 2] == '@' && isspace(line[currentIndex + 3]))
        return true;
    return false;
}


//checks if there are no remaining words that are not paragraph breaks
bool isRestParagraphBreaks(int currentIndex, char line[]) {
    while (line[currentIndex] != '\0') {
        if (isspace(line[currentIndex]) || line[currentIndex] == '@' || line[currentIndex] == 'P') {
        }
        else {
            return false;
        }
        currentIndex++;
    }
    return true;
}

bool noMoreWords(int currentIndex, char line[]) {
    for (int i = currentIndex; line[i] != '\0'; i++) {
        if (isspace(line[i])) {

        }
        else {
            return false;
        }
    }
    return true;
}

void wordSegment(int index, char line[]) {
    int length = 0;
    for (int i = index; line[i] != '\0'; i++) {
        length++;
    }
    for (int i = 0; i < length; i++) {
        if (isspace(line[i])) {
            line[i] = ' ';
        }
        if (line[i] == '-') {
            if (i > 0 && i < length - 1) {
                for (int j = length; j > i; j--) {
                    line[j + 1] = line[j];
                }
                line[i + 1] = ' ';
                length++; //increase the length to account for added space
                i++; //skip the space
            }
        }
    }
}




