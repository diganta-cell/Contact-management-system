#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CONTACTS 100
#define NAME_LEN     50
#define PHONE_LEN    20
#define EMAIL_LEN    50
#define LINE_LEN     256
#define FILE_NAME    "contacts.txt"

typedef struct {
    char name[NAME_LEN];
    char phone[PHONE_LEN];
    char email[EMAIL_LEN];
} Contact;

Contact contacts[MAX_CONTACTS];
int count = 0;

/* ---------- Helpers ---------- */

/* Prompt, read a line safely, strip newline, replace '|' (used as file separator) */
void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
    for (char *p = buf; *p; p++)
        if (*p == '|') *p = '-';
}

int readInt(const char *prompt) {
    char line[LINE_LEN];
    printf("%s", prompt);
    if (!fgets(line, sizeof line, stdin)) return -1;
    char *end;
    long v = strtol(line, &end, 10);
    if (end == line) return -1;
    return (int)v;
}

/* Case-insensitive substring search */
int containsIgnoreCase(const char *text, const char *term) {
    size_t tl = strlen(term);
    if (tl == 0) return 1;
    for (; *text; text++) {
        size_t i = 0;
        while (i < tl && text[i] &&
               tolower((unsigned char)text[i]) == tolower((unsigned char)term[i]))
            i++;
        if (i == tl) return 1;
    }
    return 0;
}

void printContact(int index) {
    printf("  [%d] %s | %s | %s\n", index + 1,
           contacts[index].name, contacts[index].phone,
           contacts[index].email[0] ? contacts[index].email : "(no email)");
}

/* ---------- File storage ---------- */

void saveContacts(void) {
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp) {
        printf("Error: could not save contacts to file.\n");
        return;
    }
    for (int i = 0; i < count; i++)
        fprintf(fp, "%s|%s|%s\n", contacts[i].name, contacts[i].phone, contacts[i].email);
    fclose(fp);
}

void loadContacts(void) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;                       /* no file yet - that's fine */

    char line[LINE_LEN];
    while (count < MAX_CONTACTS && fgets(line, sizeof line, fp)) {
        line[strcspn(line, "\n")] = '\0';
        char *p1 = strchr(line, '|');
        if (!p1) continue;
        *p1 = '\0';
        char *p2 = strchr(p1 + 1, '|');
        if (!p2) continue;
        *p2 = '\0';

        strncpy(contacts[count].name,  line,   NAME_LEN  - 1);
        strncpy(contacts[count].phone, p1 + 1, PHONE_LEN - 1);
        strncpy(contacts[count].email, p2 + 1, EMAIL_LEN - 1);
        contacts[count].name[NAME_LEN - 1]   = '\0';
        contacts[count].phone[PHONE_LEN - 1] = '\0';
        contacts[count].email[EMAIL_LEN - 1] = '\0';
        count++;
    }
    fclose(fp);
}

/* ---------- Core features ---------- */

void addContact(void) {
    if (count >= MAX_CONTACTS) {
        printf("Contact list is full.\n");
        return;
    }
    Contact c;
    readLine("Name  : ", c.name, NAME_LEN);
    if (c.name[0] == '\0') {
        printf("Name cannot be empty. Contact not added.\n");
        return;
    }
    readLine("Phone : ", c.phone, PHONE_LEN);
    if (c.phone[0] == '\0') {
        printf("Phone cannot be empty. Contact not added.\n");
        return;
    }
    readLine("Email (optional): ", c.email, EMAIL_LEN);

    contacts[count++] = c;
    saveContacts();
    printf("Contact added successfully.\n");
}

void listContacts(void) {
    if (count == 0) {
        printf("No contacts saved yet.\n");
        return;
    }
    printf("\n--- All Contacts (%d) ---\n", count);
    for (int i = 0; i < count; i++)
        printContact(i);
}

/* Prints matches for a name/phone term; returns number of matches */
int showMatches(const char *term) {
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (containsIgnoreCase(contacts[i].name, term) ||
            containsIgnoreCase(contacts[i].phone, term)) {
            printContact(i);
            found++;
        }
    }
    return found;
}

void searchContact(void) {
    char term[NAME_LEN];
    readLine("Search by name or phone: ", term, sizeof term);
    printf("\n--- Search Results ---\n");
    if (showMatches(term) == 0)
        printf("No matching contacts found.\n");
}

/* Search, then let the user pick one of the results. Returns index or -1 */
int pickContact(void) {
    char term[NAME_LEN];
    readLine("Search by name or phone: ", term, sizeof term);
    printf("\n--- Matches ---\n");
    if (showMatches(term) == 0) {
        printf("No matching contacts found.\n");
        return -1;
    }
    int n = readInt("Enter the contact number to select (0 to cancel): ");
    if (n < 1 || n > count) {
        printf("Cancelled.\n");
        return -1;
    }
    int idx = n - 1;
    if (!containsIgnoreCase(contacts[idx].name, term) &&
        !containsIgnoreCase(contacts[idx].phone, term)) {
        printf("That number was not in the search results.\n");
        return -1;
    }
    return idx;
}

void editContact(void) {
    int idx = pickContact();
    if (idx < 0) return;

    char buf[LINE_LEN];
    printf("Press Enter to keep the current value.\n");

    char prompt[100];
    snprintf(prompt, sizeof prompt, "Name  [%s]: ", contacts[idx].name);
    readLine(prompt, buf, NAME_LEN);
    if (buf[0]) strcpy(contacts[idx].name, buf);

    snprintf(prompt, sizeof prompt, "Phone [%s]: ", contacts[idx].phone);
    readLine(prompt, buf, PHONE_LEN);
    if (buf[0]) strcpy(contacts[idx].phone, buf);

    snprintf(prompt, sizeof prompt, "Email [%s]: ", contacts[idx].email);
    readLine(prompt, buf, EMAIL_LEN);
    if (buf[0]) strcpy(contacts[idx].email, buf);

    saveContacts();
    printf("Contact updated successfully.\n");
}

void deleteContact(void) {
    int idx = pickContact();
    if (idx < 0) return;

    char ans[10];
    printf("Delete this contact?\n");
    printContact(idx);
    readLine("Confirm (y/n): ", ans, sizeof ans);
    if (ans[0] != 'y' && ans[0] != 'Y') {
        printf("Delete cancelled.\n");
        return;
    }

    for (int i = idx; i < count - 1; i++)
        contacts[i] = contacts[i + 1];
    count--;
    saveContacts();
    printf("Contact deleted.\n");
}

/* ---------- Main menu ---------- */

int main(void) {
    loadContacts();

    int choice;
    do {
        printf("\n===== Contact Management System =====\n");
        printf("1. Add contact\n");
        printf("2. Search contact\n");
        printf("3. Edit contact\n");
        printf("4. Delete contact\n");
        printf("5. List all contacts\n");
        printf("0. Exit\n");
        choice = readInt("Choose an option: ");

        switch (choice) {
            case 1: addContact();    break;
            case 2: searchContact(); break;
            case 3: editContact();   break;
            case 4: deleteContact(); break;
            case 5: listContacts();  break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}