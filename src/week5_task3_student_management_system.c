// week5_task3_student_management_system.c
// Name: ________  Surname: ________  Student ID: ________
//
// Task 3: Mini-project – Student management system with file persistence
// Week 5 – Files & Modular Programming
//
// Build: make lab5   Run: ./bin/week5_task3_student_management_system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_STUDENTS 100
#define NAME_LEN 50
#define LINE_LEN 256
#define DATA_FILE "students.txt"

typedef struct Student {
    char name[NAME_LEN];
    int id;
    float gpa;
} Student;

// Function prototypes (do not change these signatures – the autograder calls them directly)
int  load_students(Student arr[], int max);          // returns number of records loaded
int  save_students(const Student arr[], int count);  // 0 on success, -1 on error
void add_student(Student arr[], int *count);
void list_students(const Student arr[], int count);
int  read_line(const char *prompt, char *buf, size_t size);

int main(void) {
    Student students[MAX_STUDENTS];
    int count = 0;
    char input[LINE_LEN];
    int running = 1;

    // Load existing data
    count = load_students(students, MAX_STUDENTS);
    printf("Loaded %d student(s) from %s\n", count, DATA_FILE);

    while (running) {
        printf("\n=== Student Management System ===\n");
        printf("1. List students\n");
        printf("2. Add student\n");
        printf("3. Save and Exit\n");

        int choice;
        if (read_line("Select an option: ", input, sizeof input) != 0) {
            choice = 3;                         // end of input: save and exit instead of looping forever
        } else {
            char *end;
            long value = strtol(input, &end, 10);
            choice = (end != input && *end == '\0') ? (int)value : 0;   // non-numbers become invalid
        }

        switch (choice) {
            case 1:
                list_students(students, count);
                break;
            case 2:
                add_student(students, &count);
                break;
            case 3:
                if (save_students(students, count) != 0) {
                    fprintf(stderr, "Error: could not save data to %s\n", DATA_FILE);
                } else {
                    printf("Data saved. Goodbye\n");
                }
                running = 0;
                break;
            default:
                printf("Invalid option. Try again.\n");
        }
    }

    return EXIT_SUCCESS;
}

// Provided helper – prints a prompt and reads one line from stdin into buf.
// The trailing newline is removed. Returns 0 on success, -1 on end of input (EOF).
int read_line(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)size, stdin) == NULL) {
        return -1;
    }
    size_t len = strcspn(buf, "\r\n");
    if (buf[len] == '\0' && len == size - 1) {
        // Line was longer than the buffer: discard the rest of it.
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    buf[len] = '\0';
    return 0;
}

// Loads records "name,id,gpa" from DATA_FILE.
// A missing or empty file is not an error: it simply loads 0 students.
// Blank or malformed lines are skipped; at most max records are loaded.
int load_students(Student arr[], int max) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        return 0;                               // first run: no file yet
    }

    char line[LINE_LEN];
    int count = 0;

    while (count < max && fgets(line, sizeof line, fp) != NULL) {
        Student tmp;
        // Keep only lines where all three fields were parsed
        if (sscanf(line, "%49[^,],%d,%f", tmp.name, &tmp.id, &tmp.gpa) == 3) {
            arr[count] = tmp;
            count++;
        }
    }

    fclose(fp);
    return count;
}

// Overwrites DATA_FILE with all current records.
int save_students(const Student arr[], int count) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        return -1;
    }

    int ok = 1;
    for (int i = 0; i < count; i++) {
        if (fprintf(fp, "%s,%d,%.2f\n", arr[i].name, arr[i].id, arr[i].gpa) < 0) {
            ok = 0;
        }
    }

    if (fclose(fp) != 0 || !ok) {
        return -1;
    }
    return 0;
}

// Asks for name, ID and GPA, validates them and appends a new student.
void add_student(Student arr[], int *count) {
    char name[LINE_LEN];
    char id_buf[LINE_LEN];
    char gpa_buf[LINE_LEN];

    if (*count >= MAX_STUDENTS) {
        printf("Cannot add more students (limit reached).\n");
        return;
    }

    // Read all three values first so a bad value does not leave input behind
    if (read_line("Enter name: ", name, sizeof name) != 0) return;
    if (read_line("Enter ID: ", id_buf, sizeof id_buf) != 0) return;
    if (read_line("Enter GPA: ", gpa_buf, sizeof gpa_buf) != 0) return;

    // Name: not empty, no comma (it is the field separator), must fit in the array
    int name_ok = name[0] != '\0'
               && strchr(name, ',') == NULL
               && strlen(name) < NAME_LEN;

    // ID: the whole input must be an integer that fits in an int
    char *end;
    errno = 0;
    long id = strtol(id_buf, &end, 10);
    int id_ok = end != id_buf && *end == '\0' && errno == 0
             && id >= INT_MIN && id <= INT_MAX;

    // GPA: the whole input must be a number between 0.0 and 4.0
    errno = 0;
    float gpa = strtof(gpa_buf, &end);
    int gpa_ok = end != gpa_buf && *end == '\0' && errno == 0
              && gpa >= 0.0f && gpa <= 4.0f;     // also rejects NaN

    if (!name_ok || !id_ok || !gpa_ok) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // Store the new student
    strcpy(arr[*count].name, name);
    arr[*count].id = (int)id;
    arr[*count].gpa = gpa;
    (*count)++;

    printf("Student added successfully\n");
}

// Prints all students as a table, or a message if there are none.
void list_students(const Student arr[], int count) {
    if (count <= 0) {
        printf("No students found.\n");
        return;
    }

    printf("%-5s %-20s %s\n", "ID", "Name", "GPA");
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %.2f\n", arr[i].id, arr[i].name, arr[i].gpa);
    }
}