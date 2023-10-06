#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#define MAXCOUNT 40000
#define MAX_STRING_LENGTH 32

WINDOW* mainWindow;

typedef struct Student {
    int code;
    char Name[MAX_STRING_LENGTH];
    char School[MAX_STRING_LENGTH];
    int Korean;
    int Math;
    int English;
    int Science1;
    int Science2;
    int Total;
} Student;

//Global Variables
int dCount = 0;
Student gdata[MAXCOUNT];
char currentMsg[100] = "";
int gMethod=0;
int gSub=0;
double gCtu = 0;

/**
 * CSV Loading Function
 * This function load csv file and make structure list using them
*/
char* getNextString(char* srcStr, char deli, char* buffer, size_t buffer_size) {
    while (*srcStr && *srcStr != deli && buffer_size > 1) {
        *buffer++ = *srcStr++;
        buffer_size--;
    }
    *buffer = '\0';
    if (*srcStr == deli) {
        srcStr++;
    }
    return srcStr;
}

int readData(const char* fileName, Student* data, unsigned int* count) {
    Student* pStart = data;
    FILE* pFile = fopen(fileName, "r");
    
    if (pFile != NULL) {
        char oneLine[128];
        if (fgets(oneLine, sizeof(oneLine), pFile) == NULL) {
            fclose(pFile);
            return 0;
        }
        
        while (fgets(oneLine, sizeof(oneLine), pFile) != NULL) {
            Student* currentStudent = data;
            
            char str[32];
            char* pPos = oneLine;
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->code = atoi(str);
            
            pPos = getNextString(pPos, ',', currentStudent->Name, sizeof(currentStudent->Name));
            pPos = getNextString(pPos, ',', currentStudent->School, sizeof(currentStudent->School));
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->Korean = atoi(str);
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->Math = atoi(str);
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->English = atoi(str);
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->Science1 = atoi(str);
            
            pPos = getNextString(pPos, ',', str, sizeof(str));
            currentStudent->Science2 = atoi(str);
            
            currentStudent->Total = currentStudent->Korean + currentStudent->Math + 
                                    currentStudent->English + currentStudent->Science1 + 
                                    currentStudent->Science2;
            
            data++;
        }
        
        fclose(pFile);
        *count = data - pStart;
        return 1;
    }
    
    return 0;
}
//End CSV Loading Function

/**
 * Showing Help
 * This function shows help to window
*/
void displayHelp() 
{
    WINDOW* helpWindow;
    int max_height, max_width;
    
    // Get the dimensions of the screen
    getmaxyx(mainWindow, max_height, max_width);

    // Create a new window for help
    helpWindow = newwin(max_height - 4, max_width - 4, 2, 2);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE); // Define a color pair (e.g., yellow text on black background)
    wattron(helpWindow, COLOR_PAIR(1));
    // Add content to the help window
    mvwprintw(helpWindow, 1, 1, "Available commands:");
    mvwprintw(helpWindow, 2, 1, "sort <Method> <Subject>: Sort students by selected subject name, using provided sorting 'method'");
    mvwprintw(helpWindow, 3, 1, "getrcut <Subject>: Get selected subject's rate cutting line");
    mvwprintw(helpWindow, 4, 1, "getr <Name>: Get student's rate");
    mvwprintw(helpWindow, 5, 1, "\t");
    mvwprintw(helpWindow, 6, 1, "Method, subject codes: ");
    mvwprintw(helpWindow, 7, 1, "Method(0 to 3): selection, bubble, merge, quick");
    mvwprintw(helpWindow, 8, 1, "Subject(1 to 6) Korean, Math, English, Science1, Science2, Total");

    // Display the help window
    wrefresh(helpWindow);

    // Wait for a key press to close the help window
    wgetch(helpWindow);
    use_default_colors();
    // Clean up and delete the help window
    delwin(helpWindow);
}
//End DisplayHelp Function

/**
 * Popup Alert, and print output etc.. function
 * This function make a popup or print main screen
*/
void popupAlert(const char *message, int code, int idx, double rate, int grade) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Calculate window size based on the message length
    int height = 5;
    int width = strlen(message) + 6; // 6 is added for padding
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    // Create the alert window
    WINDOW *alertwin = newwin(height, width, starty, startx);

    // Set color for the alert window
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    wbkgd(alertwin, COLOR_PAIR(1));

    // Center the message in the alert window
    int y, x;
    getmaxyx(alertwin, y, x);
    mvwprintw(alertwin, y / 2, (x - strlen(message)) / 2, "%d: Rank: %d, Rate: %.2lf%% Grade: %d", code, idx+1, rate, grade);

    // Refresh the alert window to display it
    wrefresh(alertwin);

    // Wait for user input (e.g., press any key) to close the alert
    getch();

    // Clean up and close the alert window
    delwin(alertwin);

    // Terminate ncurses
    endwin();
}

int getGradeByRate(int rate)
{
    if (rate <= 4) return 1;
    if (rate > 4 && rate <= 11) return 2;
    if (rate > 11 && rate <= 23) return 3;
    if (rate > 23 && rate <= 40) return 4;
    if (rate > 40 && rate <= 60) return 5;
    if (rate > 60 && rate <= 77) return 6;
    if (rate > 77 && rate <= 89) return 7;
    if (rate > 89 && rate <= 96) return 8;
    if (rate > 96 && rate <= 100) return 9;
    return 0;
}
//End Etc function

/**
 * Sort Algorithms
 * These functions sort data by each mechanism
 * usage: "Name"Sort(StudentsArray, ArraySize, WhatToSort);
*/
void SelectionSort(Student arr[], int n, int method)
{
    for (int i=0; i<n; i++){
        int Min = i;
        for (int j=i+1; j<n; j++) {
            switch (method){
                case 0:
                    if(arr[j].code<arr[Min].code) Min = j;
                    break;
                case 1:
                    if(arr[j].Korean>arr[Min].Korean) Min = j;
                    break;
                case 2:
                    if(arr[j].Math>arr[Min].Math) Min = j;
                    break;
                case 3:
                    if(arr[j].English>arr[Min].English) Min = j;
                    break;
                case 4:
                    if(arr[j].Science1>arr[Min].Science1) Min = j;
                    break;
                case 5:
                    if(arr[j].Science2>arr[Min].Science2) Min = j;
                    break;
                case 6:
                    if(arr[j].Total>arr[Min].Total) Min = j;
                    break;
            }
        }
        struct Student temp = arr[i];
        arr[i] = arr[Min];
        arr[Min] = temp;
    }
}

void BubbleSort(Student arr[], int n, int method)
{
    int i, j;
    Student temp;

    for (i=0; i<n-1; i++)
    {
        for (j=0; j<(n-i)-1; j++)
        {
            switch (method) {
                case 0:
                    if (arr[j].code > arr[j+1].code) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 1:
                    if (arr[j].Korean < arr[j+1].Korean) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 2:
                    if (arr[j].Math < arr[j+1].Math) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 3:
                    if (arr[j].English < arr[j+1].English) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 4:
                    if (arr[j].Science1 < arr[j+1].Science1) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 5:
                    if (arr[j].Science2 < arr[j+1].Science2) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
                case 6:
                    if (arr[j].Total < arr[j+1].Total) {
                        temp = arr[j];
                        arr[j] = arr[j+1];
                        arr[j+1] = temp;
                    }
                    break;
            }
        }
    }
}

//Merge Sort
void MergeTwoArea(Student arr[], int left, int mid, int right, int method)
{
    int fIdx = left;
    int rIdx = mid + 1;
    int sIdx = 0;
    int size = right - left + 1;

    Student *sortArr = (Student *)malloc(sizeof(Student) * size);

    while (fIdx <= mid && rIdx <= right)
    {
        switch (method){
            case 0:
                if (arr[fIdx].code >= arr[rIdx].code)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 1:
                if (arr[fIdx].Korean >= arr[rIdx].Korean)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 2:
                if (arr[fIdx].Math >= arr[rIdx].Math)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 3:
                if (arr[fIdx].English >= arr[rIdx].English)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 4:
                if (arr[fIdx].Science1 >= arr[rIdx].Science1)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 5:
                if (arr[fIdx].Science2 >= arr[rIdx].Science2)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
            case 6:
                if (arr[fIdx].Total >= arr[rIdx].Total)
                    sortArr[sIdx++] = arr[fIdx++];
                else
                    sortArr[sIdx++] = arr[rIdx++];
                break;
        }
    }

    while (fIdx <= mid) sortArr[sIdx++] = arr[fIdx++];

    while (rIdx <= right) sortArr[sIdx++] = arr[rIdx++];
    for (int i = 0; i < size; i++) arr[left + i] = sortArr[i];

    free(sortArr);
}

void MergeSort(Student arr[], int left, int right, int method)
{
    int mid;
    if (left < right)
    {
        mid = (left + right) / 2;
        MergeSort(arr, left, mid, method);
        MergeSort(arr, mid + 1, right, method);
        MergeTwoArea(arr, left, mid, right, method);
    }
}

//Quick Sort
void Swap(Student arr[], int idx1, int idx2) {
    Student temp = arr[idx1];
    arr[idx1] = arr[idx2];
    arr[idx2] = temp;
}

int CompareStudents(Student s1, Student s2, int method) {
    // Compare students based on the 'Total' field in descending order
    switch(method){
        case 1:
            return s2.Korean - s1.Korean;
            break;
        case 2:
            return s2.Math - s1.Math;
            break;
        case 3:
            return s2.English - s1.English;
            break;
        case 4:
            return s2.Science1 - s1.Science1;
            break;
        case 5:
            return s2.Science2 - s1.Science2;
            break;
        case 6:
            return s2.Total - s1.Total;
            break;
    }
    return s2.Total - s1.Total;
}

int Partition(Student arr[], int left, int right, int method) {
    Student pivot = arr[left];
    int low = left + 1;
    int high = right;

    while (1) {
        switch(method){
            case 1:
                while (low <= high && arr[low].Korean >= pivot.Korean)
                    low++;
                while (low <= high && arr[high].Korean <= pivot.Korean)
                    high--;
                break;
            case 2:
                while (low <= high && arr[low].Math >= pivot.Math)
                    low++;
                while (low <= high && arr[high].Math <= pivot.Math)
                    high--;
                break;
            case 3:
                while (low <= high && arr[low].English >= pivot.English)
                    low++;
                while (low <= high && arr[high].English <= pivot.English)
                    high--;
                break;
            case 4:
                while (low <= high && arr[low].Science1 >= pivot.Science1)
                    low++;
                while (low <= high && arr[high].Science1 <= pivot.Science1)
                    high--;
                break;
            case 5:
                while (low <= high && arr[low].Science2 >= pivot.Science2)
                    low++;
                while (low <= high && arr[high].Science2 <= pivot.Science2)
                    high--;
                break;
            case 6:
                while (low <= high && arr[low].Total >= pivot.Total)
                    low++;
                while (low <= high && arr[high].Total <= pivot.Total)
                    high--;
                break;
        }
        if (low <= high)
            Swap(arr, low, high);
        else
            break;
    }

    Swap(arr, left, high);
    return high;
}

void QuickSort(Student arr[], int left, int right, int method) {
    if (left <= right) {
        int pivot = Partition(arr, left, right, method);
        QuickSort(arr, left, pivot - 1, method);
        QuickSort(arr, pivot + 1, right, method);
    }
}//End of Sort functions

/**
 * Relative Evaluation
*/
int findStudent(Student arr[], int code)
{
    for (int i = 0; i < dCount; i++) {
        if (arr[i].code == code) {
            return i;
        }
    }
    return -1;
}

void getR(Student arr[], const char* code, int subject) {
    if (subject < 0 || subject > 6) {
        strcpy(currentMsg, "Invalid subject");
        return;
    }
    gSub = subject;
    MergeSort(gdata, 0, sizeof(gdata)/sizeof(Student)-1, subject);
    int idx = findStudent(gdata, atoi(code));
    if (idx == -1) {
        strcpy(currentMsg, "Student not found");
    } else {
        int grade;
        double rate = (double)idx / dCount * 100.0;
        if (subject == 3){
            int engS;
            for (int i = 0; i < dCount; i++) {
                if (arr[i].code == atoi(code)) {
                    engS = arr[i].English;
                }
            }
            if (engS >= 90) grade = 1;
            if (engS < 90 && engS >= 80) grade = 2;
            if (engS < 80 && engS >= 70) grade = 3;
            if (engS < 70 && engS >= 60) grade = 4;
            if (engS < 60 && engS >= 50) grade = 5;
            if (engS < 50 && engS >= 40) grade = 6;
            if (engS < 40 && engS >= 30) grade = 7;
            if (engS < 30 && engS >= 20) grade = 8;
            if (engS < 20) grade = 9;
        } else {
            grade = getGradeByRate(rate);
        }
        popupAlert("Student Code: %d\nRank: %d\nRate: %.2lf%%\nGrade: %d", atoi(code), idx, rate, grade);

    }
}

void getRCut(Student arr[], int subject)
{
    if (subject < 0 || subject > 6) {
        strcpy(currentMsg, "Invalid subject");
        return;
    }
    gSub = subject;
    MergeSort(gdata, 0, sizeof(gdata)/sizeof(Student)-1, subject);
    int target1Idx, target2Idx, target3Idx, target4Idx, target5Idx, target6Idx, target7Idx, target8Idx, target9Idx;
    switch (subject){
        case 1:
            target1Idx = gdata[(int)round((dCount+1)*0.04)].Korean;
            target2Idx = gdata[(int)round((dCount+1)*0.11)].Korean;
            target3Idx = gdata[(int)round((dCount+1)*0.23)].Korean;
            target4Idx = gdata[(int)round((dCount+1)*0.40)].Korean;
            target5Idx = gdata[(int)round((dCount+1)*0.60)].Korean;
            target6Idx = gdata[(int)round((dCount+1)*0.77)].Korean;
            target7Idx = gdata[(int)round((dCount+1)*0.89)].Korean;
            target8Idx = gdata[(int)round((dCount+1)*0.96)].Korean;
            target9Idx = gdata[(int)round((dCount+1)*1)].Korean;
            break;
        case 2:
            target1Idx = gdata[(int)round((dCount+1)*0.04)].Math;
            target2Idx = gdata[(int)round((dCount+1)*0.11)].Math;
            target3Idx = gdata[(int)round((dCount+1)*0.23)].Math;
            target4Idx = gdata[(int)round((dCount+1)*0.40)].Math;
            target5Idx = gdata[(int)round((dCount+1)*0.60)].Math;
            target6Idx = gdata[(int)round((dCount+1)*0.77)].Math;
            target7Idx = gdata[(int)round((dCount+1)*0.89)].Math;
            target8Idx = gdata[(int)round((dCount+1)*0.96)].Math;
            target9Idx = gdata[(int)round((dCount+1)*1)].Math;
            break;
        case 3:
            target1Idx = 90;
            target2Idx = 80;
            target3Idx = 70;
            target4Idx = 60;
            target5Idx = 50;
            target6Idx = 40;
            target7Idx = 30;
            target8Idx = 20;
            target9Idx = 20;
            break;
        case 4:
            target1Idx = gdata[(int)round((dCount+1)*0.04)].Science1;
            target2Idx = gdata[(int)round((dCount+1)*0.11)].Science1;
            target3Idx = gdata[(int)round((dCount+1)*0.23)].Science1;
            target4Idx = gdata[(int)round((dCount+1)*0.40)].Science1;
            target5Idx = gdata[(int)round((dCount+1)*0.60)].Science1;
            target6Idx = gdata[(int)round((dCount+1)*0.77)].Science1;
            target7Idx = gdata[(int)round((dCount+1)*0.89)].Science1;
            target8Idx = gdata[(int)round((dCount+1)*0.96)].Science1;
            target9Idx = gdata[(int)round((dCount+1)*1)].Science1;
            break;
        case 5:
            target1Idx = gdata[(int)round((dCount+1)*0.04)].Science2;
            target2Idx = gdata[(int)round((dCount+1)*0.11)].Science2;
            target3Idx = gdata[(int)round((dCount+1)*0.23)].Science2;
            target4Idx = gdata[(int)round((dCount+1)*0.40)].Science2;
            target5Idx = gdata[(int)round((dCount+1)*0.60)].Science2;
            target6Idx = gdata[(int)round((dCount+1)*0.77)].Science2;
            target7Idx = gdata[(int)round((dCount+1)*0.89)].Science2;
            target8Idx = gdata[(int)round((dCount+1)*0.96)].Science2;
            target9Idx = gdata[(int)round((dCount+1)*1)].Science2;
            break;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Calculate window size based on the message length
    int height = 10;
    int width = 16; // 6 is added for padding
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    // Create the alert window
    WINDOW *rcut = newwin(height, width, starty, startx);

    // Set color for the alert window
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    wbkgd(rcut, COLOR_PAIR(1));
    mvwprintw(rcut, 0, 0 / 2, "Grade cut\n1: %d\n2: %d\n3: %d\n4: %d\n5: %d\n6: %d\n7: %d\n8: %d\n9: %d", target1Idx, target2Idx, target3Idx, target4Idx, target5Idx, target6Idx, target7Idx, target8Idx, target9Idx);

    // Refresh the alert window to display it
    wrefresh(rcut);

    // Wait for user input (e.g., press any key) to close the alert
    getch();

    // Clean up and close the alert window
    delwin(rcut);

    // Terminate ncurses
    endwin();
}
//End Relative function

/**
 * Handle Command
 * This function will handle commands and arguments
*/
void splitString(const char *input, char *output[], int *count)
{
    char buffer[256]; // A buffer to store each extracted substring
    int i, j = 0;

    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] != ' ') {
            buffer[j++] = input[i];
        } else {
            buffer[j] = '\0'; // Null-terminate the substring
            if (j > 0) {
                // Allocate memory for the substring and copy it
                output[(*count)++] = strdup(buffer);
                j = 0; // Reset the buffer
            }
        }
    }

    // Process the last substring if it exists
    if (j > 0) {
        buffer[j] = '\0'; // Null-terminate the last substring
        output[(*count)++] = strdup(buffer);
    }
}

void handleCommand(WINDOW* win, const char* cmd)
{
    char *substr[5];
    int count = 0;
    splitString(cmd, substr, &count);

    if (substr[0] == NULL) return;

    if (!strcmp(substr[0], "help")) {
        displayHelp();
    } else if (!strcmp(substr[0], "sort")){
        if (substr[1] == NULL || substr[2] == NULL)
        {
            strcpy(currentMsg, "Uncorrect use of command. Please type help.");
            return;
        }
        if (atoi(substr[1]) < 0 || atoi(substr[1]) > 3) {
            strcpy(currentMsg, "Invalid method");
            return;
        }
        if (atoi(substr[2]) < 0 || atoi(substr[2]) > 6) {
            strcpy(currentMsg, "Invalid subject");
            return;
        }
        clock_t start, end;
        start = clock();
        switch (atoi(substr[1])){
            case 0:
                SelectionSort(gdata, dCount, atoi(substr[2]));
                end = clock();
                gCtu = ((double) (end - start)) / CLOCKS_PER_SEC;
                break;
            case 1:
                BubbleSort(gdata, dCount, atoi(substr[2]));
                end = clock();
                gCtu = ((double) (end - start)) / CLOCKS_PER_SEC;
                break;
            case 2:
                if (atoi(substr[2]) == 0) return;
                MergeSort(gdata, 0, sizeof(gdata)/sizeof(Student) - 1, atoi(substr[2]));
                end = clock();
                gCtu = ((double) (end - start)) / CLOCKS_PER_SEC;
                break;
            case 3:
                if (atoi(substr[2]) == 0) return;
                QuickSort(gdata, 0, dCount-1, atoi(substr[2]));
                end = clock();
                gCtu = ((double) (end - start)) / CLOCKS_PER_SEC;
                break;
        }
        gMethod = atoi(substr[1]);
        gSub = atoi(substr[2]);
        refresh();
    } else if (!strcmp(substr[0], "getr")) {
        if (substr[1] == NULL || substr[2] == NULL)
        {
            if (atoi(substr[2]) < 0 || atoi(substr[2]) > 6) {
                strcpy(currentMsg, "Invalid subject");
                return;
            }
            strcpy(currentMsg, "Uncorrect use of command. Please type help.");
            return;
        } 
        getR(gdata, substr[1], atoi(substr[2]));
        refresh();
    } else if(!strcmp(substr[0], "getrcut")){
        if (atoi(substr[2]) < 0 || atoi(substr[2]) > 6) {
            strcpy(currentMsg, "Invalid subject");
            return;
        }
        if (substr[1] == NULL)
        {
            strcpy(currentMsg, "Uncorrect use of command. Please type help.");
            return;
        } 
        getRCut(gdata, atoi(substr[1]));
        refresh();
    }else{
        strcpy(currentMsg, "Command not found");
    }
}
//End handleCommnd Function

//Main Function
int main(int argc, char* argv[]) {
    Student data[MAXCOUNT];
    unsigned int dataCount = 0;
    if (readData(argv[1], data, &dataCount)) {
        for (int i = 0; i < dataCount; i++) {
            gdata[i] = data[i];
        }
        initscr();
        noecho();
        cbreak();
        mainWindow = stdscr;
        keypad(stdscr, TRUE);
        clock_t start, end;
        start = clock();
        dCount = dataCount;
        //QuickSort(gdata, 0, sizeof(gdata)/sizeof(Student) - 1, 0);
        SelectionSort(data, dCount, 0);
        end = clock();
        gCtu = ((double) (end - start)) / CLOCKS_PER_SEC;
        int row, col;
        int max_rows = LINES - 6; // Leave room for header, footer, and input field
        int start_row = 0;
        char input_buffer[MAX_STRING_LENGTH] = ""; // Buffer for text input
        int input_col = 0;

        while (1) {
            clear();
            switch (gSub){
                case 0:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tTotal");
                    break;
                case 1:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tKorean");
                    break;
                case 2:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tMath");
                    break;
                case 3:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tEnglish");
                    break;
                case 4:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tScience1");
                    break;
                case 5:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tScience2");
                    break;
                case 6:
                    mvprintw(0, 1, "Index\tCode\tName\tSchool\tTotal");
                    break;
            }

            for (row = 0; row < max_rows && start_row + row < dCount; row++) {
                Student* currentStudent = &gdata[start_row + row];
                switch (gSub){
                    case 0:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Total);
                        break;
                    case 1:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Korean);
                        break;
                    case 2:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Math);
                        break;
                    case 3:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->English);
                        break;
                    case 4:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Science1);
                        break;
                    case 5:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Science2);
                        break;
                    case 6:
                        mvprintw(row + 1, 1, "%d\t%d\t%s\t%s\t%d", start_row+row+1, currentStudent->code, currentStudent->Name, currentStudent->School, currentStudent->Total);
                        break;
                }
            }

            // Display the input field at the bottom
            mvprintw(LINES-1, 1, currentMsg);
            mvprintw(LINES - 3, 1, "Press Q to quit, Type help to help");
            mvprintw(LINES - 2, 1, "Elapsed time for %d sorting: %lf",gMethod , gCtu);
            mvprintw(LINES - 4, 1, "Input: ");
            mvprintw(LINES - 4, 8, input_buffer);
            use_default_colors();
            refresh();

            int ch = getch();
            if (ch == 'q' || ch == 'Q')
                break;
            else if (ch == KEY_DOWN && start_row + row < dataCount)
                start_row++;
            else if (ch == KEY_UP && start_row > 0)
                start_row--;
            else if (ch == '\n') {
                handleCommand(mainWindow, input_buffer);
                refresh();
                // Clear the input_buffer
                memset(input_buffer, 0, sizeof(input_buffer));
                input_col = 0;
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                // Handle Backspace key press to delete characters from input_buffer
                if (input_col > 0) {
                    input_col--;
                    input_buffer[input_col] = '\0';
                }
            } else if (input_col < MAX_STRING_LENGTH - 1) {
                // Add the character to the input_buffer
                input_buffer[input_col] = ch;
                input_col++;
                input_buffer[input_col] = '\0';
            }
        }

        endwin();
    } else {
        printf("Error occurred opening file!\n");
    }
    return 0;
}