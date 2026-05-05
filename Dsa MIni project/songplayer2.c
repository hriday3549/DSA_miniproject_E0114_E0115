#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h> // For reading folders

// Auto-detect the Operating System for audio playback
#ifdef _WIN32
    #define PLAY_COMMAND "start \"\" \"%s\""
#elif __APPLE__
    #define PLAY_COMMAND "afplay \"%s\" &"
#else
    #define PLAY_COMMAND "xdg-open \"%s\" &"
#endif

// Node structure representing a single song
typedef struct Song {
    char title[100];
    char artist[100];
    int duration; 
    char filepath[256]; // NEW: Tells the OS where the file is
    struct Song* prev;
    struct Song* next;
} Song;

Song* head = NULL;
Song* tail = NULL;
Song* currentPlaying = NULL; 

// --- HELPER FUNCTIONS ---

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void stripNewline(char* str) {
    str[strcspn(str, "\r\n")] = 0; // Added \r to kill BOTH invisible characters
}

int matchIgnoreCase(const char* str1, const char* str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (tolower((unsigned char)str1[i]) != tolower((unsigned char)str2[i])) return 0; 
        i++;
    }
    return str1[i] == '\0' && str2[i] == '\0';
}

void swapData(Song* a, Song* b) {
    char tTitle[100], tArtist[100], tPath[256];
    int tDur;

    strcpy(tTitle, a->title); strcpy(a->title, b->title); strcpy(b->title, tTitle);
    strcpy(tArtist, a->artist); strcpy(a->artist, b->artist); strcpy(b->artist, tArtist);
    strcpy(tPath, a->filepath); strcpy(a->filepath, b->filepath); strcpy(b->filepath, tPath);
    
    tDur = a->duration; a->duration = b->duration; b->duration = tDur;
}

// --- CORE FEATURES ---

// 1. ADD SONG
void addSong(const char* title, const char* artist, int duration, const char* filepath) {
    Song* newSong = (Song*)malloc(sizeof(Song));
    if (!newSong) return;

    strcpy(newSong->title, title);
    strcpy(newSong->artist, artist);
    strcpy(newSong->filepath, filepath);
    newSong->duration = duration;
    newSong->prev = NULL;
    newSong->next = NULL;

    if (head == NULL) {
        head = tail = newSong;
    } else {
        tail->next = newSong;
        newSong->prev = tail;
        tail = newSong;
    }
}

// 2. LOAD FROM FOLDER (The new MP3 scanner)
void loadFolder(const char* folderPath) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    if ((dir = opendir(folderPath)) != NULL) {
        printf("\nScanning '%s' folder...\n", folderPath);
        while ((entry = readdir(dir)) != NULL) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext != NULL && strcmp(ext, ".mp3") == 0) {
                // Create the full path (e.g., "music/song.mp3")
                char fullPath[256];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);
                
                // Add to linked list (using filename as title, unknown artist)
                addSong(entry->d_name, "Unknown Artist", 0, fullPath);
                count++;
            }
        }
        closedir(dir);
        printf("[SUCCESS] Loaded %d MP3 files!\n", count);
    } else {
        printf("\n[ERROR] Could not find a folder named '%s'.\n", folderPath);
        printf("Make sure you created it next to your C file!\n");
    }
}

// 3. ACTUAL AUDIO PLAYBACK
void playRealAudio(const char* filepath) {
    char command[512];
    snprintf(command, sizeof(command), PLAY_COMMAND, filepath);
    printf(">> Telling your computer to play: %s\n", filepath);
    system(command); // This talks to your OS!
}

// 4. PLAYER CONTROLS
void playSong(const char* query) {
    if (head == NULL) { printf("\n[ERROR] Playlist empty.\n"); return; }

    Song* current = head;
    while (current != NULL) {
        if (matchIgnoreCase(current->title, query)) {
            currentPlaying = current;
            printf("\n====================================\n");
            printf(" ▶ NOW PLAYING: \"%s\"\n", currentPlaying->title);
            printf("====================================\n");
            playRealAudio(currentPlaying->filepath);
            return;
        }
        current = current->next;
    }
    printf("\n[ERROR] Song not found.\n");
}

void playNext() {
    if (currentPlaying && currentPlaying->next) {
        currentPlaying = currentPlaying->next;
        printf("\nSkipping to Next Track...\n");
        playSong(currentPlaying->title);
    } else {
        printf("\n[INFO] End of playlist.\n");
    }
}

void playPrev() {
    if (currentPlaying && currentPlaying->prev) {
        currentPlaying = currentPlaying->prev;
        printf("\nSkipping to Previous Track...\n");
        playSong(currentPlaying->title);
    } else {
        printf("\n[INFO] Beginning of playlist.\n");
    }
}
// 4b. PLAYER: Play the First Track
void playFirst() {
    if (head == NULL) {
        printf("\n[ERROR] The playlist is empty. Load some songs first!\n");
        return;
    }
    printf("\nSkipping to the beginning of the playlist...\n");
    playSong(head->title); // We just pass the head node's title to your existing play function!
}

// 5. SORT & DISPLAY
void sortPlaylist() {
    if (head == NULL || head->next == NULL) return;
    int swapped;
    Song* current;
    Song* lastPtr = NULL;
    do {
        swapped = 0;
        current = head;
        while (current->next != lastPtr) {
            if (tolower((unsigned char)current->title[0]) > tolower((unsigned char)current->next->title[0])) {
                swapData(current, current->next);
                swapped = 1;
            }
            current = current->next;
        }
        lastPtr = current;
    } while (swapped);
    printf("\n[SUCCESS] Playlist sorted alphabetically.\n");
}

void displayPlaylist() {
    if (head == NULL) { printf("\n[EMPTY] The playlist is empty.\n"); return; }
    printf("\n--- Current Playlist ---\n");
    Song* current = head;
    int trackNum = 1;
    while (current != NULL) {
        if (current == currentPlaying) printf(" ▶ ");
        else printf("   ");
        
        printf("%d. %s - %s\n", trackNum, current->title, current->artist);
        current = current->next;
        trackNum++;
    }
    printf("------------------------\n");
}

int main() {
    int choice;
    char tempQuery[100]; 
    setvbuf(stdout, NULL, _IONBF, 0); // Add this so Python can read the output instantly!
    printf("\n🎵 Welcome to the Ultimate C Music Player! 🎵\n");

    while (1) {
        printf("\n=== Main Menu ===\n");
        printf("1. 📂 Auto-Load 'music' Folder\n");
        printf("2. 🎵 Play a Specific Song\n");
        printf("3. ⏭  Play Next Track\n");
        printf("4. ⏮  Play Previous Track\n");
        printf("5. 🔠 Sort Playlist Alphabetically\n");
        printf("6. 📜 Display Playlist\n");
        printf("7. ⏪ Play First Track\n");
        printf("8. ❌ Exit\n");
        printf("Enter your choice: ");;
        
        if (scanf("%d", &choice) != 1) break;
        clearInputBuffer(); 

        switch (choice) {
            case 1:
                loadFolder("music");
                break;
            case 2:
                printf("\nEnter the exact Title of the song to play: ");
                fgets(tempQuery, 100, stdin);
                stripNewline(tempQuery);
                playSong(tempQuery);
                break;
            case 3: playNext(); break;
            case 4: playPrev(); break;
            case 5: sortPlaylist(); break;
            case 6: displayPlaylist(); break;
            case 7: 
                playFirst(); 
                break;
            case 8:
                printf("\nExiting player... Goodbye!\n");
                return 0;
            default: printf("\nInvalid choice.\n");
        }
    }
    return 0;
}